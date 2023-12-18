WASI_SDK_VERSION=20.22ga35b453a8731
WASI_SDK=${PWD}/wasi-sdk/extracted/wasi-sdk-${WASI_SDK_VERSION}
LLVM_BIN=${WASI_SDK}/bin
WASI_SYSROOT=${WASI_SDK}/share/wasi-sysroot
TRIPLE=wasm32-wasi-threads

WASMTIME=${PWD}/wasmtime/target/release/wasmtime

out/main.wasm: src/main.cpp | out
	${LLVM_BIN}/clang++ -v -pthread -Wl,--import-memory,--export-memory,--max-memory=67108864 -fno-exceptions --target=${TRIPLE} --sysroot ${WASI_SYSROOT} -s -o $@ $<

out: 
	@mkdir $@

.PHONY: run
run:
	${WASMTIME} run  -W threads -S threads --dir ./models out/main.wasm -- models/llama.txt

.PHONY: clean

clean: 
	@${RM} -rf out

clone-wasi-sdk:
	@git clone --recursive https://github.com/WebAssembly/wasi-sdk.git

build-wasi-sdk:
	cd wasi-sdk && env NINJA_FLAGS=-v make package

extract-wasi-sdk:
	@${RM} -rf wasi-sdk/extracted
	@mkdir -p wasi-sdk/extracted
	@tar -xf wasi-sdk/dist/wasi-sdk-${WASI_SDK_VERSION}-linux.tar.gz -C wasi-sdk/extracted
	@tar -xf wasi-sdk/dist/libclang_rt.builtins-wasm32-wasi-${WASI_SDK_VERSION}.tar.gz -C wasi-sdk/extracted/wasi-sdk-${WASI_SDK_VERSION}

clone-wasmtime:
	@git clone --recursive https://github.com/bytecodealliance/wasmtime

build-wasmtime:
	cd wasmtime && cargo build --release

update-llama:
	git submodule update --remote --merge llama.cpp
