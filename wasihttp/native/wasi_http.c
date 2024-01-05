#include <assert.h>
#include <string.h>
#include <mono-wasi/driver.h>
#include "wasi.h"
#include "dotnet_method.h"

DEFINE_DOTNET_METHOD(serve_http, "wasihttp.dll", "", "Binder", "ServeHTTP");

void print_headers(wasi_http_0_2_0_rc_2023_11_10_types_own_headers_t header_handle) {
    wasi_http_0_2_0_rc_2023_11_10_types_list_tuple2_field_key_field_value_t header_list;
    wasi_http_0_2_0_rc_2023_11_10_types_method_fields_entries(wasi_http_0_2_0_rc_2023_11_10_types_borrow_fields(header_handle), &header_list);

    for (int i = 0; i < header_list.len; i++) {
        char name[128];
        char value[128];
        strncpy(name, header_list.ptr[i].f0.ptr, header_list.ptr[i].f0.len);
        name[header_list.ptr[i].f0.len] = 0;
        strncpy(value, (const char*)header_list.ptr[i].f1.ptr, header_list.ptr[i].f1.len);
        value[header_list.ptr[i].f1.len] = 0;
        printf("%s: %s\n", name, value);
    }
}

const char* str_for_method(wasi_http_0_2_0_rc_2023_11_10_types_method_t method) {
    switch (method.tag) {
        case WASI_HTTP_0_2_0_RC_2023_11_10_TYPES_METHOD_GET: return "GET";
        case WASI_HTTP_0_2_0_RC_2023_11_10_TYPES_METHOD_POST: return "POST";
        case WASI_HTTP_0_2_0_RC_2023_11_10_TYPES_METHOD_PUT: return "PUT";
        case WASI_HTTP_0_2_0_RC_2023_11_10_TYPES_METHOD_DELETE: return "DELETE";
        case WASI_HTTP_0_2_0_RC_2023_11_10_TYPES_METHOD_PATCH: return "PATCH";
        case WASI_HTTP_0_2_0_RC_2023_11_10_TYPES_METHOD_HEAD: return "HEAD";
        case WASI_HTTP_0_2_0_RC_2023_11_10_TYPES_METHOD_OPTIONS: return "OPTIONS";
        case WASI_HTTP_0_2_0_RC_2023_11_10_TYPES_METHOD_CONNECT: return "CONNECT";
        case WASI_HTTP_0_2_0_RC_2023_11_10_TYPES_METHOD_TRACE: return "TRACE";
    }
    return "unknown";
}

static int count = 0;

MonoString* mono_wasm_string_from_proxy_string(wasi_string_t str) {
    // This is broken, but it avoids dynamic allocation
    assert(str.len < 1024);

    char buff[1024];
    strncpy(buff, str.ptr, str.len);
    buff[str.len] = 0;
    return mono_wasm_string_from_js(buff);
}

void exports_wasi_http_0_2_0_rc_2023_11_10_incoming_handler_handle(exports_wasi_http_0_2_0_rc_2023_11_10_incoming_handler_own_incoming_request_t req, exports_wasi_http_0_2_0_rc_2023_11_10_incoming_handler_own_response_outparam_t response_out) {
    // This is necessary to initialize dotnet. Figure out a better way to do this.
    exports_wasi_cli_0_2_0_rc_2023_11_10_run_run();
    
    wasi_string_t authority;
    wasi_http_0_2_0_rc_2023_11_10_types_method_incoming_request_authority(wasi_http_0_2_0_rc_2023_11_10_types_borrow_incoming_request(req), &authority);
    MonoString *authority_string = mono_wasm_string_from_proxy_string(authority);
    free(authority.ptr);

    wasi_string_t path_query;
    wasi_http_0_2_0_rc_2023_11_10_types_method_incoming_request_path_with_query(wasi_http_0_2_0_rc_2023_11_10_types_borrow_incoming_request(req), &path_query);
    MonoString* path_string = mono_wasm_string_from_proxy_string(path_query);
    free(path_query.ptr);
    
    wasi_http_0_2_0_rc_2023_11_10_types_method_t method;
    wasi_http_0_2_0_rc_2023_11_10_types_method_incoming_request_method(wasi_http_0_2_0_rc_2023_11_10_types_borrow_incoming_request(req), &method);
    MonoString* method_string = mono_wasm_string_from_js(str_for_method(method));

    void* method_params[] = { &req.__handle, &response_out.__handle, authority_string, path_string, method_string };

    serve_http(NULL, method_params);
}

