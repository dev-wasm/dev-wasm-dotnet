.phony: clean

default: libwasihttp.a

wasi-http: ; git clone https://github.com/WebAssembly/wasi-http; cd wasi-http; git checkout v0.2.0 ; cd ../ ; cp wasi.wit wasi-http/wit

libwasihttp.a:
	wget https://github.com/dev-wasm/dev-wasm-c/releases/download/0.2.0/libwasihttp-0.2.0.tgz
	tar -xvzf libwasihttp-0.2.0.tgz
	rm libwasihttp-0.2.0.tgz
	cat wasi_http.h | grep -v '#include "client.h"' > wasi_http.h.fixed
	mv wasi_http.h.fixed wasi_http.h

clean: ; rm -f libwasihttp.a wasi_http.h