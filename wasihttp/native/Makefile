.phony: gen clean run

default: wasi-http

wasi-http: ; git clone https://github.com/WebAssembly/wasi-http; cd wasi-http; git checkout v0.2.0-rc-2023-11-10 ; cd ../ ; cp wasi.wit wasi-http/wit

# Note that this doesn't quite work right there's a bug in wit-bindgen 0.4.0 that missing a C definition
# it is fixed in later versions, but they don't understand this wit syntax *bleeding-edge*
gen: wasi-http ; wit-bindgen c ${PWD}/wasi-http/wit -w wasi

clean: ; rm -rf wasi-http