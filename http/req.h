// This was hand-written, following the definition here:
// https://github.com/deislabs/wasi-experimental-http/blob/main/crates/as/raw.ts
// TODO: Machine generate this.

#include <stdlib.h>

typedef void* ResponseHandle;

typedef enum HttpError_t {
    SUCCESS = 0,
    INVALID_HANDLE,
    MEMORY_NOT_FOUND,
    MEMORY_ACCESS_ERROR,
    BUFFER_TOO_SMALL,
    HEADER_NOT_FOUND,
    UTF_8_ERROR,
    DESTINATION_NOT_ALLOWED,
    INVALID_METHOD,
    INVALID_ENCODING,
    INVALID_URL,
    REQUEST_ERROR,
    RUNTIME_ERROR,
    TOO_MANY_SESSIONS
} HttpError;

__attribute__((import_module("wasi_experimental_http"), import_name("req"))) HttpError req(
    const char* url_ptr,
    size_t url_len,
    const char* method_ptr,
    size_t method_len,
    const char* headers_ptr,
    size_t headers_len,
    const char* body_ptr,
    size_t body_len,
    uint16_t* status_code,
    ResponseHandle* response_handle
);

__attribute__((import_module("wasi_experimental_http"), import_name("close"))) HttpError http_close(ResponseHandle handle);

__attribute__((import_module("wasi_experimental_http"), import_name("body_read"))) HttpError bodyRead(
    ResponseHandle handle,
    char* body_buffer,
    size_t body_buffer_len,
    size_t* written_bytes);

__attribute__((import_module("wasi_experimental_http"), import_name("header_get"))) HttpError getHeader(
    ResponseHandle handle,
    char* header_name,
    size_t header_name_len,
    char* header_value_buffer,
    size_t header_value_buffer_len,
    size_t* written_bytes);

__attribute__((import_module("wasi_experimental_http"), import_name("headers_get_all"))) HttpError getAllHeaders(
    ResponseHandle handle,
    char* header_value_buffer,
    size_t header_value_buffer_len,
    size_t* written_bytes);