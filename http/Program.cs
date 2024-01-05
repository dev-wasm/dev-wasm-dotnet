using Wasi.Http;
using System;
using System.Net.Http;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

class HttpWasm
{
    static void printResponse(HttpResponseMessage response)
    {
        Console.WriteLine(response);
        Console.WriteLine(response.Content.ReadAsStringAsync().Result);
    }

    static async Task testPost(HttpClient client)
    {
        // taken from https://learn.microsoft.com/en-us/dotnet/fundamentals/networking/http/httpclient
        using StringContent jsonContent = new(
            JsonSerializer.Serialize(new
            {
                userId = 77,
                id = 1,
                title = "write code sample",
                completed = false
            }),
            Encoding.UTF8,
            "application/json");

        using var response = await client.PostAsync("https://postman-echo.com/post", jsonContent);
        printResponse(response);
    }

    static async Task testGet(HttpClient client)
    {
        using var result = await client.GetAsync("https://postman-echo.com/get", System.Threading.CancellationToken.None);
        printResponse(result);
    }

    static async Task Main(string[] args)
    {
        var client = new HttpClient(new WasiHttpHandler());
        try
        {
            await testGet(client);
            await testPost(client);
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex);
        }
    }
}

