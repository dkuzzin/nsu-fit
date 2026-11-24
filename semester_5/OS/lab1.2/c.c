#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void *thread_func(void *arg) {
    (void)arg;
    static char message[] = "hello world";

    return message;
}

int main(void) {
    pthread_t thread;
    

    int err = pthread_create(&thread, NULL, thread_func, NULL);
    if (err != 0) {
        fprintf(stderr, "[main] pthread_create: %s\n", strerror(err));
        return EXIT_FAILURE;
    }
    void *retval;

    err = pthread_join(thread, &retval);
    if (err != 0) {
        fprintf(stderr, "[main] pthread_join: %s\n", strerror(err));
        return EXIT_FAILURE;
    }
    printf("Worker thread has finished\n");
    
    if (retval == NULL) {
        fprintf(stderr, "[main] Worker thread returned NULL\n");
        return EXIT_FAILURE;
    }

    const char *message = retval;

    printf("Thread returned: %s\n", message);
    
    return EXIT_SUCCESS;
}