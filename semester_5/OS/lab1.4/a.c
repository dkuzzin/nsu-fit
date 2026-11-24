#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define WAIT_TILL_CANCEL 2


void *thread_func(void *arg){
    (void)arg;
    while (1) {
        printf("hello world\n");
    }

    return NULL;
}

int main(void){
    pthread_t thread;

    int err = pthread_create(&thread, NULL, thread_func, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_create failed: %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    sleep(WAIT_TILL_CANCEL);

    err = pthread_cancel(thread);
    if (err != 0) {
        fprintf(stderr, "pthread_cancel failed: %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    void *result = NULL;

    err = pthread_join(thread, &result);
    if (err != 0) {
        fprintf(stderr, "pthread_join failed: %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    if (result == PTHREAD_CANCELED) {
        printf("thread was canceled\n");
    } else {
        printf("thread finished normally\n");
    }

    return EXIT_SUCCESS;
}