#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *thread_func(void *arg) {
    (void)arg;
    pthread_attr_t attr;
    int err;
    err = pthread_detach(pthread_self());
    if (err != 0) {
        fprintf(stderr, "[thread] pthread_detach: %s\n", strerror(err));
        return NULL;
    }

    int state;
    err = pthread_getattr_np(pthread_self(), &attr);
    if (err != 0){
        fprintf(stderr, "error");
        return NULL;
    }
    err = pthread_attr_getdetachstate(&attr, &state);
    if (err != 0){
        fprintf(stderr, "err2");
    }

    if (state == PTHREAD_CREATE_JOINABLE){
        printf("JOINABLE\n");
    }else if (state == PTHREAD_CREATE_DETACHED){
        printf("DETACHED\n");
    }else{
        printf("o_O");
    }

    pthread_attr_destroy(&attr);


    printf("pthread_self: %lu, gettid: %ld\n", (unsigned long)pthread_self(),(long)gettid());

    return NULL;
}

int main(void) {
    pthread_attr_t attr;

    int err = pthread_attr_init(&attr);
    if (err != 0) {
        fprintf(stderr, "[main] pthread_attr_init: %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    //err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (err != 0) {
        fprintf(stderr, "[main] pthread_attr_setdetachstate: %s\n", strerror(err));

        pthread_attr_destroy(&attr);
        return EXIT_FAILURE;
    }

    
        pthread_t thread;
        
        err = pthread_create(&thread, &attr, thread_func, NULL);
        //err = pthread_create(&thread, NULL, thread_func, NULL);
        if (err != 0) {
            fprintf(stderr, "[main] pthread_create: %s\n", strerror(err));
            return EXIT_FAILURE;
        }
    err = pthread_join(thread, NULL);
    if (err != 0){
        printf("f");
    }
    return 0;
}