#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include <list.h>

#include "threads/thread.h"

/* Process data */
struct process_data {
    struct list children;
    struct list confirmed_dead_children;
};

/* List node that contains the process identifier */
struct pid_container {
    int pid;
    struct list_elem elem;
};

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

#endif /* userprog/process.h */
