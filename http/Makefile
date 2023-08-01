cc := /usr/local/lib/wasi-sdk-20.0/bin/clang

.phony: gen clean

default: gen

wasi-http: ; git clone https://github.com/WebAssembly/wasi-http; cd wasi-http; git checkout 1c95bc2

gen: wasi-http ; wit-bindgen c ${PWD}/wasi-http/wit -w proxy