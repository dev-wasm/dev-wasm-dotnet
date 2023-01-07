using Wasi.Http;

class HttpWasm {
    unsafe static void lowLevel() {
                int code;
        int handle;
        WasiHttpExperimental.HttpError err = WasiHttpExperimental.Req("https://postman-echo.com/get", "GET", "", "", &code, &handle);
        if (err != WasiHttpExperimental.HttpError.SUCCESS) {
            Console.WriteLine("Request failed: " + err);
        } else {
            Console.WriteLine("Response returned: " + code);
            string headers = WasiHttpExperimental.ReadAllHeaders(handle);
            Console.WriteLine(headers);

            string body = WasiHttpExperimental.ReadBody(handle);
            Console.WriteLine(body);
        }
        WasiHttpExperimental.Close(handle);
    }

    unsafe static void Main(string[] args) {
        // lowLevel();
        
        // high level example.
        var client = new HttpClient(new WasiHttpHandler());
        var task = client.GetAsync("https://postman-echo.com/get", System.Threading.CancellationToken.None);
        task.Wait();
        Console.WriteLine(task.Result);
    }
}

