.phony: clean run gen

clean:
	dotnet clean
	rm -f main*.wasm wasi_snapshot_preview1.reactor.wasm
	make -C ../wasihttp/native clean

gen:
	make -C ../wasihttp/native libwasihttp.a
	make -C ../wasihttp/native wasi-http

bin/Debug/net7.0/www-wasi.wasm: Program.cs www-wasi.csproj gen
	dotnet build

wasi_snapshot_preview1.reactor.wasm: ; wget https://github.com/bytecodealliance/wasmtime/releases/download/v18.0.2/wasi_snapshot_preview1.reactor.wasm

main.component.wasm: bin/Debug/net7.0/www-wasi.wasm wasi_snapshot_preview1.reactor.wasm
	wasm-tools component embed ../wasihttp/native/wasi-http/wit bin/Debug/net7.0/www-wasi.wasm -o main.embed.wasm -w wasi
	wasm-tools component new main.embed.wasm -o main.component.wasm --adapt wasi_snapshot_preview1.reactor.wasm

run: main.component.wasm
	wasmtime serve -Scommon main.component.wasm 