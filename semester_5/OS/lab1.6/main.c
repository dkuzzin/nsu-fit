#include "mythread.h"

#include <stdio.h>

static void *worker(void *arg)
{
    int *value = arg;
    *value = 42;

    return arg;
}

int main(void)
{
    mythread_t thread;
    int value = 0;

    int error = mythread_create(&thread, NULL, worker, &value);
    if (error != 0) {
        fprintf(stderr, "mythread_create: %d\n", error);
        return 1;
    }

    void *retval = NULL;

    error = mythread_join(thread, &retval);
    if (error != 0) {
        fprintf(stderr, "mythread_join: %d\n", error);
        return 1;
    }

    printf("value = %d\n", value);
    printf("return pointer matches = %d\n", retval == &value);

    return 0;
}