void create_response(uint32_t res_ptr, uint32_t status_code, MonoArray* headers, MonoArray* header_values, MonoString* mono_body) {
    const char* body_str = mono_wasm_string_get_utf8(mono_body);

    uint32_t header_count = mono_array_length(headers);
    wasi_http_0_2_0_rc_2023_11_10_types_tuple2_field_key_field_value_t *header_arr = malloc(sizeof(wasi_http_0_2_0_rc_2023_11_10_types_tuple2_field_key_field_value_t) * header_count);
    wasi_http_0_2_0_rc_2023_11_10_types_list_tuple2_field_key_field_value_t headers_list = {
        .ptr = header_arr,
        .len = header_count,
    };
    for (int i = 0; i < header_count; i++) {
        MonoString* name = mono_array_get(headers, MonoString*, i);
        MonoString* value = mono_array_get(header_values, MonoString*, i);
        char* name_str = mono_wasm_string_get_utf8(name);
        char* value_str = mono_wasm_string_get_utf8(value);

        header_arr[i].f0.ptr = name_str;
        header_arr[i].f0.len = strlen(name_str);

        header_arr[i].f1.ptr = value_str;
        header_arr[i].f1.len = strlen(value_str);

        // TODO there's definitely memory leaks here.
    }

    wasi_http_0_2_0_rc_2023_11_10_types_own_headers_t f;
    wasi_http_0_2_0_rc_2023_11_10_types_header_error_t err;
    wasi_http_0_2_0_rc_2023_11_10_types_static_fields_from_list(&headers_list, &f, &err);

    wasi_http_0_2_0_rc_2023_11_10_types_own_outgoing_response_t res = wasi_http_0_2_0_rc_2023_11_10_types_constructor_outgoing_response(f);

    wasi_http_0_2_0_rc_2023_11_10_types_own_outgoing_body_t out_body;
    wasi_http_0_2_0_rc_2023_11_10_types_method_outgoing_response_body(wasi_http_0_2_0_rc_2023_11_10_types_borrow_outgoing_response(res), &out_body);

    wasi_http_0_2_0_rc_2023_11_10_types_own_response_outparam_t outparam = {
        .__handle = res_ptr,
    };
    wasi_http_0_2_0_rc_2023_11_10_types_result_own_outgoing_response_error_code_t res_err = {
        .is_err = false,
        .val = {
            .ok = res,
        },
    };
    wasi_http_0_2_0_rc_2023_11_10_types_static_response_outparam_set(outparam, &res_err);
    
    if (res_err.is_err) {
        printf("Failed to set response outparam: %d -> %d\n", res_err.val.err, res);
    }

    wasi_http_0_2_0_rc_2023_11_10_types_own_output_stream_t stream;
    if (!wasi_http_0_2_0_rc_2023_11_10_types_method_outgoing_body_write(wasi_http_0_2_0_rc_2023_11_10_types_borrow_outgoing_body(out_body), &stream)) {
        printf("Failed to get response\n");
    }

    wasi_io_0_2_0_rc_2023_11_10_streams_list_u8_t buf = {
        .ptr = (uint8_t *) body_str,
        .len = strlen(body_str),
    };
    wasi_io_0_2_0_rc_2023_11_10_streams_stream_error_t stream_err;
    wasi_io_0_2_0_rc_2023_11_10_streams_method_output_stream_blocking_write_and_flush(wasi_io_0_2_0_rc_2023_11_10_streams_borrow_output_stream(stream), &buf, &stream_err);

    wasi_io_0_2_0_rc_2023_11_10_streams_output_stream_drop_own(stream);

    wasi_http_0_2_0_rc_2023_11_10_types_error_code_t error_code;
    wasi_http_0_2_0_rc_2023_11_10_types_static_outgoing_body_finish(out_body, NULL, &error_code);	
}

