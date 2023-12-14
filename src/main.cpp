#include <iostream>
#include <pthread.h>

void* thread_entry_point(void* ctx) {
    int id = (int) ctx;
    std::cout << "in thread: " << id << std::endl;
    return 0;
}

int main() {
    std::cout << "Hello, wasm32-wasi-threads!" << std::endl;

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
