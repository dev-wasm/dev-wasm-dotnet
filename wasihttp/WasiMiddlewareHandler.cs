namespace Wasi.Http;

using Microsoft.Extensions.DependencyInjection;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Http.Features;
using System.Text;
using Microsoft.VisualBasic;

public class WasiMiddlewareHandler {

    public delegate HttpResponseFeature HandlerDelegate(HttpRequestFeature req);

    public static HandlerDelegate Handler;

    public static void Handle(uint req, uint res, string authority, string path, string method) {
        var request = new HttpRequestFeature();
        request.Path = path;
        request.Method = method;
        var response = Handler(request);
        var values = new string[2];
        values[0] = response.Headers.ContentType!;
        values[1] = response.Headers.Server!;

        var reader = new StreamReader(response.Body);
        var body = reader.ReadToEnd();
        
        WasiHttpExperimental.CreateResponse(res, (uint) response.StatusCode, new string[]{"Content-type", "Server"}, values, body);
    }
}