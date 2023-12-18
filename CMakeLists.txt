cmake_minimum_required(VERSION 3.5.0)
project(WasiThreads)

set(WASI_SDK_VERSION "20.22ga35b453a8731")
set(WASI_SDK "${CMAKE_CURRENT_SOURCE_DIR}/wasi-sdk/extracted/wasi-sdk-${WASI_SDK_VERSION}")
set(WASI_SYSROOT "${WASI_SDK}/share/wasi-sysroot")
set(LLVM_BIN "${WASI_SDK}/bin")
set(TRIPLE "wasm32-wasi-threads")

# Explicitly set the C++ compiler
set(CMAKE_CXX_COMPILER "${LLVM_BIN}/clang++")

# Set the sysroot and toolchain file for the WASI SDK
set(CMAKE_SYSROOT ${WASI_SYSROOT})
set(CMAKE_TOOLCHAIN_FILE ${WASI_SDK}/share/cmake/wasi-sdk-pthread.cmake)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)

# Set the path to the WASI SDK (modify this to your WASI SDK path)
set(WASI_SDK_PATH "wasi-sdk")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v")

# Add executable
add_executable(wasi-threads src/wasi-threads.cpp)

target_compile_options(wasi-threads PRIVATE -pthread -s -v -fno-exceptions --target=${TRIPLE} --sysroot ${WASI_SYSROOT})
target_link_options(wasi-threads PRIVATE -pthread -Wl,--import-memory,--export-memory,--max-memory=67108864 --target=${TRIPLE})


# Set the target properties for WebAssembly
set_target_properties(wasi-threads PROPERTIES 
    SUFFIX ".wasm"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/wasm
)

