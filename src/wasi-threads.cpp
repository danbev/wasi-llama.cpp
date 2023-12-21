#include <pthread.h>
#include <string.h>
#include <cstdlib>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "llama.h"
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    std::string model_path = argv[2];
    llama_model_params model_params = llama_model_default_params();

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

    std::string prompt = argv[3];
 
    bool numa = false;
    llama_backend_init(numa);

    llama_model* model = llama_load_model_from_file(model_path.c_str(), model_params);
    if (model == NULL) {
        fprintf(stderr , "%s: error: failed to to load model %s\n" , __func__, model_path.c_str());
        return 1;
    }

    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.seed  = 1234;
    ctx_params.n_ctx = 1024;
    ctx_params.n_threads = 4;
    ctx_params.n_threads_batch = 4;
    //ctx_params.rope_scaling_type = LLAMA_ROPE_SCALING_LINEAR;

    llama_context * ctx = llama_new_context_with_model(model, ctx_params);
    if (ctx == NULL) {
        fprintf(stderr , "%s: error: failed to create the llama_context\n" , __func__);
        return 1;
    }

    // Tokenize the prompt.
    const int add_bos_token = llama_add_bos_token(model);
    const bool add_bos  = add_bos_token != -1 ? bool(add_bos_token) :
        (llama_vocab_type(model) == LLAMA_VOCAB_TYPE_SPM); // SPM = SentencePiece Model

    printf("add_bos: %d\n", add_bos);
    printf("prompt.len: %ld\n", prompt.length());
    int n_tokens = prompt.length() + add_bos;
    std::vector<llama_token> input_tokens(n_tokens);
    n_tokens = llama_tokenize(model,
                              prompt.data(),
                              prompt.length(),
                              input_tokens.data(),
                              input_tokens.size(),
                              true,
                              false);
    // llama_tokenize will return the negative length of the token if
    // it is longer that the passed in result.length. If that is the case
    // then we need to resize the result vector to the length of the token
    // and call llama_tokenize again.
    if (n_tokens < 0) {
        input_tokens.resize(-n_tokens);
        int new_len = llama_tokenize(model, prompt.data(), prompt.length(), input_tokens.data(), input_tokens.size(), add_bos, false);
    } else {
        input_tokens.resize(n_tokens);
    }
    fprintf(stderr, "\n");
    fprintf(stdout, "n_tokens: %d\n", n_tokens);

    llama_batch batch = llama_batch_init(512,/*embd*/ 0, /*n_seq_max*/ 1);
    for (int i = 0; i < n_tokens; i++) {
        // the token of this batch entry.
        batch.token[i] = input_tokens[i];
        // the position in the sequence of this batch entry.
        batch.pos[i] = i,
        // the number of sequence id's of this batch entry.
        batch.n_seq_id[i] = 1;
        batch.seq_id[i][0] = 0;  // the sequence id
        // Determins if the logits for this token should be generated or not.
        batch.logits[i] = false;
        // Increment the number of tokens in the batch.
        batch.n_tokens++;
    }

    // Instruct llama to generate the logits for the last token
    batch.logits[batch.n_tokens - 1] = true;
    fprintf(stderr, "batch.n_tokens: %d\n", batch.n_tokens);
    fprintf(stderr, "batch.tokens: [");
    for (int i = 0; i < batch.n_tokens; i++) {
        fprintf(stderr, "%d, ", batch.token[i]);
    }
    fprintf(stderr, "]\n");

    fprintf(stderr, "prompt: %s", prompt.c_str());
    fflush(stderr);

    // Now we run the inference on the batch. This will populate the logits
    // for the last token in the batch.
    if (llama_decode(ctx, batch) != 0) {
        fprintf(stderr, "llama_decode() failed\n");
        return 1;
    }

    // This is the total number of tokens that we will generate, which recall
    // includes our query tokens (they are all in the llm_batch).
    const int n_len = 80;

    int n_cur = batch.n_tokens;
    int n_decode = batch.n_tokens;
    int n_vocab = llama_n_vocab(model);

    float* all_logits = llama_get_logits(ctx);
    // All the logits are stored in a 2d vector std::vector<float> logits
    // where the first dimension is the number of tokens in the batch and
    // the second dimension is the number of tokens in the vocabulary.
    float* last_logits = all_logits + (batch.n_tokens - 1) * n_vocab;

    int n_batch_tokens = batch.n_tokens;
    
    //while (true) {
    while (n_cur <= n_len) {
        // logits are stored in the last token of the batch and are the 
        // raw unnormalized predictions.
        //float* logits = llama_get_logits_ith(ctx, batch.n_tokens - 1);
        float* logits = all_logits + (n_batch_tokens - 1) * n_vocab;

        std::vector<llama_token_data> candidates;
        candidates.reserve(n_vocab);

        // The following is populating the candidates vector with the
        // logit for each token in the vocabulary (32000).
        for (llama_token token_id = 0; token_id < n_vocab; token_id++) {
            candidates.emplace_back(llama_token_data{ token_id, logits[token_id], 0.0f });
        }
        // Here we are creating an unsorted array of token data from the vector.
        bool sorted = false;
        llama_token_data_array candidates_p = { candidates.data(), candidates.size(), sorted };

        // Find the token with the highest raw score (logit) and return it.
        const llama_token new_token_id = llama_sample_token_greedy(ctx, &candidates_p);
        // This is the token id that the model predicted.

        // is it an end of stream?
        if (new_token_id == llama_token_eos(model) || n_cur == n_len) {
            fprintf(stderr, "\n");
            fflush(stderr);
            break;
        }

        // Next we get the string value for the token id. This is called a
        // piece // which I think comes from SentencePiece.
        // We don't know that actual length of the token so we are using 
        // 8 here a "guess". If the token is longer than 8 bytes then we
        // will resize the result vector and call llama_token_to_piece again.
        std::vector<char> piece(8, 0);
        int n_tokens = llama_token_to_piece(model, new_token_id, piece.data(), piece.size());
        // llama_token_to_piece will return the negative length of the token if
        // it is longer that the passed in result.length. If that is the case
        // then we need to resize the result vector to the length of the token
        // and call llama_token_to_piece again.
        if (n_tokens < 0) {
            piece.resize(-n_tokens);
            int new_len = llama_token_to_piece(model, new_token_id, piece.data(), piece.size());
        } else {
            piece.resize(n_tokens);
        }
        std::string piece_str = std::string(piece.data(), piece.size());
        fprintf(stderr, "%s", piece_str.c_str());
        // stdout is line buffered and we are not printing a newline so we
        // above so we need to call flush.
        fflush(stderr);

        // So we initially had a batch of size equal to the number of tokens
        // of the prompt. Now, we want to pass in the token that we just
        // predicted to the model and get the logits for the next token.

        // Update the batch to include the new token id, and the position of the
        // token in the sequence.
        llama_batch single_token_batch = llama_batch_init(1,/*embd*/ 0, /*n_seq_max*/ 1);
        single_token_batch.n_tokens = 1; // We are only passing in one token.
        single_token_batch.token[0] = new_token_id; // the new token id.
        single_token_batch.pos[0] = n_cur, // the position in the sequence.
        single_token_batch.n_seq_id[0] = 1;  // the number of sequences for this token.
        single_token_batch.seq_id[0][0] = 0; // the actual sequence id.
        single_token_batch.logits[0] = true;
        n_batch_tokens = single_token_batch.n_tokens;

        n_decode += 1;
        n_cur += 1;

        // With the new token added to the batch, we can now predict the
        // next token.
        if (llama_decode(ctx, single_token_batch)) {
            fprintf(stderr, "%s : failed to eval, return code %d\n", __func__, 1);
            return 1;
        }
        llama_batch_free(single_token_batch);
    }
    fprintf(stdout, "\nDecoded %d tokens\n", n_decode);

    llama_free(ctx);
    llama_free_model(model);
    llama_backend_free();

    return 0;
}
