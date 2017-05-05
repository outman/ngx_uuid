# dm_gen_uuid
Nginx uuid module based on libuuid.

# required 
nginx-1.10.2+
libuuid-1.0.3+

# compile for nginx

- cd nginx source compile path
- ./configure --prefix=/your/nginx/install/path
  --add-module=/youer/ngx_uuid_module/path

# uuid mode

- normal
- random
- time

# config nginx.conf

```c
    location /uuid {
        dm_gen_uuid normal;
    }
```

