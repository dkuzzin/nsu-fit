#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CLEANUP_EXECUTE 1
#define CLEANUP_DONT_EXECUTE 0
#define TIME_TILL_CANCEL 1

const char text[] = "text";

void cleanup(void *arg){
    if (arg != NULL){
        free(arg);
    }
    printf("memory freed\n");
}

void *thread_func(void *arg){
    (void)arg;

    char *str = NULL;
    pthread_cleanup_push(cleanup, str);
    str = malloc(sizeof(text));
    if (str == NULL) {
        fprintf(stderr, "malloc failed\n");
        return NULL;
    }

    memcpy(str, text, sizeof(text));


    while (1) {
        printf("%s\n", str);
    }

    pthread_cleanup_pop(CLEANUP_EXECUTE);

    return NULL;
}

int main(void){
    pthread_t thread;

    int err = pthread_create(&thread, NULL, thread_func, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_create failed: %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    sleep(TIME_TILL_CANCEL);

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
    }else{
        printf("thread wasn't canceled");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}