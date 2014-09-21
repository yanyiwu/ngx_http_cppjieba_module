extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
}

#include "CppJieba/MixSegment.hpp"

using std::string;
using std::vector;

//static ngx_str_t g_cppjieba_conf_arg1;
//static ngx_str_t g_cppjieba_conf_arg2;

inline unsigned char fromHex(unsigned char x) 
{
    return isdigit(x) ? x - '0' : x - 'A' + 10;
}
/*
inline unsigned char toHex(unsigned char x)
{
    return x > 9 ? x -10 + 'A': x + '0';
}
inline void URLEncode(const string &sIn, string& sOut)
{
    for( size_t ix = 0; ix < sIn.size(); ix++ )
    {      
        unsigned char buf[4];
        memset( buf, 0, 4 );
        if( isalnum( (unsigned char)sIn[ix] ) )
        {      
            buf[0] = sIn[ix];
        }
        else
        {
            buf[0] = '%';
            buf[1] = toHex( (unsigned char)sIn[ix] >> 4 );
            buf[2] = toHex( (unsigned char)sIn[ix] % 16);
        }
        sOut += (char *)buf;
    }
};
*/

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
CppJieba::MixSegment * g_mix_segment;//(DICT_PATH, HMM_PATH, USER_DICT_PATH);

typedef struct {
    ngx_str_t output_words;
} ngx_http_cppjieba_loc_conf_t;

// To process HelloWorld command arguments
static char* ngx_http_cppjieba_set_conf(ngx_conf_t* cf, ngx_command_t* cmd, void* conf);

// Allocate memory for HelloWorld command
static void* ngx_http_cppjieba_create_loc_conf(ngx_conf_t* cf);

// Copy HelloWorld argument to another place
static char* ngx_http_cppjieba_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child);

//static ngx_int_t ngx_http_cppjieba_init(ngx_cycle_t *cf);
//static void ngx_http_cppjieba_finalize(ngx_cycle_t *cf);

static ngx_int_t get_post_content(ngx_http_request_t *r, char * data_buf, size_t content_length);
// Structure for the HelloWorld command
static ngx_command_t ngx_http_cppjieba_commands[] = {
    {
        ngx_string("cppjieba"), // The command name
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE2,
        ngx_http_cppjieba_set_conf, // The command handler
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_cppjieba_loc_conf_t, output_words),
        NULL
    },
    ngx_null_command
};

// Structure for the HelloWorld context
static ngx_http_module_t ngx_http_cppjieba_module_ctx = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ngx_http_cppjieba_create_loc_conf,
    ngx_http_cppjieba_merge_loc_conf
};

// Structure for the HelloWorld module, the most important thing
ngx_module_t ngx_http_cppjieba_module = {
    NGX_MODULE_V1,
    &ngx_http_cppjieba_module_ctx,
    ngx_http_cppjieba_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL, //ngx_http_cppjieba_init,
    NULL,
    NULL,
    NULL, //ngx_http_cppjieba_finalize,
    NULL,
    NGX_MODULE_V1_PADDING
};

static void ngx_http_cppjieba_post_handler(ngx_http_request_t* r);

