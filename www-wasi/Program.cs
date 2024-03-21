using Wasi.Http;
using System;
using System.Text;

public class HttpWasmServer {
    static void Main(string[] args)
    {
        WasiMiddlewareHandler.Handler = (string authority, string path, string method) => {
            Console.WriteLine(path);
            Console.WriteLine(method);

            var response = new HttpResponse(201, "This is a new test!");
            // response.Headers["Content-type"] = "text/plain";
            // response.Headers["Server"] = "aspdotnet-wasm";

            return response;
        };
    }
}
 
