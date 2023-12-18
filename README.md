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

### Running
The following command will run the main.wasm module:
```console
$ make run
```
