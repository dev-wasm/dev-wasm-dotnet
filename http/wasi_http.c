#include <assert.h>
#include <string.h>
#include <mono-wasi/driver.h>
#include "proxy.h"

// This is a dummy required export to link, but unused.
void exports_wasi_http_incoming_handler_handle(uint32_t arg, uint32_t arg0) {
}

int request(uint8_t method_tag, uint8_t scheme_tag, const char * authority_str, const char* path_with_query_str, const char* body, int *status_code, int *handle) {
    proxy_tuple2_string_string_t content_type[] = {{
        .f0 = { .ptr = "User-agent", .len = 10 },
        .f1 = { .ptr = "WASI-HTTP/0.0.1", .len = 15},
    },
    {
        .f0 = { .ptr = "Content-type", .len = 12 },
        .f1 = { .ptr = "application/json", .len = 16},
    }};
    proxy_list_tuple2_string_string_t headers_list = {
        .ptr = &content_type[0],
        .len = 2,
    };
    wasi_http_types_fields_t headers = wasi_http_types_new_fields(&headers_list);
    wasi_http_types_method_t method = { .tag = method_tag };
    wasi_http_types_scheme_t scheme = { .tag = scheme_tag };
    proxy_string_t path, authority, query;
    proxy_string_set(&path, path_with_query_str);
    proxy_string_set(&authority, authority_str);

    wasi_http_outgoing_handler_outgoing_request_t req = wasi_http_types_new_outgoing_request(&method, &path, &scheme, &authority, headers);
    wasi_http_outgoing_handler_future_incoming_response_t res;

    if (req == 0) {
        printf("Error creating request\n");
        return 4;
    }
    if (body != NULL) {
        wasi_http_types_outgoing_stream_t ret;
        if (!wasi_http_types_outgoing_request_write(req, &ret)) {
            printf("Error getting output stream\n");
            return 7;
        }
        proxy_list_u8_t buf = {
            .ptr = (uint8_t *) body,
            .len = strlen(body),
        };
        uint64_t ret_val;
        wasi_io_streams_write(ret, &buf, &ret_val, NULL);
    }

    res = wasi_http_outgoing_handler_handle(req, NULL);
    if (res == 0) {
        printf("Error sending request\n");
        return 5;
    }
    
    proxy_result_incoming_response_error_t result;
    if (!wasi_http_types_future_incoming_response_get(res, &result)) {
        printf("failed to get value for incoming request\n");
        return 1;
    }

    if (result.is_err) {
        printf("response is error!\n");
        return 2;
    }
    // poll_drop_pollable(res);

    wasi_http_types_status_code_t code = wasi_http_types_incoming_response_status(result.val.ok);
    *status_code = code;

    *handle = result.val.ok;

    wasi_http_types_drop_outgoing_request(req);

    return 0;
}

int http_req(int32_t method, int32_t scheme, MonoString *authority, MonoString *pathWithQuery, MonoString *headers, MonoString *body, int *status_code, int *handle) {
    char* authority_utf8 = mono_wasm_string_get_utf8(authority);
    char* path_with_query_utf8 = mono_wasm_string_get_utf8(pathWithQuery);
    char* headers_utf8 = mono_wasm_string_get_utf8(headers);
    char* body_utf8 = mono_wasm_string_get_utf8(body);
    
    request(method, scheme, authority_utf8, path_with_query_utf8, body_utf8, status_code, handle);

    free(authority_utf8);
    free(path_with_query_utf8);
    free(headers_utf8);
    free(body_utf8);

    return 0;
}

MonoString* http_read_body(int handle) {
    wasi_http_types_incoming_stream_t stream;
    if (!wasi_http_types_incoming_response_consume(handle, &stream)) {
        printf("stream is error!\n");
        return NULL;
    }

    int32_t len = 64 * 1024;
    proxy_tuple2_list_u8_bool_t body_res;
    wasi_io_streams_stream_error_t err;
    if (!wasi_io_streams_read(stream, len, &body_res, &err)) {
        printf("BODY read is error!\n");
        return NULL;
    }
    printf("data from read: %s\n", body_res.f0.ptr);
    MonoString* result = mono_wasm_string_from_js((const char*)body_res.f0.ptr);
    proxy_tuple2_list_u8_bool_free(&body_res);

    return result;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))

MonoString* http_read_headers_all(int handle) {
    char buffer[1024 * 1024];
    size_t written = 0;

    wasi_http_types_headers_t header_handle = wasi_http_types_incoming_response_headers(handle);
    proxy_list_tuple2_string_list_u8_t header_list;
    wasi_http_types_fields_entries(header_handle, &header_list);

    for (int i = 0; i < header_list.len && written < 1024 * 1023; i++) {
        char name[128];
        char value[128];
        strncpy(name, header_list.ptr[i].f0.ptr, MIN(127, header_list.ptr[i].f0.len));
        name[header_list.ptr[i].f0.len] = 0;
        strncpy(value, (const char*) header_list.ptr[i].f1.ptr, MIN(127, header_list.ptr[i].f1.len));
        value[header_list.ptr[i].f1.len] = 0;
        written += sprintf(&(buffer[written]), "%s: %s\n", name, value);
    }
    buffer[written] = 0;
    
    return mono_wasm_string_from_js(&buffer[0]);
}

void noop_settimeout(int timeout) {
    // Not implemented
}

MonoMethod *threadpool_callback;

void wasi_queuecallback() {
    if (!threadpool_callback) {
        threadpool_callback = lookup_dotnet_method("System.Private.CoreLib.dll", "System.Threading", "ThreadPool", "Callback", -1);
        assert(threadpool_callback);
    }

    MonoObject* exception;
    MonoObject* res = mono_wasm_invoke_method(threadpool_callback, NULL, NULL, &exception);
    assert(!exception);
}

void handle_close(int handle) {
    wasi_http_types_drop_incoming_response(handle);
}

void wasi_http_attach_internal_calls() {
    // Workaround lack of threading
    mono_add_internal_call("System.Threading.TimerQueue::SetTimeout", noop_settimeout);
    mono_add_internal_call("System.Threading.ThreadPool::QueueCallback", wasi_queuecallback);

    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::Req", http_req);
    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::Close", handle_close);
    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::ReadBody", http_read_body);
    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::ReadAllHeaders", http_read_headers_all);
}