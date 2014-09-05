extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
}

static const char* RESPONSE_STRING = "hello cppjieba nginx module.";

typedef struct {
    ngx_str_t output_words;
} ngx_cppjieba_loc_conf_t;

// To process HelloWorld command arguments
static char* ngx_cppjieba(ngx_conf_t* cf, ngx_command_t* cmd, void* conf);

// Allocate memory for HelloWorld command
static void* ngx_cppjieba_create_loc_conf(ngx_conf_t* cf);

// Copy HelloWorld argument to another place
static char* ngx_cppjieba_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child);

// Structure for the HelloWorld command
static ngx_command_t ngx_cppjieba_commands[] = {
    {
        ngx_string("cppjieba"), // The command name
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_cppjieba, // The command handler
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_cppjieba_loc_conf_t, output_words),
        NULL
    },
    ngx_null_command
};

// Structure for the HelloWorld context
static ngx_http_module_t cppjieba_nginx_module_ctx = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ngx_cppjieba_create_loc_conf,
    ngx_cppjieba_merge_loc_conf
};

// Structure for the HelloWorld module, the most important thing
ngx_module_t cppjieba_nginx_module = {
    NGX_MODULE_V1,
    &cppjieba_nginx_module_ctx,
    ngx_cppjieba_commands,
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

static void print_debug_file(const char * str) {
    FILE * fp;
    fp = fopen("/tmp/nginx_debug.log", "a");
    fprintf(fp, "%s\n", str);
    fclose(fp);
}

static ngx_int_t ngx_cppjieba_handler(ngx_http_request_t* r) {
    print_debug_file("69");
    ngx_int_t rc;
    ngx_buf_t* b;
    ngx_chain_t out;

    //ngx_cppjieba_loc_conf_t* hlcf;
    //hlcf = ngx_http_get_module_loc_conf(r, cppjieba_nginx_module);

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = strlen(RESPONSE_STRING);
    ngx_str_t type = ngx_string("text/plain");
    r->headers_out.content_type = type;
    //r->headers_out.content_type.len = sizeof("text/plain") - 1;
    //r->headers_out.content_type.data = (u_char*)"text/plain";

    if (r->method == NGX_HTTP_GET) {
        size_t i;
        FILE * fp;
        fp = fopen("/tmp/nginx_debug.log", "a");
        fprintf(fp, "method is get. ");
        for(i = 0; i < r->uri.len; i++) {
            fprintf(fp, "%c", r->uri.data[i]);
        }
        fprintf(fp, "\n");
        fclose(fp);
    }

    print_debug_file("94");
    b = (ngx_buf_t*)ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    out.buf = b;
    out.next = NULL;

    b->pos = (u_char*)RESPONSE_STRING;
    b->last = b->pos + strlen(RESPONSE_STRING);
    b->memory = 1;
    b->last_buf = 1;

    //b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    //out[1].buf = b;
    //out[1].next = NULL;

    //b->pos = hlcf->output_words.data;
    //b->last = hlcf->output_words.data + (hlcf->output_words.len);
    //b->memory = 1;
    //b->last_buf = 1;

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    print_debug_file("122");
    return ngx_http_output_filter(r, &out);
}

static void* ngx_cppjieba_create_loc_conf(ngx_conf_t* cf) {
    ngx_cppjieba_loc_conf_t* conf;

    conf = (ngx_cppjieba_loc_conf_t*)ngx_pcalloc(cf->pool, sizeof(ngx_cppjieba_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    conf->output_words.len = 0;
    conf->output_words.data = NULL;

    return conf;
}

static char* ngx_cppjieba_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child) {
    ngx_cppjieba_loc_conf_t* prev = (ngx_cppjieba_loc_conf_t*)parent;
    ngx_cppjieba_loc_conf_t* conf = (ngx_cppjieba_loc_conf_t*)child;
    ngx_conf_merge_str_value(conf->output_words, prev->output_words, "Nginx");
    return NGX_CONF_OK;
}

static char* ngx_cppjieba(ngx_conf_t* cf, ngx_command_t* cmd, void* conf) {
    ngx_http_core_loc_conf_t* clcf;
    clcf = (ngx_http_core_loc_conf_t*)ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_cppjieba_handler;
    ngx_conf_set_str_slot(cf, cmd, conf);
    return NGX_CONF_OK;
}
