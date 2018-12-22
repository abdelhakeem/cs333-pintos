#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include <list.h>
#include <hash.h>
#include "threads/synch.h"

typedef int tid_t;

/* Process data */
struct process_data {
    struct list children;                   /* Direct children of the process */
    struct list confirmed_dead_children;    /* Children waited upon and confirmed dead */
    struct hash file_descriptors;           /* File descriptors acquired from kernel */
    struct file *file;                      /* Process executable file */
    int next_file_fd;                       /* Number of the next file descriptor */
};


/* Elements for a hashing process information */

/* Structure for the hashed element */
struct process_hash {
    struct hash_elem elem;          /* Hash element for donated priorities
                                       table. */
    int key;                        /* Key for finding this process */

    tid_t id;                       /* Identifier. */
    int status;
    struct thread *t;               /* Pointer to thread if exists */
    struct semaphore started;       /* Semaphore for synchronization
                                       with parent */
};
/* Hashing function for process_hash structure */
unsigned process_hash_func (const struct hash_elem *, void *);
/* Comparator function for hash_struct that returns true if a precedes b */
bool process_hash_less (const struct hash_elem *,
                                 const struct hash_elem *, void *);
/* Looks up the table for the hashed element */
struct process_hash * process_lookup (struct hash *table, int key);

//tid_t process_execute (const char *file_name);
tid_t process_execute (const char *cmd_str);

int process_wait (tid_t);
void process_exit (void);
void process_activate (void);


#endif /* userprog/process.h */
