## wasi-threads exporation
This project contains code for exploring wasi-threads. It is not intended to be
used for anything other than experimentation.

### Setup/Configuration
This project requires wasi-sdk to be installed:
```console
$ make clone-wasi-sdk build-wasi-sdk extract-wasi-sdk
```
And wasmtime is also the WASI runtime that is used:
```console
$ make clone-wasmtime build-wasmtime
```

### Building
With the above setup/configuration it should be possible to build this project:
```console
$ make out/main.wasm
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
```
And use the following if it was build using cmake:
```console
$ make cmake-build-run
```
