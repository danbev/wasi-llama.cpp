## wasi-threads exporation
This project contains code for exploring wasi-threads specifically trying to
get ggml and llama.cpp working with wasi-threads using the WASI-SDK.

This project is currently using make and cmake as I don't really know which
the upstream project will use and I want to be able to test both.

### Setup/Configuration
This project requires wasi-sdk to be installed:
```console
$ make clone-wasi-sdk build-wasi-sdk extract-wasi-sdk
```

And wasmtime is the WASI runtime that is used and is also required:
```console
$ make clone-wasmtime build-wasmtime
```

### Building ggml with wasi-threads support
The ggml c library is includes as a git submodule so make sure to run the
following command if it is not already cloned:
```console
$ git submodule update --init --recursive
```

The following command will build ggml
```console
$ make build-ggml-wasi
```
Currently, the above command will copy the file GGML-CMakeLists.txt to the
ggml submodule and then build it using cmake. This copying will be removed in
the future but for now it is required as these changes have not been upstream
nor have a created a branch for them in my fork yet.
TODO: figure out what the best approach is for these changes.

### Building
With the above setup/configuration/build it should be possible to build this
project:
```console
$ make out/wasi-threads.wasm
```
The above will use the [make](./Makefile) file to build the main.wasm module.

This can also be built using [cmake](./CMakeLists.txt):
```console
$ make cmake-build
```

### Running
The following command will run the main.wasm module if the project was built
using make::
```console
$ make run 
/home/danielbevenius/work/c++/wasi-threads/wasmtime/target/release/wasmtime run  -W threads -S threads --dir ./models out/wasi-threads.wasm -- models/llama.txt
argc: 3
opened input file models/llama.txt
in thread: 0
in thread: 1
in thread: 2
in thread: 3
in thread: 4
in thread: 5
in thread: 6
in thread: 7
in thread: 8
in thread: 9
ctx mem size: 16777216
ctx mem used: 0
x tensor type: f32
x tensor backend: 0 
x tensor dimensions: 1
x tensor data: 0xd6d6c
```
This has been a test-by-step process where I first wanted to make sure that
threading work, and then I wanted to veriy that I could use ggml with wasi.
Next step is to get llama.cpp working with wasi-threads (I hope).

And use the following if it was build using cmake:
```console
$ make cmake-build-run
```
