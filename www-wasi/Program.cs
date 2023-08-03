using Wasi.Http;
using Microsoft.AspNetCore.Http.Features;
using System.Text;

public class HttpWasmServer {
    static void Main(string[] args)
    {
        WasiMiddlewareHandler.Handler = (HttpRequestFeature req) => {
            Console.WriteLine(req.Path);
            Console.WriteLine(req.Method);

            var response = new HttpResponseFeature();
            response.Body = new MemoryStream(Encoding.UTF8.GetBytes("This is a new test!" ));
            response.StatusCode = 200;
            response.Headers["Content-type"] = "text/plain";
            response.Headers["Server"] = "aspdotnet-wasm";

            return response;
        };
    }
}
 
