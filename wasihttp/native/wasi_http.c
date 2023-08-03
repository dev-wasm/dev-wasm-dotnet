#include <assert.h>
#include <string.h>
#include <mono-wasi/driver.h>
#include "proxy.h"
#include "dotnet_method.h"

char buffer[1024];
__attribute__((import_module("types"), import_name("log-it")))
void _wasm_log(int32_t, int32_t);

void send_log() {
    _wasm_log((int32_t) buffer, strlen(buffer));
}

DEFINE_DOTNET_METHOD(serve_http, "wasihttp.dll", "", "Binder", "ServeHTTP");

void print_headers(types_headers_t header_handle) {
    types_list_tuple2_string_string_t header_list;
    types_fields_entries(header_handle, &header_list);

    for (int i = 0; i < header_list.len; i++) {
        char name[128];
        char value[128];
        strncpy(name, header_list.ptr[i].f0.ptr, header_list.ptr[i].f0.len);
        name[header_list.ptr[i].f0.len] = 0;
        strncpy(value, (const char*)header_list.ptr[i].f1.ptr, header_list.ptr[i].f1.len);
        value[header_list.ptr[i].f1.len] = 0;
        sprintf(buffer, "%s: %s\n", name, value);
        send_log();
    }
}

const char* str_for_method(types_method_t method) {
    switch (method.tag) {
        case TYPES_METHOD_GET: return "GET";
        case TYPES_METHOD_POST: return "POST";
        case TYPES_METHOD_PUT: return "PUT";
        case TYPES_METHOD_DELETE: return "DELETE";
        case TYPES_METHOD_PATCH: return "PATCH";
        case TYPES_METHOD_HEAD: return "HEAD";
        case TYPES_METHOD_OPTIONS: return "OPTIONS";
        case TYPES_METHOD_CONNECT: return "CONNECT";
        case TYPES_METHOD_TRACE: return "TRACE";
    }
    return "unknown";
}

static int count = 0;

MonoString* mono_wasm_string_from_proxy_string(proxy_string_t str) {
    // This is broken, but it avoids dynamic allocation
    assert(str.len < 1024);

    char buff[1024];
    strncpy(buff, str.ptr, str.len);
    buff[str.len] = 0;
    return mono_wasm_string_from_js(buff);
}

void http_handle(uint32_t req, uint32_t res) {
    proxy_string_t authority;
    types_incoming_request_authority(req, &authority);
    MonoString *authority_string = mono_wasm_string_from_proxy_string(authority);
    free(authority.ptr);

    proxy_string_t path;
    types_incoming_request_path(req, &path);
    MonoString* path_string = mono_wasm_string_from_proxy_string(path);
    free(path.ptr);
    
    types_method_t method;
    types_incoming_request_method(req, &method);
    MonoString* method_string = mono_wasm_string_from_js(str_for_method(method));

    // types_headers_t headers = types_incoming_request_headers(req);
    // print_headers(headers);

    void* method_params[] = { &req, &res, authority_string, path_string, method_string };
    serve_http(NULL, method_params);
}

void create_response(uint32_t res, uint32_t status_code, MonoArray* headers, MonoArray* header_values, MonoString* mono_body) {
    const char* body = mono_wasm_string_get_utf8(mono_body);

    uint32_t header_count = mono_array_length(headers);
    types_tuple2_string_string_t *header_arr = malloc(sizeof(types_tuple2_string_string_t) * header_count);
    types_list_tuple2_string_string_t headers_list = {
        .ptr = header_arr,
        .len = header_count,
    };
    for (int i = 0; i < header_count; i++) {
        MonoString* name = mono_array_get(headers, MonoString*, i);
        MonoString* value = mono_array_get(header_values, MonoString*, i);
        char* name_str = mono_wasm_string_get_utf8(name);
        char* value_str = mono_wasm_string_get_utf8(value);

        header_arr[i].f0.ptr = name_str;
        header_arr[i].f0.len = strlen(name_str) - 1;

        header_arr[i].f1.ptr = value_str;
        header_arr[i].f1.len = strlen(value_str) - 1;

        // TODO there's definitely memory leaks here.
    }

    types_fields_t out_headers = types_new_fields(&headers_list);

    types_outgoing_response_t response = types_new_outgoing_response(status_code, out_headers);

    types_result_outgoing_response_error_t res_err = {
        .is_err = false,
        .val = {
            .ok = response,
        },
    };
    if (!types_set_response_outparam(res, &res_err)) {
        sprintf(buffer, "Failed to set response outparam: %d -> %d\n", res, response);
        send_log();
    }

    types_outgoing_stream_t stream;
    if (!types_outgoing_response_write(response, &stream)) {
        sprintf(buffer, "Failed to get response\n");
        send_log();
    }

    streams_list_u8_t buf = {
        .ptr = (uint8_t *) body,
        .len = strlen(body),
    };
    uint64_t ret_val;
    streams_write(stream, &buf, &ret_val, NULL);

    types_drop_outgoing_response(res);
    types_drop_fields(out_headers);
}

