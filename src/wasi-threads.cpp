#include <pthread.h>
#include <string.h>
#include <cstdlib>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "llama.h"
#include <string>

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    //std::string model_path = argv[2];
    llama_model_params model_params = llama_model_default_params();

    std::string model_path = "models/llama-2-7b-chat.Q4_0.gguf";
    fprintf(stdout, "llama.cpp example using model: %s\n", model_path.c_str());

    /*
    // Try opening the file using the same code that llama.cpp uses which is
    // just here for easier debugging/testing.
    FILE* fp = std::fopen(model_path.c_str(), "rb");
    if (fp == NULL) {
        fprintf(stderr, "failed to open %s: %s", model_path.c_str(), strerror(errno));
    }
    int ret = std::fseek(fp, (long) 0, SEEK_END);
    fprintf(stdout, "fseek returned %d\n", ret);
    ret = std::ftell(fp);
    if (ret == -1) {
        fprintf(stderr, "ftell will fail on wasi: %s\n", strerror(errno));
        // std::ftell will fail on wasm/wasi as it is a 32-bit platform 
        // and if the model size if larger than 2GB, it will fail. The
        // alternative is to use ftello64 which is not available on wasi.
        // I've added a macro guard for this in llama.cpp.
    }
    ret = ftello64(fp);
    if (ret == -1) {
        fprintf(stderr, "ftell failed: %s\n", strerror(errno));
        exit(2);
    }
    */

    std::string prompt = "What is LoRA?";
 
    bool numa = false;
    llama_backend_init(numa);

    llama_model* model = llama_load_model_from_file(model_path.c_str(), model_params);
    if (model == NULL) {
        fprintf(stderr , "%s: error: failed to to load model %s\n" , __func__, model_path.c_str());
        return 1;
    }
    fprintf(stdout, "model loaded\n");

    /*
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.seed  = 1234;
    ctx_params.n_ctx = 1024;
    ctx_params.n_threads = 4;
    ctx_params.n_threads_batch = 4;
    ctx_params.rope_scaling_type = LLAMA_ROPE_SCALING_LINEAR;

    llama_context * ctx = llama_new_context_with_model(model, ctx_params);
    if (ctx == NULL) {
        fprintf(stderr , "%s: error: failed to create the llama_context\n" , __func__);
        return 1;
    }

    llama_free(ctx);
    */
    llama_free_model(model);
    llama_backend_free();

    return 0;
}