int request(uint8_t method_tag, uint8_t scheme_tag, const char * authority_str, const char* path_query_str, const char* body, int *status_code, int *handle) {
    wasi_http_0_2_0_rc_2023_11_10_types_tuple2_field_key_field_value_t content_type[] = {{
        .f0 = { .ptr = "User-agent", .len = 10 },
        .f1 = { .ptr = "WASI-HTTP/0.0.1", .len = 15},
    },
    {
        .f0 = { .ptr = "Content-type", .len = 12 },
        .f1 = { .ptr = "application/json", .len = 16},
    }};
    wasi_http_0_2_0_rc_2023_11_10_types_list_tuple2_field_key_field_value_t headers_list = {
        .ptr = &content_type[0],
        .len = 2,
    };
    wasi_http_0_2_0_rc_2023_11_10_types_own_fields_t headers;
    wasi_http_0_2_0_rc_2023_11_10_types_header_error_t err;
    if (!wasi_http_0_2_0_rc_2023_11_10_types_static_fields_from_list(&headers_list, &headers, &err)) {
        printf("Header create failed\n");
    }

    wasi_http_0_2_0_rc_2023_11_10_types_method_t method = { .tag = method_tag };
    wasi_http_0_2_0_rc_2023_11_10_types_scheme_t scheme = { .tag = scheme_tag };
    wasi_string_t path_query, authority;
    wasi_string_set(&path_query, path_query_str);
    wasi_string_set(&authority, authority_str);

    wasi_http_0_2_0_rc_2023_11_10_types_own_outgoing_body_t out_body;

    wasi_http_0_2_0_rc_2023_11_10_types_own_outgoing_request_t req = wasi_http_0_2_0_rc_2023_11_10_types_constructor_outgoing_request(headers);
    bool ok = wasi_http_0_2_0_rc_2023_11_10_types_method_outgoing_request_set_method(wasi_http_0_2_0_rc_2023_11_10_types_borrow_outgoing_request(req), &method) &&
        wasi_http_0_2_0_rc_2023_11_10_types_method_outgoing_request_set_path_with_query(wasi_http_0_2_0_rc_2023_11_10_types_borrow_outgoing_request(req), &path_query) &&
        wasi_http_0_2_0_rc_2023_11_10_types_method_outgoing_request_set_scheme(wasi_http_0_2_0_rc_2023_11_10_types_borrow_outgoing_request(req), &scheme) &&
        wasi_http_0_2_0_rc_2023_11_10_types_method_outgoing_request_set_authority(wasi_http_0_2_0_rc_2023_11_10_types_borrow_outgoing_request(req), &authority) &&
        wasi_http_0_2_0_rc_2023_11_10_types_method_outgoing_request_body(wasi_http_0_2_0_rc_2023_11_10_types_borrow_outgoing_request(req), &out_body);

    if (!ok) {
        printf("Error creating request\n");
        return 4;
    }
    if (body != NULL) {
        wasi_http_0_2_0_rc_2023_11_10_types_own_output_stream_t ret;
        if (!wasi_http_0_2_0_rc_2023_11_10_types_method_outgoing_body_write(wasi_http_0_2_0_rc_2023_11_10_types_borrow_outgoing_body(out_body), &ret)) {
            printf("Error getting output stream\n");
            return 7;
        }
        wasi_io_0_2_0_rc_2023_11_10_streams_list_u8_t buf = {
            .ptr = (uint8_t *) body,
            .len = strlen(body),
        };
        wasi_io_0_2_0_rc_2023_11_10_streams_stream_error_t stream_err;
        // TODO check error here.
        wasi_io_0_2_0_rc_2023_11_10_streams_method_output_stream_blocking_write_and_flush(wasi_io_0_2_0_rc_2023_11_10_streams_borrow_output_stream(ret), &buf, &stream_err);
        wasi_io_0_2_0_rc_2023_11_10_streams_output_stream_drop_own(ret);
    }

    wasi_http_0_2_0_rc_2023_11_10_outgoing_handler_own_future_incoming_response_t ret;
    wasi_http_0_2_0_rc_2023_11_10_outgoing_handler_error_code_t handler_err;
    if (!wasi_http_0_2_0_rc_2023_11_10_outgoing_handler_handle(req, NULL, &ret, &handler_err)) {
        printf("Error sending request\n");
        return 2;
    }

    wasi_http_0_2_0_rc_2023_11_10_types_error_code_t finish_err;
    if (!wasi_http_0_2_0_rc_2023_11_10_types_static_outgoing_body_finish(out_body, NULL, &finish_err)) {
        printf("Failed to finish body\n");
        return 3;
    }

    wasi_http_0_2_0_rc_2023_11_10_types_own_pollable_t poll = wasi_http_0_2_0_rc_2023_11_10_types_method_future_incoming_response_subscribe(wasi_http_0_2_0_rc_2023_11_10_types_borrow_future_incoming_response(ret));
    wasi_io_0_2_0_rc_2023_11_10_poll_method_pollable_block(wasi_io_0_2_0_rc_2023_11_10_poll_borrow_pollable(poll));

    wasi_http_0_2_0_rc_2023_11_10_types_result_result_own_incoming_response_error_code_void_t result;
    if (!wasi_http_0_2_0_rc_2023_11_10_types_method_future_incoming_response_get(wasi_http_0_2_0_rc_2023_11_10_types_borrow_future_incoming_response(ret), &result)) {
        printf("failed to get value for incoming request\n");
        return 4;
    }

    if (result.is_err || result.val.ok.is_err) {
        printf("response is error!\n");
        return 5;
    }

    wasi_io_0_2_0_rc_2023_11_10_poll_pollable_drop_own(poll);
    wasi_http_0_2_0_rc_2023_11_10_types_future_incoming_response_drop_own(ret);

    wasi_http_0_2_0_rc_2023_11_10_types_own_incoming_response_t resp = result.val.ok.val.ok;

    wasi_http_0_2_0_rc_2023_11_10_types_status_code_t code = wasi_http_0_2_0_rc_2023_11_10_types_method_incoming_response_status(wasi_http_0_2_0_rc_2023_11_10_types_borrow_incoming_response(resp));
    *status_code = code;

    *handle = resp.__handle;

    return 0;
}


