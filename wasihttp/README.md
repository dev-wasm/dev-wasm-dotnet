# Experimental HTTP Binding library for wasi-http
*NB*: this example uses an experimental `wasi-http` that is part of the WASI specification
but is still going through iteration. Expect breaking changes in future releases.

## Building
```sh
dotnet build
```

## Examples
There is a client example in `../http` and a server example in `../www-wasi`.

## Supported Runtimes
This should work in [wasirun](https://github.com/stealthrocket/wasi-go) 0.7.5+ and [wasmtime](https://github.com/bytecodealliance/wasmtime) 9.0.x [10.0.x and 11.0.x use a newer version of wasi-http]


## Regnerating code
`native/proxy.h`, `native/proxy.c` and `native/proxy_component_type.o` are all generated by `wit-bindgen`.

You probably don't need to regenerate, but if you do:

```sh
git clone https://github.com/WebAssembly/wasi-http
cd wasi-http
# corresponds to wasmtime 9.0.x
git checkout 244e068
/usr/local/lib/wit-bindgen c --world proxy ./wit
```

## TODO
* Headers on the request are not currently supported.
* Validate no memory leaks.
* Expose more of the 'native' wasi-http interface?