.phony: clean run

clean:
	dotnet clean
	rm main*.wasm wasi_snapshot_preview1.reactor.wasm

wasi_snapshot_preview1.reactor.wasm: ; wget https://github.com/bytecodealliance/wasmtime/releases/download/v15.0.1/wasi_snapshot_preview1.reactor.wasm

main.component.wasm: bin/Debug/net7.0/http.wasm wasi_snapshot_preview1.reactor.wasm
	dotnet build
	wasm-tools component embed ../wasihttp/native/wasi-http/wit bin/Debug/net7.0/http.wasm -o main_2023_11_10.embed.wasm -w wasi
	wasm-tools component new main_2023_11_10.embed.wasm -o main.component.wasm --adapt wasi_snapshot_preview1.reactor.wasm

run: main.component.wasm
	wasmtime -S http --wasm component-model main.component.wasm 