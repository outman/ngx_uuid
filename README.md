# dm_gen_uuid
Nginx uuid module based on libuuid.

# Required 
- nginx-1.10.2+
- libuuid-1.0.3+

# Compile for nginx

- cd nginx source compile path
- ./configure --prefix=/your/nginx/install/path
  --add-module=/youer/ngx_uuid_module/path

# UUID mode

- normal
- random
- time

# Config nginx.conf

```c
location /uuid {
    dm_gen_uuid normal;
}
```

