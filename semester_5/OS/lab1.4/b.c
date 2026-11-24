#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define WAIT_TILL_CANCEL 2

void *thread_func(void *arg){
    (void)arg;

    volatile unsigned long count = 0;

    while (1) {
        count++;
        //pthread_testcancel();
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

    printf("Sending cancellation request\n");

    err = pthread_cancel(thread);
    if (err != 0) {
        fprintf(stderr, "pthread_cancel failed: %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    printf("Cancellation request sent\n");
    
    err = pthread_join(thread, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_join failed: %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    printf("Thread finished\n");

    return EXIT_SUCCESS;
}