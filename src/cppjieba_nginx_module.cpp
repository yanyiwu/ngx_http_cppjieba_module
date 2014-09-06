extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
}

#include "dict_path.h"
#include "CppJieba/MixSegment.hpp"

using std::string;
using std::vector;

inline unsigned char fromHex(unsigned char x) 
{
    return isdigit(x) ? x - '0' : x - 'A' + 10;
}

static void URLDecode(const string &sIn, string& sOut)
{
    for( size_t ix = 0; ix < sIn.size(); ix++ )
    {
        unsigned char ch = 0;
        if(sIn[ix]=='%')
        {
            ch = (fromHex(sIn[ix+1])<<4);
            ch |= fromHex(sIn[ix+2]);
            ix += 2;
        }
        else if(sIn[ix] == '+')
        {
            ch = ' ';
        }
        else
        {
            ch = sIn[ix];
        }
        sOut += (char)ch;
    }
}
CppJieba::MixSegment * mix_segment;//(DICT_PATH, HMM_PATH, USER_DICT_PATH);

typedef struct {
    ngx_str_t output_words;
} ngx_cppjieba_loc_conf_t;

// To process HelloWorld command arguments
static char* ngx_cppjieba(ngx_conf_t* cf, ngx_command_t* cmd, void* conf);

// Allocate memory for HelloWorld command
static void* ngx_cppjieba_create_loc_conf(ngx_conf_t* cf);

// Copy HelloWorld argument to another place
static char* ngx_cppjieba_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child);

static ngx_int_t ngx_cppjieba_init(ngx_conf_t *cf) ;

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
    ngx_cppjieba_init,  //NULL,
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

/*
static void print_debug_file(const std::string& str) {
    FILE * fp;
    fp = fopen("/tmp/nginx_debug.log", "a");
    fprintf(fp, "%s\n", str.c_str());
    fclose(fp);
}
*/

static ngx_int_t ngx_cppjieba_handler(ngx_http_request_t* r) {
    ngx_int_t rc;
    ngx_buf_t* b;
    ngx_chain_t out;

    if(!(r->method & NGX_HTTP_GET)) {
        return NGX_HTTP_NOT_ALLOWED;
    }


    std::string uri((const char*)r->uri.data, r->uri.len);
    if(r->args.len == 0) {
        return NGX_HTTP_BAD_REQUEST;
    }
    string args((const char*)r->args.data, r->args.len);
    string sentence;
    URLDecode(args, sentence);
    vector<string> words;
    mix_segment->cut(sentence, words);
    string response;
    response << words;

    b = ngx_create_temp_buf(r->pool, response.size());
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ngx_memcpy(b->pos, response.c_str(), response.size());
    b->last = b->pos + response.size();
    b->last_buf = 1;

    out.buf = b;
    out.next = NULL;

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = response.size();
    ngx_str_t type = ngx_string("text/plain");
    r->headers_out.content_type = type;

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

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

static ngx_int_t ngx_cppjieba_init(ngx_conf_t *cf) 
{
    mix_segment = new CppJieba::MixSegment(DICT_PATH, HMM_PATH, USER_DICT_PATH);
    return NGX_OK;
}
