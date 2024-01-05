using System.Reflection.Metadata;
using Microsoft.AspNetCore.Diagnostics;

namespace Wasi.Http;

public class HttpResponse {

    public HttpResponse(int StatusCode, string Body) {
        this.StatusCode = StatusCode;
        this.Body = Body;
    }

    public int StatusCode { get; }
    public string Body { get; }
}

public class WasiMiddlewareHandler {

    public delegate HttpResponse HttpHandler(string authority, string path, string method);

    public static HttpHandler Handler;

    public static void Handle(uint req, uint res, string authority, string path, string method) {
        var response = Handler(authority, path, method);
        var values = new string[2];
        values[0] = "text/plain";
        values[1] = "Dotnet + WASI";

        WasiHttpExperimental.CreateResponse(res, (uint) response.StatusCode, new string[]{"Content-type", "Server"}, values, response.Body);
    }
}