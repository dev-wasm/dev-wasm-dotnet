#ifndef __WASI_HTTP_H__
#define __WASI_HTTP_H__

#include <stdint.h>

#define GET WASI_HTTP_TYPES_METHOD_GET
#define POST WASI_HTTP_TYPES_METHOD_POST
#define PUT WASI_HTTP_TYPES_METHOD_PUT
#define DELETE WASI_HTTP_TYPES_METHOD_DELETE
#define HEAD WASI_HTTP_TYPES_METHOD_HEAD
#define OPTIONS WASI_HTTP_TYPES_METHOD_OPTIONS

#define HTTPS WASI_HTTP_TYPES_SCHEME_HTTPS
#define HTTP WASI_HTTP_TYPES_SCHEME_HTTP


typedef struct {
    const char* authority;
    const char* path_query;
    const char* body;
} wasi_http_request_t;

typedef struct {
    char *name;
    char *value;
} header_t;

typedef struct {
    header_t* headers;
    uint16_t len;
} header_list_t;

typedef struct {
    uint16_t status_code;

    // Body should be non-null and owned by the caller
    // Will be filled in and '\0' terminated after call.
    char* body;
    // Body length is the maximum length for the body
    uint32_t body_max_len;

    // Header memory is owned by the caller after the call
    // and must be freed to prevent a leak.
    header_list_t headers;    
} wasi_http_response_t;

typedef void (*handler_func)(wasi_http_request_t* req, wasi_http_response_t *res);
extern handler_func handler;

void free_response(wasi_http_response_t* response);
int wasi_http_request(uint8_t method_tag, uint8_t scheme_tag, const char * authority_str, const char* path_query_str, const char* body, wasi_http_response_t *response_out);

#endif // __WASI_HTTP_H__
