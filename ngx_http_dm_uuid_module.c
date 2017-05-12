#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_log.h>

#include <uuid/uuid.h>

#ifndef DM_UUID_GEN_NORMAL
#define DM_UUID_GEN_NORMAL "normal"
#endif

#ifndef DM_UUID_GEN_RANDOM
#define DM_UUID_GEN_RANDOM "random"
#endif

#ifndef DM_UUID_GEN_TIME
#define DM_UUID_GEN_TIME "time"
#endif

/* loccation conf struct */
typedef struct {
    ngx_str_t dm_gen_uuid;
} ngx_http_dm_uuid_loc_conf_t;

/* moudule init callback function */
static ngx_int_t ngx_http_dm_uuid_init(ngx_conf_t *cf);

/* module create location conf callback function */
static void *ngx_http_dm_uuid_create_loc_conf(ngx_conf_t *cf);

/* module's configuration */
static char *ngx_http_dm_gen_uuid(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

/* module's commands config */
static ngx_command_t ngx_http_dm_uuid_commands[] = {
    {
        ngx_string("dm_gen_uuid"),
        NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS|NGX_CONF_TAKE1,
        ngx_http_dm_gen_uuid,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_dm_uuid_loc_conf_t, dm_gen_uuid),
        NULL
    },
    ngx_null_command
};

/* module context */
static ngx_http_module_t ngx_http_dm_uuid_module_ctx = {
    NULL,
    ngx_http_dm_uuid_init,
    NULL,
    NULL,
    NULL,
    NULL,
    ngx_http_dm_uuid_create_loc_conf,
    NULL
};

/* module config */
ngx_module_t ngx_http_dm_uuid_module = {
    NGX_MODULE_V1,
    &ngx_http_dm_uuid_module_ctx,
    ngx_http_dm_uuid_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING
};

/** module's handler **/
static ngx_int_t ngx_http_dm_uuid_handler(ngx_http_request_t *r)
{
    ngx_int_t    rc;
    ngx_buf_t   *b;
    ngx_chain_t  out;
    ngx_http_dm_uuid_loc_conf_t* local_conf;
    u_char dm_uuid_string[32] = {0};
    ngx_uint_t content_length = 0;

    local_conf = ngx_http_get_module_loc_conf(r, ngx_http_dm_uuid_module);
    if (local_conf->dm_gen_uuid.len == 0 ) {
        ngx_log_debug0(NGX_LOG_DEBUG_CORE, r->connection->log, 0, "dm_get_uuid_empty");
        return NGX_DECLINED;
    }

    uuid_t uuid_buf;
    if (ngx_strcmp(local_conf->dm_gen_uuid.data, DM_UUID_GEN_RANDOM) == 0) {
        uuid_generate_random(uuid_buf);
    } else if (ngx_strcmp(local_conf->dm_gen_uuid.data, DM_UUID_GEN_TIME) == 0) {
        uuid_generate_time(uuid_buf);
    } else {
        uuid_generate(uuid_buf);
    }

    uuid_unparse(uuid_buf, (char *) dm_uuid_string);
    uuid_clear(uuid_buf);

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, r->connection->log, 0, "dm_gen_uuid %s", dm_uuid_string);

    content_length = ngx_strlen(dm_uuid_string);

    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    /* discard client request body */
    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK) {
        return rc;
    }

    ngx_str_set(&r->headers_out.content_type, "text/html");

    /* send the header only, if the request type is http 'HEAD' */
    if (r->method == NGX_HTTP_HEAD) {
        r->headers_out.status = NGX_HTTP_OK;
        r->headers_out.content_length_n = content_length;

        return ngx_http_send_header(r);
    }

    /* allocate a buffer for your response body */
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    /* attach this buffer to the buffer chain */
    out.buf = b;
    out.next = NULL;

    /* adjust the pointers of the buffer */
    b->pos = dm_uuid_string;
    b->last = dm_uuid_string + content_length;
    b->memory = 1;    /* this buffer is in memory */
    b->last_buf = 1;  /* this is the last buffer in the buffer chain */

    /* set the status line */
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = content_length;

    /* send the headers of your response */
    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    /* send the buffer chain of your response */
    return ngx_http_output_filter(r, &out);
}

static void *ngx_http_dm_uuid_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_dm_uuid_loc_conf_t* local_conf = NULL;
    local_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_dm_uuid_loc_conf_t));
    if (local_conf == NULL) {
        return NULL;
    }

    ngx_str_null(&local_conf->dm_gen_uuid);
    return local_conf;
}

static char *ngx_http_dm_gen_uuid(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    char* rv = ngx_conf_set_str_slot(cf, cmd, conf);
    return rv;
}

static ngx_int_t ngx_http_dm_uuid_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_dm_uuid_handler;

    return NGX_OK;
}
