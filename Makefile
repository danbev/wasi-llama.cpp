WASI_SDK_VERSION=20.22ga35b453a8731
WASI_SDK=${PWD}/wasi-sdk/extracted/wasi-sdk-${WASI_SDK_VERSION}
CMAKE_TOOLCHAIN_FILE=${WASI_SDK}/share/cmake/wasi-sdk-pthread.cmake
LLVM_BIN=${WASI_SDK}/bin
WASI_SYSROOT=${WASI_SDK}/share/wasi-sysroot
TRIPLE=wasm32-wasi-threads

WASMTIME=${PWD}/wasmtime/target/release/wasmtime

out/wasi-threads.wasm: src/wasi-threads.cpp | out
	${LLVM_BIN}/clang++ -v -pthread \
		-lwasi-emulated-signal \
		-lwasi-emulated-process-clocks \
		-Lllama.cpp/build-wasm -Illama.cpp \
		-lllama \
		-Wl,--import-memory,--export-memory \
		-Wl,--initial-memory=4000055296,--max-memory=4294967296,--no-check-features \
		-fno-exceptions --target=${TRIPLE} --sysroot ${WASI_SYSROOT} \
	       	-o $@ $<

out: 
	@mkdir $@

.PHONY: run
run:
	env WASMTIME_BACKTRACE_DETAILS=1 ${WASMTIME} run  -W threads -S threads \
        --dir ./models out/wasi-threads.wasm -- \
        models/llama-2-7b-chat.Q4_0.gguf

cmake-build-wasi:
	@mkdir -p build
	@cd build && cmake -DUSE_WASI=ON .. && make

cmake-run-wasi:
	${WASMTIME} run  -W threads -S threads \
        --dir ./models build/wasm/wasi-threads.wasm -- \
        models/llama-2-7b-chat.Q4_0.gguf

cmake-build:
	@mkdir -p build
	@cd build && cmake -DUSE_WASI=OFF .. && make

cmake-run:
	build/wasi-threads build/wasi-threads models/llama.txt

.PHONY: clean
clean: 
	@${RM} -rf out build ggml/build-wasm 
	@cd ggml && git checkout CMakeLists.txt

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

build-llama-wasi:
	@echo "building llama.cpp as a wasi module"
	@rm -rf build/llama.cpp/build-wasm
	@mkdir -p llama.cpp/build-wasm
	@cd llama.cpp/build-wasm && cmake -DLLAMA_WASI=ON \
		-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} \
		-DBUILD_SHARED_LIBS=OFF \
		-DLLAMA_BUILD_TESTS=OFF \
		-DLLAMA_BUILD_EXAMPLES=OFF \
		.. && make

update-ggml:
	git submodule update --remote --merge ggml.cpp

build-ggml-wasi:
	@echo "building ggml as a wasi module"
	@echo "Overwriting ggml/CMakeLists.txt (this will be removed in the future is accepted upstream)"
	@cp GGML-CMakeLists.txt ggml/CMakeLists.txt
	@rm -rf build/ggml/build-wasm
	@mkdir -p ggml/build-wasm
	@cd ggml/build-wasm && cmake -DGGML_WASI=ON \
		-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} \
		-DBUILD_SHARED_LIBS=OFF \
		-DGGML_BUILD_TESTS=OFF \
		-DGGML_BUILD_EXAMPLES=OFF \
		.. && make

.PHONY: download-model
download-model:
	@mkdir -p models
	@cd models && curl -LO https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGUF/resolve/5db6994dca7288297c59693b5e27b62f22a54e1f/llama-2-7b-chat.Q4_0.gguf

