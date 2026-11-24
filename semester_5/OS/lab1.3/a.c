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

    return NULL;
}

int main(void){
    pthread_t thread;

    ThreadArgs args = {
        .number = 1,
        .text = "text"
    };

    int err = pthread_create(&thread, NULL, thread_func, &args);
    if (err != 0) {
        fprintf(stderr, "pthread_create failed: %d\n", err);
        return EXIT_FAILURE;
    }

    err = pthread_join(thread, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_join failed: %d\n", err);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}