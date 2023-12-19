#include <pthread.h>
#include <string.h>
#include <cstdlib>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

void* thread_entry_point(void* ctx) {
    int id = reinterpret_cast<intptr_t>(ctx);
    fprintf(stdout, "in thread: %d\n", id);
    return 0;
}

int main(int argc, char** argv) {
    fprintf(stderr, "argc: %d\n", argc);

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int in = open(argv[2], O_RDONLY);
    if (in < 0) {
        fprintf(stderr, "error opening input file %s: %s\n", argv[2], strerror(errno));
        exit(1);
    }
    fprintf(stderr, "opened input file %s\n", argv[2]);

    void *p = malloc(16);
    free(p);

    int const NUM_THREADS = 10;
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        int ret = pthread_create(
                      &threads[i],
                      NULL,
                      &thread_entry_point,
                      reinterpret_cast<void*>(static_cast<intptr_t>(i)));

        if (ret) {
            fprintf(stderr, "failed to spawn thread: %s\n", strerror(ret));
        }
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}