int http_req(int32_t method, int32_t scheme, MonoString *authority, MonoString *path_query, MonoString *headers, MonoString *body, int *status_code, int *handle) {
    char* authority_utf8 = mono_wasm_string_get_utf8(authority);
    char* path_query_utf8 = mono_wasm_string_get_utf8(path_query);
    char* headers_utf8 = mono_wasm_string_get_utf8(headers);
    char* body_utf8 = mono_wasm_string_get_utf8(body);
    
    request(method, scheme, authority_utf8, path_query_utf8, body_utf8, status_code, handle);

    free(authority_utf8);
    free(path_query_utf8);
    free(headers_utf8);
    free(body_utf8);

    return 0;
}

MonoString* http_read_body(int handle) {
    wasi_http_0_2_0_rc_2023_11_10_types_own_incoming_response_t resp = {
        .__handle = handle,
    };
 
    wasi_http_0_2_0_rc_2023_11_10_types_own_incoming_body_t in_body;
    if (!wasi_http_0_2_0_rc_2023_11_10_types_method_incoming_response_consume(wasi_http_0_2_0_rc_2023_11_10_types_borrow_incoming_response(resp), &in_body)) {
        printf("body is error!\n");
        return NULL;
    }
    wasi_io_0_2_0_rc_2023_11_10_streams_own_input_stream_t stream;
    if (!wasi_http_0_2_0_rc_2023_11_10_types_method_incoming_body_stream(wasi_http_0_2_0_rc_2023_11_10_types_borrow_incoming_body(in_body), &stream)) {
        printf("stream is error\n");
        return NULL;
    }

    wasi_io_0_2_0_rc_2023_11_10_streams_own_pollable_t stream_poll = wasi_io_0_2_0_rc_2023_11_10_streams_method_input_stream_subscribe(wasi_io_0_2_0_rc_2023_11_10_streams_borrow_input_stream(stream));

    int32_t len = 1024 * 1024;
    char buff[len + 1];
    int32_t offset = 0;
    while (true) {
        wasi_io_0_2_0_rc_2023_11_10_poll_method_pollable_block(wasi_io_0_2_0_rc_2023_11_10_poll_borrow_pollable(stream_poll));
        wasi_io_0_2_0_rc_2023_11_10_streams_list_u8_t body_res;
        wasi_io_0_2_0_rc_2023_11_10_streams_stream_error_t stream_err;
        if (wasi_io_0_2_0_rc_2023_11_10_streams_method_input_stream_read(wasi_io_0_2_0_rc_2023_11_10_streams_borrow_input_stream(stream), len, &body_res, &stream_err)) {
            strncpy(buff + offset, (char *) body_res.ptr, body_res.len);
            buff[offset + body_res.len] = 0;
            offset = offset + body_res.len;
        } else if (stream_err.tag == WASI_IO_0_2_0_RC_2023_11_10_STREAMS_STREAM_ERROR_CLOSED) {
            break;
        } else {
            printf("BODY read is error!\n");
            return 6;
        }
        wasi_io_0_2_0_rc_2023_11_10_streams_list_u8_free(&body_res);
    }
    MonoString* result = mono_wasm_string_from_js((const char*)buff);

    // wasi_http_0_2_0_rc_2023_11_10_types_outgoing_request_drop_own(req);
    // wasi_io_0_2_0_rc_2023_11_10_streams_input_stream_drop_own(stream);
    return result;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))

