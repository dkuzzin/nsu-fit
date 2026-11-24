#define _GNU_SOURCE

#include "mythread.h"

#include <sched.h>
#include <stdatomic.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <linux/futex.h>
#include <sys/syscall.h>

#define MYTHREAD_REAP_INTERVAL 16U

static _Atomic unsigned detached_calls = 0;

#define MYTHREAD_STACK_SIZE (8UL * 1024 * 1024)

enum {
    MYTHREAD_JOINABLE,
    MYTHREAD_JOINING,
    MYTHREAD_DETACHED
};

struct mythread_control {
    pid_t tid;

    void *stack_mapping;
    size_t mapping_size;

    void *(*start_routine)(void *);
    void *arg;
    void *retval;

    _Atomic int child_tid;
    _Atomic int state;

    struct mythread_control *next;
};




static mythread_t threads_head = NULL;
static atomic_flag threads_lock = ATOMIC_FLAG_INIT;

static void register_thread(mythread_t thread);
static void unregister_thread(mythread_t thread);
static int reap_detached(void);

static int allocate_stack(mythread_t thread) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        return EINVAL;
    }

    size_t guard_size = (size_t)page_size;
    size_t mapping_size = MYTHREAD_STACK_SIZE + guard_size;

    void *mapping = mmap(NULL, mapping_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (mapping == MAP_FAILED) {
        return errno;
    }

    if (mprotect(mapping, guard_size, PROT_NONE) == -1) {
        int error = errno;

        if (munmap(mapping, mapping_size) == -1) {
            return errno;
        }

        return error;
    }

    thread->stack_mapping = mapping;
    thread->mapping_size = mapping_size;

    return 0;
}

//because clone wants int fn(void*)
static int thread_entry(void *arg){
    mythread_t thread = arg;

    register_thread(thread);
    thread->retval = thread->start_routine(thread->arg);
    return 0;
}



int mythread_create(
    mythread_t *thread,
    const mythread_attr_t *attr,
    void *(*start_routine)(void *),
    void *arg
) {
    if (thread == NULL || start_routine == NULL || attr != NULL) {
        return EINVAL;
    }

    mythread_t control = calloc(1, sizeof(*control));
    if (control == NULL) {
        return ENOMEM;
    }

    control->start_routine = start_routine;
    control->arg = arg;

    atomic_init(&control->child_tid, 1);
    atomic_init(&control->state, MYTHREAD_JOINABLE);

    int error = allocate_stack(control);
    if (error != 0) {
        free(control);
        return error;
    }

    void *stack_top = (char *)control->stack_mapping + control->mapping_size;

    int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | 
                CLONE_THREAD | CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID;

    int tid = clone(thread_entry, stack_top, flags, control, NULL, NULL, (int*)&control->child_tid);

    if (tid == -1) {
        int error = errno;

        if (munmap(control->stack_mapping, control->mapping_size) == -1) {
            int cleanup_error = errno;
            free(control);
            return cleanup_error;
        }

        free(control);
        return error;
    }

    control->tid = tid;
    *thread = control;

    return 0;
}

int mythread_join(mythread_t thread, void **retval) {
    if (thread == NULL) {
        return EINVAL;
    }

    if (thread->tid == (pid_t)syscall(SYS_gettid)) {
        return EDEADLK;
    }

    //compare and change operation
    int expected = MYTHREAD_JOINABLE;
    if (!atomic_compare_exchange_strong(&thread->state, &expected, MYTHREAD_JOINING)) {
        return EINVAL;
    }

    while (1) {
        int tid = atomic_load(&thread->child_tid);

        if (tid == 0) {
            break;
        }

        if (syscall(SYS_futex, (int *)&thread->child_tid, FUTEX_WAIT, tid, NULL, NULL, 0) == -1) {
            int error = errno;

            if (error == EAGAIN || error == EINTR) {
                continue;
            }

            atomic_store(&thread->state, MYTHREAD_JOINABLE);
            return error;
        }
    }

    //stack munmap
    if (munmap(thread->stack_mapping, thread->mapping_size) == -1) {
        int error = errno;
        atomic_store(&thread->state, MYTHREAD_JOINABLE);
        return error;
    }

    if (retval != NULL) {
        *retval = thread->retval;
    }

    unregister_thread(thread);
    free(thread);

    return 0;
}

static void lock_threads(void){
    while (atomic_flag_test_and_set_explicit(&threads_lock, memory_order_acquire)) {}
}

static void unlock_threads(void){
    atomic_flag_clear_explicit(&threads_lock, memory_order_release);
}


static void register_thread(mythread_t thread)
{
    lock_threads();

    thread->next = threads_head;
    threads_head = thread;

    unlock_threads();
}

static void unregister_thread(mythread_t thread)
{
    lock_threads();

    mythread_t *current = &threads_head;

    while (*current != NULL) {
        if (*current == thread) {
            *current = thread->next;
            break;
        }

        current = &(*current)->next;
    }

    unlock_threads();
}

static mythread_t find_current_thread(void)
{
    pid_t tid = (pid_t)syscall(SYS_gettid);

    lock_threads();

    mythread_t current = threads_head;

    while (current != NULL) {
        if (atomic_load(&current->child_tid) == tid) {
            unlock_threads();
            return current;
        }

        current = current->next;
    }

    unlock_threads();
    return NULL;
}

void mythread_exit(void *retval)
{
    mythread_t thread = find_current_thread();

    if (thread != NULL) {
        thread->retval = retval;
    }

    syscall(SYS_exit, 0);
}

int mythread_detach(mythread_t thread)
{
    if (thread == NULL) {
        return EINVAL;
    }

    int expected = MYTHREAD_JOINABLE;

    if (!atomic_compare_exchange_strong(
            &thread->state, &expected, MYTHREAD_DETACHED)) {
        return EINVAL;
    }

    unsigned count = atomic_fetch_add_explicit(
        &detached_calls, 1U, memory_order_relaxed) + 1U;

    if (count % MYTHREAD_REAP_INTERVAL == 0) {
        int error = reap_detached();
        if (error != 0) {
            return error;
        }
    }

    return 0;
}
static int reap_detached(void)
{
    int first_error = 0;

    lock_threads();

    mythread_t *curr_link = &threads_head;

    while (*curr_link != NULL) {
        mythread_t current = *curr_link;

        if (atomic_load(&current->state) != MYTHREAD_DETACHED ||
            atomic_load(&current->child_tid) != 0) {
            curr_link = &current->next;
            continue;
        }

        if (munmap(current->stack_mapping,
                   current->mapping_size) == -1) {
            if (first_error == 0) {
                first_error = errno;
            }

            curr_link = &current->next;
            continue;
        }

        *curr_link = current->next;
        free(current);
    }

    unlock_threads();
    return first_error;
}