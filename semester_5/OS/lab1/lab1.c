#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#define THREAD_COUNT 5

int global = 1;


void *mythread(void *arg) {
    int local = 2;
    static int static_local = 2;
    const int const_local = 3;
    
    (void)arg;
	printf("    mythread [%d %d %d]: pthread_self = %lu\n",
           getpid(), getppid(), gettid(), (unsigned long)pthread_self());

    printf("        local = %p\n", (void *)&local);
    printf("        static_local = %p\n", (void *)&static_local);
    printf("        const_local = %p\n", (void *)&const_local);
    printf("        global = %p\n", (void *)&global);

    printf("                before: local = %d, global = %d\n", local, global);

    local++;
    global++;

    printf("                after:  local = %d, global = %d\n", local, global);

    sleep(300);

	return NULL;
}

int main() {
    pthread_t tids[THREAD_COUNT];
    
	int err;
    
    for (int i = 0; i < THREAD_COUNT; ++i) {
        err = pthread_create(&tids[i], NULL, mythread, NULL);

        if (err) {
            printf("main: pthread_create() failed: %s\n", strerror(err));
            return -1;
        }

        printf("pthread_create: tids[%d] = %lu\n", i, (unsigned long)tids[i]);
    }

    
    for (int i = 0; i < THREAD_COUNT; ++i) {
        err = pthread_join(tids[i], NULL);
        if (err) {
            printf("main: pthread_join() failed: %s\n", strerror(err));
            return -1;
        }
    }

	return 0;
}