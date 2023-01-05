# NginxCheck

## Info
Detect NginxStealth

This module list the address :
1. checker and handler in ngx_http_core_main_conf_t -> phase_engine -> handlers
2. elts in ngx_http_core_main_conf_t -> phases[NGX_HTTP_LOG_PHASE] -> handlers 

Tested on nginx version: nginx/1.18.0

This module is modified from https://github.com/vgo0/nginx-backdoor

## Usage

```
curl -H "check: on" <c2 address>

```

## Compile

```
sudo ./configure --add-dynamic-module=<module path> 
# for self-compiled version

sudo ./configure --add-dynamic-module=<module path> --with-compat # for compiled version

sudo make modules
#Compile module

```

## Install
```
sudo cp objs/ngx_http_check_headers_module.so <nginx_module_path>

load_module <path_to_your_module>/ngx_http_check_headers_module.so;
# Modify nginx.conf

sudo nginx -s reload
# Reload the configuration file without disconnecting current established connections
```
## Example Result

```

```
