using System.Runtime.CompilerServices;

namespace Wasi.Http;

internal class WasiHttpExperimental
{
    public enum Scheme: int {
        HTTP = 0,
        HTTPS
    }

    public enum Method: int {
        GET = 0,
        HEAD,
        POST,
        PUT,
        DELETE,
        CONNECT,
        OPTIONS,
        TRACE,
        PATCH
    };

    [MethodImpl(MethodImplOptions.InternalCall)]
    public unsafe static extern int Req(Method method, Scheme scheme, string authority, string path, string query, string headers, string body, int *statusCode, int *handle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern string ReadBody(int handle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern string ReadAllHeaders(int handle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void Close(int handle);
}