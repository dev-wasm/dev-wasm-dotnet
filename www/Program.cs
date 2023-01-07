class WagiWasm {

    public Dictionary<string, string> ParseQueryParams(string? queryString)
    {
        var pairs = new Dictionary<string, string>();
        if (queryString == null) {
            return pairs;
        }
        var parts = queryString.Split("&");
        foreach (var elt in parts)
        {
            var pieces = elt.Split("=");
            if (pieces.Length == 0) {
                return pairs;
            }
            if (pieces.Length == 1) {
                pairs.Add(pieces[0], "");
            } else {
                pairs.Add(pieces[0], pieces[1]);
            }
        }

        return pairs;
    }

    static void Main(string[] args) {
        Console.WriteLine("Content-type: text/html\n\n");

        var wasm = new WagiWasm();
        var query = wasm.ParseQueryParams(Environment.GetEnvironmentVariable("QUERY_STRING"));
        var name = "Unknown";
        if (query.ContainsKey("name")) {
            name = query["name"];
        }
        Console.WriteLine("<html><body><h3>Hello " + name + "</h3></body></html>");
    }    
}

