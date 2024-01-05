using Wasi.Http;

// TODO: I can't seem to make namespaced classes work right, I think it's something on my understanding.
// Move this to WasiMiddlewareHandler.cs
public class Binder {
    public static unsafe void ServeHTTP(uint req, uint res, string authority, string path, string method) {
        WasiMiddlewareHandler.Handle(req, res, authority, path, method);
    }
}