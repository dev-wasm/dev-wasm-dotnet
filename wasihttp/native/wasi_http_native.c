#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <mono-wasi/driver.h>
#include "wasi_http.h"
#include "dotnet_method.h"

DEFINE_DOTNET_METHOD(serve_http, "wasihttp.dll", "", "Binder", "ServeHTTP");

void print_headers(header_list_t* header_list) {
    for (int i = 0; i < header_list->len; i++) {
        printf("%s: %s\n", header_list->headers[i].name, header_list->headers[i].value);
    }
}

// Temporary hack
#define WASI_HTTP_TYPES_METHOD_GET 0
#define WASI_HTTP_TYPES_METHOD_POST 1
#define WASI_HTTP_TYPES_METHOD_PUT 2
#define WASI_HTTP_TYPES_METHOD_DELETE 3
#define WASI_HTTP_TYPES_METHOD_HEAD 4
#define WASI_HTTP_TYPES_METHOD_OPTIONS 5

const char* str_for_method(uint32_t tag) {
    switch (tag) {
        case GET: return "GET";
        case POST: return "POST";
        case PUT: return "PUT";
        case DELETE: return "DELETE";
        // case PATCH: return "PATCH";
        case HEAD: return "HEAD";
        case OPTIONS: return "OPTIONS";
        // case CONNECT: return "CONNECT";
        // case TRACE: return "TRACE";
    }
    return "unknown";
}

static int count = 0;
bool exports_wasi_cli_run_run(void);

void dotnet_handler(wasi_http_request_t* req, wasi_http_response_t *res) {
    // This is necessary to initialize dotnet. Figure out a better way to do this.
    exports_wasi_cli_run_run();
    
    MonoString *authority_string = mono_wasm_string_from_js(req->authority);
    MonoString* path_string = mono_wasm_string_from_js(req->path_query);
    // TODO: FIX THIS!
    MonoString* method_string = mono_wasm_string_from_js(str_for_method(0));

    // TODO: FIX THESE HANDLES!
    void* method_params[] = { &req, &res, authority_string, path_string, method_string };
    serve_http(NULL, method_params);
}

void create_response(uint32_t res_ptr, uint32_t status_code, MonoArray* headers, MonoArray* header_values, MonoString* mono_body) {
    const char* body_str = mono_wasm_string_get_utf8(mono_body);

    uint32_t header_count = mono_array_length(headers);
    wasi_http_response_t *res = (wasi_http_response_t*) res_ptr;
    res->status_code = status_code;
    res->headers.headers = malloc(sizeof(header_t) * header_count);
    res->headers.len = header_count;

    for (int i = 0; i < header_count; i++) {
        MonoString* name = mono_array_get(headers, MonoString*, i);
        MonoString* value = mono_array_get(header_values, MonoString*, i);
        char* name_str = mono_wasm_string_get_utf8(name);
        char* value_str = mono_wasm_string_get_utf8(value);

        res->headers.headers[i].name = name_str;
        res->headers.headers[i].value = value_str;
    }
    res->body = body_str;
}

int request(uint8_t method_tag, uint8_t scheme_tag, const char * authority_str, const char* path_query_str, const char* body, int *status_code, int *handle) {
    // TODO
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
    // TODO
    return NULL;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))

MonoString* http_read_headers_all(int handle) {
    char buffer[1024 * 1024];
    size_t written = 0;

    // TODO

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
    // TODO
}

void wasi_http_attach_internal_calls() {
    exports_wasi_cli_run_run();
    handler = (handler_func) dotnet_handler;

    // Workaround lack of threading
    mono_add_internal_call("System.Threading.TimerQueue::SetTimeout", noop_settimeout);
    mono_add_internal_call("System.Threading.ThreadPool::QueueCallback", wasi_queuecallback);

    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::Req", http_req);
    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::Close", handle_close);
    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::ReadBody", http_read_body);
    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::ReadAllHeaders", http_read_headers_all);

    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::CreateResponse", create_response);
}

bool exports_wasi_cli_run_run(void) {
    // TODO: actually get arguments here.
    int argc = 1;
    char argv[] = {
        "dotnet"
    };
    //return !main(argc, (char**) &argv);
    return true;
}