#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int number;
    char *text;
} ThreadArgs;

void *thread_func(void *arg){
    ThreadArgs *args = arg;

    printf("number: %d\n", args->number);
    printf("text: %s\n", args->text);

    free(args);

    return NULL;
}

int main(void){
    pthread_t thread;
    pthread_attr_t attr;

    ThreadArgs *args = malloc(sizeof(*args));
    if (args == NULL) {
        fprintf(stderr, "malloc failed\n");
        return EXIT_FAILURE;
    }

    args->number = 42;
    args->text = "hello world";

    int err = pthread_attr_init(&attr);
    if (err != 0) {
        fprintf(stderr, "pthread_attr_init failed: %d\n", err);
        free(args);
        return EXIT_FAILURE;
    }

    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (err != 0) {
        fprintf(stderr, "pthread_attr_setdetachstate failed: %d\n", err);

        int destroy_err = pthread_attr_destroy(&attr);
        if (destroy_err != 0) {
            fprintf(stderr, "pthread_attr_destroy failed: %d\n", destroy_err);
        }

        free(args);
        return EXIT_FAILURE;
    }

    err = pthread_create(&thread, &attr, thread_func, args);
    if (err != 0) {
        fprintf(stderr, "pthread_create failed: %d\n", err);

        int destroy_err = pthread_attr_destroy(&attr);
        if (destroy_err != 0) {
            fprintf(stderr, "pthread_attr_destroy failed: %d\n", destroy_err);
        }

        free(args);
        return EXIT_FAILURE;
    }

    err = pthread_attr_destroy(&attr);
    if (err != 0) {
        fprintf(stderr, "pthread_attr_destroy failed: %d\n", err);
        return EXIT_FAILURE;
    }

    pthread_exit(NULL);
}