int request(uint8_t method_tag, uint8_t scheme_tag, const char * authority_str, const char* path_str, const char* query_str, const char* body, int *status_code, int *handle) {
    types_tuple2_string_string_t content_type[] = {{
        .f0 = { .ptr = "User-agent", .len = 10 },
        .f1 = { .ptr = "WASI-HTTP/0.0.1", .len = 15},
    },
    {
        .f0 = { .ptr = "Content-type", .len = 12 },
        .f1 = { .ptr = "application/json", .len = 16},
    }};
    types_list_tuple2_string_string_t headers_list = {
        .ptr = &content_type[0],
        .len = 2,
    };
    types_fields_t headers = types_new_fields(&headers_list);
    types_method_t method = { .tag = method_tag };
    types_scheme_t scheme = { .tag = scheme_tag };
    proxy_string_t path, authority, query;
    proxy_string_set(&path, path_str);
    proxy_string_set(&authority, authority_str);
    proxy_string_set(&query, query_str);

    default_outgoing_http_outgoing_request_t req = types_new_outgoing_request(&method, &path, &query, &scheme, &authority, headers);
    default_outgoing_http_future_incoming_response_t res;

    if (req == 0) {
        printf("Error creating request\n");
        return 4;
    }
    if (body != NULL) {
        types_outgoing_stream_t ret;
        if (!types_outgoing_request_write(req, &ret)) {
            printf("Error getting output stream\n");
            return 7;
        }
        streams_list_u8_t buf = {
            .ptr = (uint8_t *) body,
            .len = strlen(body),
        };
        uint64_t ret_val;
        streams_write(ret, &buf, &ret_val, NULL);
    }

    res = default_outgoing_http_handle(req, NULL);
    if (res == 0) {
        printf("Error sending request\n");
        return 5;
    }
    
    types_result_incoming_response_error_t result;
    if (!types_future_incoming_response_get(res, &result)) {
        printf("failed to get value for incoming request\n");
        return 1;
    }

    if (result.is_err) {
        printf("response is error!\n");
        return 2;
    }
    // poll_drop_pollable(res);

    types_status_code_t code = types_incoming_response_status(result.val.ok);
    *status_code = code;

    *handle = result.val.ok;

    types_drop_outgoing_request(req);

    return 0;
}


int http_req(int32_t method, int32_t scheme, MonoString *authority, MonoString *path, MonoString *query, MonoString *headers, MonoString *body, int *status_code, int *handle) {
    char* authority_utf8 = mono_wasm_string_get_utf8(authority);
    char* path_utf8 = mono_wasm_string_get_utf8(path);
    char* query_utf8 = mono_wasm_string_get_utf8(query);
    char* headers_utf8 = mono_wasm_string_get_utf8(headers);
    char* body_utf8 = mono_wasm_string_get_utf8(body);
    
    request(method, scheme, authority_utf8, path_utf8, query_utf8, body_utf8, status_code, handle);

    free(authority_utf8);
    free(path_utf8);
    free(query_utf8);
    free(headers_utf8);
    free(body_utf8);

    return 0;
}

MonoString* http_read_body(int handle) {
    types_incoming_stream_t stream;
    if (!types_incoming_response_consume(handle, &stream)) {
        printf("stream is error!\n");
        return NULL;
    }

    int32_t len = 64 * 1024;
    streams_tuple2_list_u8_bool_t body_res;
    streams_stream_error_t err;
    if (!streams_read(stream, len, &body_res, &err)) {
        printf("BODY read is error!\n");
        return NULL;
    }
    printf("data from read: %s\n", body_res.f0.ptr);
    MonoString* result = mono_wasm_string_from_js((const char*)body_res.f0.ptr);
    streams_tuple2_list_u8_bool_free(&body_res);

    return result;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))

MonoString* http_read_headers_all(int handle) {
    char buffer[1024 * 1024];
    size_t written = 0;

    types_headers_t header_handle = types_incoming_response_headers(handle);
    types_list_tuple2_string_string_t header_list;
    types_fields_entries(header_handle, &header_list);

    for (int i = 0; i < header_list.len && written < 1024 * 1023; i++) {
        char name[128];
        char value[128];
        strncpy(name, header_list.ptr[i].f0.ptr, MIN(127, header_list.ptr[i].f0.len));
        name[header_list.ptr[i].f0.len] = 0;
        strncpy(value, header_list.ptr[i].f1.ptr, MIN(127, header_list.ptr[i].f1.len));
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
    types_drop_incoming_response(handle);
}

void wasi_http_attach_internal_calls() {
    // Workaround lack of threading
    mono_add_internal_call("System.Threading.TimerQueue::SetTimeout", noop_settimeout);
    mono_add_internal_call("System.Threading.ThreadPool::QueueCallback", wasi_queuecallback);

    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::Req", http_req);
    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::Close", handle_close);
    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::ReadBody", http_read_body);
    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::ReadAllHeaders", http_read_headers_all);

    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::CreateResponse", create_response);

}