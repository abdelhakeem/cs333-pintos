#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "threads/synch.h"
#include "process.h"
#include "threads/malloc.h"

static void syscall_handler (struct intr_frame *);


void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

  lock_init (&waiting_lock);
  hash_init (&zombies, process_hash_func, process_hash_less, NULL);
  hash_init (&waiting_parents, process_hash_func, process_hash_less, NULL);
}

static void
syscall_handler (struct intr_frame *f) 
{
  //hex_dump(0,f->esp,200,true);	

  int *esp = f->esp;
  int sys_call_type = esp[0];
  int arg1 = esp[1];
  int arg2 = esp[2];
  int arg3 = esp[3];
  printf("%08x\n%08x\n%08x\n%08x\n",sys_call_type,arg1,arg2,arg3);
  
  switch (sys_call_type)
  {
    case SYS_HALT:  /* Halt the operating system. */
    {
      printf("halt\n");
      halt ();
      break;
    }
    case SYS_EXIT:  /* Terminate this process. */
    {
      printf("exit\n");
      exit (arg1);
    }
    case SYS_EXEC:  /* Start another process. */
    {
      //call exec
      printf("exec\n");
      break;
    }
    case SYS_WAIT:  /* Wait for a child process to die. */
    {
      printf("wait\n");
      wait(arg1);
      // TODO: H: Push output (return status) to stack
      break;
    }
    case SYS_CREATE:  /* Create a file. */
    {
      //call create
      printf("create\n");
      break;
    }
    case SYS_REMOVE:  /* Delete a file. */
    {
      //call remove
      printf("remove\n");
      break;
    }
    case SYS_OPEN:  /* Open a file. */
    {
      //call open
      printf("open\n");
      break;
    }
    case SYS_FILESIZE:  /* Obtain a file's size. */
    {
      //call fileSize
      printf("fileSize\n");
      break;
    }
    case SYS_READ:  /* Read from a file. */
    {
      //call read
      printf("read\n");
      break;
    }
    case SYS_WRITE:  /* Write to a file. */
    {
      //call write
      if(arg1 == 1)
      {
        putbuf(arg2,arg3);
      }
      printf("write\n");
      break;
    }
    case SYS_SEEK:  /* Change position in a file. */
    {
      //call seek
      printf("seek\n");
      break;
    }
    case SYS_TELL:  /* Report current position in a file. */
    {
      //call tell
      printf("tell\n");
      break;
    }
      case SYS_CLOSE:  /* Close a file. */
    {
      //call close
      printf("close\n");
      break;
    }
    /* Project 3 and optionally project 4. */
    case SYS_MMAP:  /* Map a file into memory. */
    {
      //call MMAP
      printf("MMAP\n");
      break;
    }
    case SYS_MUNMAP:  /* Remove a memory mapping. */
    {
      //call MUNMAP
      printf("MUNMAP\n");
      break;
    }
    /* Project 4 only. */
     case SYS_CHDIR: /* Change the current directory. */
    {
      //call CHDIR
      printf("CHDIR\n");
      break;
    }
    case SYS_MKDIR: /* Create a directory. */
    {
      //call MKDIR
      printf("MKDIR\n");
      break;
    }
    case SYS_READDIR: /* Reads a directory entry. */
    {
      //call READDIR
      printf("READDIR\n");
      break;
    }
    case SYS_ISDIR: /* Tests if a fd represents a directory. */
    {
      //call ISDIR
      printf("ISDIR\n");
      break;
    }
    case SYS_INUMBER: /* Returns the inode number for a fd. */
    {
      //call INUMBER
      printf("INUMBER\n");
      break;
    }
    default:
    {
      PANIC("unKnown system call 04%x\n",sys_call_type);
    }
  }
  printf ("system call!\n");
  thread_exit ();
}

void
halt (void) {
  shutdown_power_off ();
}

void exit (int status) {
  struct thread* cur = thread_current ();
    /* Close all open files */
  struct list *file_descriptors = &cur->process.file_descriptors;
  while (!list_empty (file_descriptors)) {
    struct list_int_container *container =
            list_entry(list_pop_front (file_descriptors), struct list_int_container, elem);
    close (container->value);
    free (container);
  }
  /* Kill zombie children */
  lock_acquire (&waiting_lock);
  list_int_destroy_all (&cur->process.children);
  struct list *children = &cur->process.children;
    while (!list_empty (children)) {
      struct list_int_container *container =
              list_entry(list_pop_front (children), struct list_int_container, elem);
      struct process_hash *zombie_searcher = process_lookup (&zombies, container->value);
      if (zombie_searcher != NULL) {
        hash_delete (&zombies, &zombie_searcher->elem);
        free (zombie_searcher);
      }
      free (container);
    }
    /* Awake waiting parent if exists */
    struct process_hash *parent_searcher = process_lookup (&waiting_parents, cur->tid);
    if (parent_searcher != NULL) {
      hash_delete (&waiting_parents, &parent_searcher->elem);
      parent_searcher->status = status;
      thread_unblock (parent_searcher->t);
    } else {
      /* Build zombie_container for hashing into zombies */
      struct process_hash *zombie_container =
              (struct process_hash *) malloc (sizeof (struct process_hash));
      zombie_container->id = cur->tid;
      zombie_container->key = cur->tid;
      zombie_container->status = status;

      hash_insert (&zombies, &zombie_container->elem);
    }
    lock_release (&waiting_lock);

    list_int_destroy_all (&cur->process.confirmed_dead_children);
}

int wait (pid_t pid) {
  struct thread* parent = thread_current ();
  int status = -1;
  if(!list_int_contains (&parent->process.children, pid)) {
    return -1; // Not a child
  }
  if (!list_int_contains (&parent->process.confirmed_dead_children, pid)) {
    return -1; // Confirmed dead
  }

  lock_acquire (&waiting_lock);
  struct process_hash *searcher = process_lookup (&zombies, pid);
  if (searcher != NULL) { // Child is zombie
    hash_delete (&zombies, &searcher->elem);
    status = searcher->status;
    free (searcher);
    lock_release (&waiting_lock);
  } else {
    /* Build parent_container for hashing into waiting_parents */
    struct process_hash *parent_container =
            (struct process_hash *) malloc (sizeof (struct process_hash));
    parent_container->id = parent->tid;
    parent_container->key = pid;
    parent_container->t = parent;

    hash_insert (&waiting_parents, &parent_container->elem);
    lock_release (&waiting_lock);

    thread_block ();
    /* Awaken by child here */
    status = parent_container->status; // Updated by child
    free(parent_container);
  }
  struct list_int_container *death_log_container =
          (struct list_int_container *) malloc (sizeof (struct list_int_container));
  death_log_container->value = pid;

  list_push_front (&parent->process.confirmed_dead_children, &death_log_container->elem);
  return status;
}


void close (int fd) {
  // TODO: H: Implement this.
  printf("H says you should close file descriptor %d", fd);
}