MonoString* http_read_headers_all(int handle) {
    char buffer[1024 * 1024];
    size_t written = 0;

    wasi_http_0_2_0_rc_2023_11_10_types_own_incoming_response_t resp = {
        .__handle = handle,
    };
    wasi_http_0_2_0_rc_2023_11_10_types_own_headers_t header_handle = wasi_http_0_2_0_rc_2023_11_10_types_method_incoming_response_headers(wasi_http_0_2_0_rc_2023_11_10_types_borrow_incoming_response(resp));
    wasi_http_0_2_0_rc_2023_11_10_types_list_tuple2_field_key_field_value_t header_list;
    wasi_http_0_2_0_rc_2023_11_10_types_method_fields_entries(wasi_http_0_2_0_rc_2023_11_10_types_borrow_fields(header_handle), &header_list);

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
    wasi_http_0_2_0_rc_2023_11_10_types_fields_drop_own(header_handle);
    
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
    wasi_http_0_2_0_rc_2023_11_10_types_own_incoming_response_t resp = {
        .__handle = handle,
    };
    wasi_http_0_2_0_rc_2023_11_10_types_incoming_response_drop_own(resp);
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

bool exports_wasi_cli_0_2_0_rc_2023_11_10_run_run(void) {
    // TODO: actually get arguments here.
    int argc = 1;
    char argv[] = {
        "dotnet"
    };
    return !main(argc, (char**) &argv);
}