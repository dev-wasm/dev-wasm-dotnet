# Experimental HTTP Server Example
*NB*: this example uses an experimental `wasi-http` that is part of the WASI specification
but is still going through iteration. Expect breaking changes in future releases.

## Building
```sh
dotnet build
```

## Running
This requires `wasirun` so you need to run `go get github.com/stealthrocket/wasi-go`.

```sh
# Run the server
wasirun --http-server-addr localhost:8080 --http v1 bin/Debug/net7.0/www-wasi.wasm

# Connect to the server
curl localhost:8080
```