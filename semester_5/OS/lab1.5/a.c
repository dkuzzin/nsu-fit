#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static void sigint_handler(int signo) {
    (void)signo;

    const char message[] = "SIGINT received\n";
    ssize_t n = write(STDOUT_FILENO, message, sizeof(message) - 1);
    (void)n;
}

static void *blocked_thread(void *arg) {
    (void)arg;

    while (1) {
        pause();
        printf("SIGNAL thread 1 received\n");
        fflush(stdout);
    }

    return NULL;
}

static void *sigint_thread(void *arg) {
    (void)arg;

    sigset_t set;
    if (sigemptyset(&set) == -1 || sigaddset(&set, SIGINT) == -1) {
        perror("sigset");
        return NULL;
    }

    int err = pthread_sigmask(SIG_UNBLOCK, &set, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_sigmask: %s\n", strerror(err));
        return NULL;
    }

    while (1) {
        pause();
    }

    return NULL;
}

static void *sigquit_thread(void *arg) {
    (void)arg;

    sigset_t set;
    if (sigemptyset(&set) == -1 || sigaddset(&set, SIGQUIT) == -1) {
        perror("sigset");
        return NULL;
    }

    while (1) {
        int signo;
        int err = sigwait(&set, &signo);

        if (err != 0) {
            fprintf(stderr, "sigwait: %s\n", strerror(err));
            return NULL;
        }

        printf("SIGQUIT received\n");
        fflush(stdout);
    }

    return NULL;
}

int main(void) {
    printf("Process PID = %ld\n", (long)getpid());

    sigset_t set;
    if (sigfillset(&set) == -1) {
        perror("sigfillset");
        return 1;
    }

    int err = pthread_sigmask(SIG_SETMASK, &set, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_sigmask: %s\n", strerror(err));
        return 1;
    }

    struct sigaction sa = {0};
    sa.sa_handler = sigint_handler;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    pthread_t t1, t2, t3;

    err = pthread_create(&t1, NULL, blocked_thread, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(err));
        return 1;
    }

    err = pthread_create(&t2, NULL, sigint_thread, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(err));
        return 1;
    }

    err = pthread_create(&t3, NULL, sigquit_thread, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(err));
        return 1;
    }

    err = pthread_join(t1, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_join: %s\n", strerror(err));
        return 1;
    }

    err = pthread_join(t2, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_join: %s\n", strerror(err));
        return 1;
    }

    err = pthread_join(t3, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_join: %s\n", strerror(err));
        return 1;
    }

    return 0;
}