static ngx_int_t ngx_http_cppjieba_handler(ngx_http_request_t* r) {
    ngx_int_t rc;
    ngx_buf_t* b;
    ngx_chain_t out;

    if(r->method & NGX_HTTP_POST) {
        ngx_int_t rc = ngx_http_read_client_request_body(r, ngx_http_cppjieba_post_handler);
        if (rc >= NGX_HTTP_SPECIAL_RESPONSE) {
            return rc;
        }
        return NGX_DONE;
    }

    if(!(r->method & NGX_HTTP_GET)) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    // args is s=xxxxx
    if(r->args.len < 2) {
        return NGX_HTTP_BAD_REQUEST;
    }

    string args((const char*)(r->args.data + 2), r->args.len - 2);
    string sentence;
    URLDecode(args, sentence);
    vector<string> words;
    g_mix_segment->cut(sentence, words);
    string response;
    //string tmp;
    //tmp << words;
    //URLEncode(tmp, response);
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

static void* ngx_http_cppjieba_create_loc_conf(ngx_conf_t* cf) {
    ngx_http_cppjieba_loc_conf_t* conf;

    conf = (ngx_http_cppjieba_loc_conf_t*)ngx_pcalloc(cf->pool, sizeof(ngx_http_cppjieba_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    conf->output_words.len = 0;
    conf->output_words.data = NULL;

    return conf;
}

static char* ngx_http_cppjieba_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child) {
    ngx_http_cppjieba_loc_conf_t* prev = (ngx_http_cppjieba_loc_conf_t*)parent;
    ngx_http_cppjieba_loc_conf_t* conf = (ngx_http_cppjieba_loc_conf_t*)child;
    ngx_conf_merge_str_value(conf->output_words, prev->output_words, "Nginx");
    return NGX_CONF_OK;
}

static char* ngx_http_cppjieba_set_conf(ngx_conf_t* cf, ngx_command_t* cmd, void* conf) {
    ngx_http_core_loc_conf_t* clcf;
    clcf = (ngx_http_core_loc_conf_t*)ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_cppjieba_handler;
    ngx_conf_set_str_slot(cf, cmd, conf);
    if (cf->args->nelts != 3) {
        ngx_log_error(NGX_LOG_ERR, cf->log, 0, " [the number of conf'a args is not 3] ");
        return (char*)NGX_CONF_ERROR;
    }
    ngx_str_t * value = (ngx_str_t *)cf->args->elts;

    g_mix_segment = new CppJieba::MixSegment(
                string((const char *)value[1].data, value[1].len), 
                string((const char *)value[2].data, value[2].len));
    return NGX_CONF_OK;
}

//static ngx_int_t ngx_http_cppjieba_init(ngx_cycle_t *cf) 
//{
//    g_mix_segment = new CppJieba::MixSegment(
//                string((const char *)g_cppjieba_conf_arg1.data, g_cppjieba_conf_arg1.len), 
//                string((const char *)g_cppjieba_conf_arg2.data, g_cppjieba_conf_arg2.len));
//    return NGX_OK;
//}

//static void ngx_http_cppjieba_finalize(ngx_cycle_t *cf)
//{
//    delete g_mix_segment;
//    g_mix_segment = NULL;
//}

static ngx_int_t get_post_content(ngx_http_request_t *r, char * data_buf, size_t content_length) {
    ngx_log_error(NGX_LOG_NOTICE, r->connection->log, 0, "[get_post_content] [content_length:%d]", content_length); //DEBUG
    if(r->request_body == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "reqeust_body:null");
        return NGX_ERROR;
    }
    ngx_chain_t* bufs = r->request_body->bufs;
    ngx_buf_t* buf = NULL;
    size_t body_length = 0;
    size_t buf_length;
    while(bufs) {
        buf = bufs->buf;
        bufs = bufs->next;
        buf_length = buf->last - buf->pos;
        if(body_length + buf_length > content_length) {
            memcpy(data_buf + body_length, buf->pos, content_length - body_length);
            body_length = content_length;
            break;
        }
        memcpy(data_buf + body_length, buf->pos, buf->last - buf->pos);
        body_length += buf->last - buf->pos;
    }
    if(body_length != content_length) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "get_post_content's body_length != content_length in headers");
        return NGX_ERROR;
    }
    return NGX_OK;
}


static ngx_int_t ngx_http_cppjieba_send_response(ngx_http_request_t * r, const char* type, const char* data_buf, size_t len) {
    ngx_int_t rc;
    ngx_buf_t* b;
    ngx_chain_t out;

    b = ngx_create_temp_buf(r->pool, len);
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ngx_memcpy(b->pos, data_buf, len);
    b->last = b->pos + len;
    b->last_buf = 1;

    out.buf = b;
    out.next = NULL;

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = len;
    r->headers_out.content_type.data = (u_char*) type;
    r->headers_out.content_type.len = strlen(type);

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    return ngx_http_output_filter(r, &out);
}

static void ngx_http_cppjieba_post_handler(ngx_http_request_t* r) {
    if(r->headers_in.content_length_n == 0) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "r->headers_in.content_length_n is 0");
        ngx_http_finalize_request(r, NGX_ERROR);
        return;
    }
    ngx_int_t rc;
    char * data_buf = NULL;
    data_buf = (char*) ngx_pcalloc(r->pool, r->headers_in.content_length_n + 1);
    if (data_buf == NULL) {
        ngx_http_finalize_request(r, NGX_ERROR);
        return;
    }

    if (NGX_ERROR == get_post_content(r, data_buf, r->headers_in.content_length_n)) {
        ngx_http_finalize_request(r, NGX_ERROR);
        return;
    }

    string sentence;
    URLDecode(data_buf, sentence);
    vector<string> words;
    g_mix_segment->cut(sentence, words);
    string response;
    response << words;


    rc = ngx_http_cppjieba_send_response(
                r, 
                "text/plain", 
                response.c_str(), 
                response.size());

    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, "[ngx_http_cppjieba_send_response] [response size:%d]", response.size());
    ngx_http_finalize_request(r, rc);
}
