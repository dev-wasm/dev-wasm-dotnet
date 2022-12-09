class DotnetWasm {
    public DotnetWasm() {}

    public void Print() {
        Console.WriteLine("Hello dotnet world: " + System.IO.Directory.GetCurrentDirectory());
    }

    public void Read() {
        foreach (var line in System.IO.File.ReadLines("/test.txt", new System.Text.UTF8Encoding())) {
            Console.WriteLine(line);
        }
    }

    static void Main(String[] args) {
        var dw = new DotnetWasm();
        dw.Print();
        dw.Read();
    }    
}

