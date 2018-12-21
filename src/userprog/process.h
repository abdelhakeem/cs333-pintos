#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include <list.h>

typedef int tid_t;

/* Process data */
struct process_data {
    struct list children;                   /* Direct children of the process */
    struct list confirmed_dead_children;    /* Children waited upon and confirmed dead */
    struct list file_descriptors;           /* File descriptors acquired from kernel */
};

tid_t process_execute (const char *cmd_str);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

#endif /* userprog/process.h */
