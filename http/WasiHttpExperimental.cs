using System.Runtime.CompilerServices;

namespace Wasi.Http;

internal class WasiHttpExperimental
{
    public enum HttpError: int {
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
    };

    [MethodImpl(MethodImplOptions.InternalCall)]
    public unsafe static extern HttpError Req(string url, string method, string headers, string body, int *statusCode, int *handle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern string ReadBody(int handle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern string ReadAllHeaders(int handle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern HttpError Close(int handle);
}