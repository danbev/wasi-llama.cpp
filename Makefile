WASI_SDK_VERSION=20.22ga35b453a8731
WASI_SDK=${PWD}/wasi-sdk/extracted/wasi-sdk-${WASI_SDK_VERSION}
LLVM_BIN=${WASI_SDK}/bin
WASI_SYSROOT=${WASI_SDK}/share/wasi-sysroot
TRIPLE=wasm32-wasi

WASMTIME=~/work/wasm/wasmtime/target/debug/wasmtime

out/main.wasm: src/main.cpp | out
	${LLVM_BIN}/clang++ -v  -fno-exceptions --target=${TRIPLE} --sysroot ${WASI_SYSROOT} -s -o $@ $<

out: 
	@mkdir $@

.PHONY: run
run:
	RUST_BACKTRACE=1 ${WASMTIME} out/main.wasm

.PHONY: clean

clean: 
	@${RM} -rf out


extract-wasi-sdk:
	@${RM} -rf wasi-sdk/extracted
	@mkdir -p wasi-sdk/extracted
	@tar -xf wasi-sdk/dist/wasi-sdk-${WASI_SDK_VERSION}-linux.tar.gz -C wasi-sdk/extracted
	@tar -xf wasi-sdk/dist/libclang_rt.builtins-wasm32-wasi-${WASI_SDK_VERSION}.tar.gz -C wasi-sdk/extracted/wasi-sdk-${WASI_SDK_VERSION}
