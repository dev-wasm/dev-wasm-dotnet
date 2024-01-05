using Wasi.Http;
using System.Text;
using Microsoft.AspNetCore.Builder;

var builder = WebApplication.CreateBuilder(args).UseWasiServer();
var app = builder.Build();
app.MapGet("/", () => "Hello, world! See also: /api/getvalue/{key} and /api/setvalue/{key}/{value}");
await app.StartAsync();

/*
public class HttpWasmServer {
    static async Task Main(string[] args)
    {
        WasiMiddlewareHandler.Handler = (string authority, string path, string method) => {
            Console.WriteLine(path);
            Console.WriteLine(method);

            var response = new HttpResponse(200, "This is a new test!");
            // response.Headers["Content-type"] = "text/plain";
            // response.Headers["Server"] = "aspdotnet-wasm";

            return response;
        };
    }
}
*/
 
