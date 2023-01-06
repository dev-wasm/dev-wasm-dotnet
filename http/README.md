# Experimental HTTP Client Example
*NB*: this example uses an experimental `wasmtime-http` that incorporates a highly
experimental HTTP client library that is not yet part of the WASI specification.
Use at your own risk, things are likely to change drastically in the future.

## Building
```sh
dotnet build
```

## Running without allowed hosts
When you first run this client with `wasmtime-http` no hosts are allowed and so you will see an error.

```sh
$ wasmtime-http --wasi-modules experimental-wasi-http bin/Debug/net7.0/http.wasm 
Request failed: DESTINATION_NOT_ALLOWED
```

By default the WASI-http runtime blocks all URLs, to open up the URL, use the `WASI_HTTP_ALLOWED_HOSTS` environment variable:

```sh
$ WASI_HTTP_ALLOWED_HOSTS=https://postman-echo.com wasmtime-http --wasi-modules experimental-wasi-http bin/Debug/net7.0/http.wasm 
Response returned: 200
date:Fri, 06 Jan 2023 19:24:03 GMT
content-type:application/json; charset=utf-8
content-length:236
connection:keep-alive
etag:W/"ec-Khw68J6h/yTOAw9bxeZnp3+3c9U"
vary:Accept-Encoding
set-cookie:sails.sid=s%3AXuLjBo4uYKqwyQuZAxFcqwabMtloWhgW.t1vFjCB5ciUk72UEc9O51wYQv%2BqijTHxa%2F2hqE1MwyY; Path=/; HttpOnly

{"args":{},"headers":{"x-forwarded-proto":"https","x-forwarded-port":"443","host":"postman-echo.com","x-amzn-trace-id":"Root=1-63b87553-70addf2e207be0f2189e6574","content-length":"0","accept":"*/*"},"url":"https://postman-echo.com/get"}
```