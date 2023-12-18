#include <iostream>
#include <pthread.h>
#include <string>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

void* thread_entry_point(void* ctx) {
    int id = (int) ctx;
    std::cout << "in thread: " << id << std::endl;
    return 0;
}

int main(int argc, char** argv) {
    fprintf(stderr, "argc: %d\n", argc);

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    int in = open(argv[2], O_RDONLY);
    if (in < 0) {
        fprintf(stderr, "error opening input file %s: %s\n", argv[2], strerror(errno));
        exit(1);
    }

    std::cout << "Hello, wasm32-wasi-threads!" << std::endl;
    void *p = malloc(16);
    free(p);

    int const NUM_THREADS = 10;
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        int ret = pthread_create(&threads[i], NULL, &thread_entry_point, (void *) i);
        if (ret) {
            std::cout << "failed to spawn thread "  << strerror(ret) << std::endl;
        }
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}
