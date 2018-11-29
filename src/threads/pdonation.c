#include "threads/pdonation.h"
#include <debug.h>
#include "threads/malloc.h"
#include "threads/thread.h"

/* Hash function for struct donated_priority using acquired_lock
   pointer as the key. */
unsigned
donated_priority_hash (const struct hash_elem *dp_, void *aux UNUSED)
{
  ASSERT (dp_ != NULL);

  const struct donated_priority *dp
    = hash_entry (dp_, struct donated_priority, elem);
  return hash_bytes (&dp->acquired_lock, sizeof dp->acquired_lock);
}

/* Compares struct donated_priority instances based on their hashing
   keys (acquired_lock pointers). */
bool
donated_priority_hash_less (const struct hash_elem *a_,
                            const struct hash_elem *b_, void *aux UNUSED)
{
  ASSERT (a_ != NULL);
  ASSERT (b_ != NULL);

  const struct donated_priority *a = hash_entry (a_, struct donated_priority,
                                                 elem);
  const struct donated_priority *b = hash_entry (b_, struct donated_priority,
                                                 elem);

  return a->acquired_lock < b->acquired_lock;
}

/* Frees memory allocated for an entry in donated priorities table. */
void
donated_priority_hash_destroy (struct hash_elem *dp_, void *aux UNUSED)
{
  free (hash_entry (dp_, struct donated_priority, elem));
}

/* Creates and intializes a donated priorities table. */
struct hash *
dp_table_create (void)
{
  struct hash *table
    = (struct hash*) malloc (sizeof (struct hash));
  hash_init (table, donated_priority_hash, donated_priority_hash_less, NULL);
  return table;
}

/* Destroys donated priorities table and frees its memory resources. */
void
dp_table_destroy (struct hash *table)
{
  hash_destroy (table, donated_priority_hash_destroy);
  free (table);
}

/* Inserts an entry in TABLE with donated priority of VALUE on LOCK.
   Returns the inserted donated_priority instance. */
struct donated_priority *
dp_table_insert (struct hash *table, struct lock *lock, int value)
{
  struct donated_priority *dp
    = (struct donated_priority *) malloc (sizeof (struct donated_priority));
  dp->acquired_lock = lock;
  dp->value = value;
  hash_insert (table, &dp->elem);
  return dp;
}

/* Finds the entry in TABLE whose acquired_lock is LOCK
   (i.e.: donated priority on LOCK).
   Returns a null pointer if no such entry is found. */
struct donated_priority *
dp_table_find (struct hash *table, struct lock *lock)
{
  struct donated_priority dp;
  dp.acquired_lock = lock;
  struct hash_elem *found = hash_find (table, &dp.elem);
  if (found != NULL)
    return hash_entry (found, struct donated_priority, elem);
  return NULL;
}

/* Finds and deletes ELEM in TABLE and frees memory allocated
   for its containing donated_priority instance.
   Returns true if found, false otherwise. */
bool
dp_table_delete (struct hash *table, struct hash_elem *elem)
{
  if (hash_delete (table, elem) != NULL)
    {
      donated_priority_hash_destroy (elem, NULL);
      return true;
    }
  return false;
}

/* Returns the maximum donated priority in TABLE
   or PRI_MIN if TABLE is empty. */
int
dp_table_max_priority (struct hash *table)
{
  int max = PRI_MIN;
  if (!hash_empty (table))
    {
      struct hash_iterator i;
      hash_first (&i, table);
      while (hash_next (&i))
        {
          struct donated_priority *dp
            = hash_entry (hash_cur (&i), struct donated_priority, elem);
          if (dp->value > max)
            max = dp->value;
        }
    }
  return max;
}
