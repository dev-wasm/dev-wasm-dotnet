.phony: clean

default: libwasihttp.a

wasi-http: ; git clone https://github.com/WebAssembly/wasi-http; cd wasi-http; git checkout v0.2.0 ; cd ../ ; cp wasi.wit wasi-http/wit

wasihttp_version := 0.2.3

libwasihttp.a:
	wget https://github.com/dev-wasm/dev-wasm-c/releases/download/$(wasihttp_version)/libwasihttp-$(wasihttp_version).tgz
	tar -xvzf libwasihttp-$(wasihttp_version).tgz
	rm libwasihttp-$(wasihttp_version).tgz

clean: ; rm -f libwasihttp.a wasi_http.h; rm -rf wasi-http