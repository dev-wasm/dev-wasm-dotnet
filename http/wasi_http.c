#include <assert.h>
#include <string.h>
#include <mono-wasi/driver.h>
#include "req.h"

int handle_close(int handle) {
    return http_close((ResponseHandle)handle);
}

int http_req_raw(const char* url, const char* method, const char* headers, const char* body, uint16_t *statusCode, ResponseHandle *handle) {
    return req(url, url ? strlen(url) : 0,
               method, method ? strlen(method) : 0,
               headers, headers ? strlen(headers) : 0,
               body, body ? strlen(body) : 0,
               statusCode, handle);
}

int http_req(MonoString *url, MonoString *method, MonoString *headers, MonoString *body, int *statusCode, int *handle) {
    char* url_utf8 = mono_wasm_string_get_utf8(url);
    char* method_utf8 = mono_wasm_string_get_utf8(method);
    char* headers_utf8 = mono_wasm_string_get_utf8(headers);
    char* body_utf8 = mono_wasm_string_get_utf8(body);
    
    uint16_t code;
    HttpError result = http_req_raw(url_utf8, method_utf8, headers_utf8, body_utf8, &code, (ResponseHandle *)handle);
    *statusCode = code;

    free(url_utf8);
    free(method_utf8);
    free(headers_utf8);
    free(body_utf8);

    return result;
}

MonoString* http_read_body(int handle) {
    // TODO: read content-length header here
    char buffer[1024 * 1024];
    size_t written;

    HttpError err = bodyRead(
        (ResponseHandle) handle,
        &buffer[0],
        1024 * 1024 - 1 /* save a space for the null terminator */,
        &written);
    if (err != 0) {
        return NULL;
    }
    buffer[written] = 0;
    
    return mono_wasm_string_from_js(&buffer[0]);
}

MonoString* http_read_headers_all(int handle) {
    char buffer[1024 * 1024];
    size_t written;

    HttpError err = getAllHeaders(
        (ResponseHandle) handle,
        &buffer[0],
        1024 * 1024 - 1 /* save a space for the null terminator */,
        &written);
    if (err != 0) {
        return NULL;
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

void wasi_http_attach_internal_calls() {
    // Workaround lack of threading
    mono_add_internal_call("System.Threading.TimerQueue::SetTimeout", noop_settimeout);
    mono_add_internal_call("System.Threading.ThreadPool::QueueCallback", wasi_queuecallback);

    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::Req", http_req);
    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::Close", handle_close);
    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::ReadBody", http_read_body);
    mono_add_internal_call("Wasi.Http.WasiHttpExperimental::ReadAllHeaders", http_read_headers_all);
}