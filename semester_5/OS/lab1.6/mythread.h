#ifndef MYTHREAD_H
#define MYTHREAD_H

typedef struct mythread_control *mythread_t;
typedef struct mythread_attr mythread_attr_t;

int mythread_create(
    mythread_t *thread,
    const mythread_attr_t *attr,
    void *(*start_routine)(void *),
    void *arg
);

int mythread_join(mythread_t thread, void **retval);

int mythread_detach(mythread_t thread);

void mythread_exit(void *retval);

#endif