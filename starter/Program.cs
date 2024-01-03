using System.IO;
using System;

class DotnetWasm {
    public DotnetWasm() {}

    public void Print() {
        Console.WriteLine("Hello dotnet world: " + System.IO.Directory.GetCurrentDirectory());
    }

    public void Write() {
        using StreamWriter file = new("test.txt");
        file.WriteLine("This is a test");
    }

    public void Copy() {
        using StreamWriter file = new("test2.txt");
        foreach (var line in System.IO.File.ReadLines("test.txt", new System.Text.UTF8Encoding())) {
            file.WriteLine(line);
        }
    }

    static void Main(String[] args) {
        var dw = new DotnetWasm();
        dw.Print();
        dw.Write();
        dw.Copy();
    }    
}

