#ifndef THREADS_PDONATION_H
#define THREADS_PDONATION_H

/* Priority donation.

   A higher-priority thread waiting on a lower-priority thread
   (i.e.: to acquire a lock) "donates" its priority to it so that it
   takes over the CPU and releases the lock as soon as possible, allowing
   the higher-priority thread to acquire the lock and do its task.

   Multiple higher-priority threads can be waiting on a given lower-priority
   thread at the same time, and they need not be waiting for the same lock
   (i.e.: the lower-priority thread may have more than one acquired lock for
   which higher-priority threads are waiting). In such cases, we only are
   interested in the highest donated priority on each lock.

   A higher-priority thread A of priority H may wait on a lower-priority
   thread B of priority L1 while thread B is waiting on another lower-priority
   thread C of priority L2, where L1 is not necessarily larger than L2.
   In such cases, the donated priority (H) propagates all down the chain of
   waiting threads and including the first donee thread (which is not waiting
   on any locks).

   At any time, the "effective priority" of a given thread may be different
   from its original priority. The effective priority of a thread is defined
   as the highest donated priority to the thread on all of its acquired locks.
   In case of no donated priorities, the effective priority is the thread's
   original priority.

   We keep an entry in the thread's table of donated priorities for each lock
   acquired by the thread, this entry stores a pointer to the acquired lock
   and the value of the highest priority donated to the thread on this lock.
   This implementation accounts for the prementioned special cases. */

#include <hash.h>
#include <stdbool.h>
#include "threads/synch.h"

/* An entry in the table of donated priorities of a thread. */
struct donated_priority
  {
    struct hash_elem elem;          /* Hash element for donated priorities
                                       table. */
    struct lock *acquired_lock;     /* Acquired lock associated with donated
                                       priority. */
    int value;                      /* Value of donated priority. */
  };

/* Basic functions for hashing donated_priority instances. */
unsigned donated_priority_hash (const struct hash_elem *, void *);
bool donated_priority_hash_less (const struct hash_elem *,
                                 const struct hash_elem *, void *);
void donated_priority_hash_destroy (struct hash_elem *, void *);

/* Donated priorities table basic life cycle functions. */
struct hash *dp_table_create (void);
void dp_table_destroy (struct hash *);

/* Helper functions for maintaining donated priorities table. */
struct donated_priority *dp_table_insert (struct hash *, struct lock *, int);
struct donated_priority *dp_table_find (struct hash *, struct lock *);
bool dp_table_delete (struct hash *, struct hash_elem *);
int dp_table_max_priority (struct hash *);

#endif /* threads/pdonation.h */
