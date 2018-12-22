#include "userprog/syscall.h"
#include <stdio.h>
#include <hash.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "process.h"


static void syscall_handler (struct intr_frame *);
static int generate_fd (struct file *);
static struct file * translate_fd (int fd);
static void remove_fd (int fd);
static bool check_n_user_bytes(void *, int n);
static bool check_user_name (const char *);

static struct lock files_lock;

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}
 
/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static bool
put_user (uint8_t *udst, uint8_t byte)
{
  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
       : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}

void
syscall_init (void) 
{
  lock_init (&files_lock);
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
  //printf("%08x\n%08x\n%08x\n%08x\n",sys_call_type,arg1,arg2,arg3);
  
  switch (sys_call_type)
  {
    case SYS_HALT:  /* Halt the operating system. */
    {
      //printf("halt\n");
      halt ();
      break;
    }
    case SYS_EXIT:  /* Terminate this process. */
    {
      //printf("exit\n");
      exit (arg1);
    }
    case SYS_EXEC:  /* Start another process. */
    {
      //printf("exec\n");
      f->eax = exec (arg1);
      break;
    }
    case SYS_WAIT:  /* Wait for a child process to die. */
    {
      //printf("wait\n");
      f->eax = wait(arg1);
      break;
    }
    case SYS_CREATE:  /* Create a file. */
    {
      lock_acquire (&files_lock);
      if (!check_user_name (arg1)) 
        {
          lock_release (&files_lock);
          break;
        }
      f->eax = filesys_create ((const char *) arg1, (off_t) arg2);
      lock_release (&files_lock);
      break;
    }
    case SYS_REMOVE:  /* Delete a file. */
    {
      lock_acquire (&files_lock);
      if (!check_user_name (arg1)) 
        {
          lock_release (&files_lock);
          break;
        }
      f->eax = filesys_remove ((const char *) arg1);
      lock_release (&files_lock);
      break;
    }
    case SYS_OPEN:  /* Open a file. */
    {
      lock_acquire (&files_lock);
      if (!check_user_name (arg1)) 
        {
          lock_release (&files_lock);
          break;
        }
      f->eax = generate_fd (filesys_open ((const char *) arg1));
      lock_release (&files_lock);
      break;
    }
    case SYS_FILESIZE:  /* Obtain a file's size. */
    {
      lock_acquire (&files_lock);
      f->eax = file_length (translate_fd (arg1));
      lock_release (&files_lock);
      break;
    }
    case SYS_READ:  /* Read from a file. */
    {
      lock_acquire (&files_lock);
      if (!check_n_user_bytes(arg2, arg3))
        {
          lock_release (&files_lock);
          break;
        }
      if(arg1 == 0) {
        unsigned i;
        for (i = 0;i < arg3;i++)
          *(char *)(arg2 + i) = input_getc ();
      }
      else
      {
        struct file *file = translate_fd (arg1);
        if (file == NULL)
          f->eax = -1;
        else
          f->eax = file_read (file, arg2, arg3);
      }
      lock_release (&files_lock);
      break;
    }
    case SYS_WRITE:  /* Write to a file. */
    {
      lock_acquire (&files_lock);
      if (!check_n_user_bytes(arg2, arg3))
        {
          lock_release (&files_lock);
          break;
        }
      if(arg1 == 1)
        putbuf (arg2,arg3);
      else
      {
        struct file *file = translate_fd (arg1);
        if (file == NULL)
          f->eax = -1;
        else
          f->eax = file_write (file, arg2, arg3);
      }
      lock_release (&files_lock);
      break;
    }
    case SYS_SEEK:  /* Change position in a file. */
    {
      lock_acquire (&files_lock);
      file_seek (translate_fd (arg1), arg2);
      lock_release (&files_lock);
      break;
    }
    case SYS_TELL:  /* Report current position in a file. */
    {
      lock_acquire (&files_lock);
      file_tell (translate_fd (arg1));
      lock_release (&files_lock);
      break;
    }
      case SYS_CLOSE:  /* Close a file. */
    {
      lock_acquire (&files_lock);
      file_close (translate_fd (arg1));
      remove_fd (arg1);
      lock_release (&files_lock);
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
  //printf ("system call!\n");
  //thread_exit ();
}

int 
generate_fd (struct file *file) {
  if (file == NULL)
    return -1;
  struct thread* cur = thread_current ();
  struct file_desc* file_desc = (struct file_desc*) malloc (sizeof (struct file_desc));
  file_desc->fd = &cur->process.next_file_fd;
  file_desc->file = file;
  hash_insert (&cur->process.file_descriptors, &file_desc->hash_elem);
  return cur->process.next_file_fd++;
}

static struct file * 
translate_fd (int fd) {
  struct thread* cur = thread_current ();
  struct file_desc p;
  struct hash_elem *e;
  p.fd = fd;
  e = hash_find (&cur->process.file_descriptors, &p.hash_elem);
  return e != NULL ? hash_entry (e, struct file_desc, hash_elem) : NULL;
}

void
remove_fd (int fd) {
  struct thread* cur = thread_current ();
  struct file_desc p;
  p.fd = fd;
  hash_delete (&cur->process.file_descriptors, &p.hash_elem);
}

bool 
check_n_user_bytes (void * location, int n) {
  if (location <= 0 || location + n > PHYS_BASE)
    return false;
  int i;
  for (i = 0;i < n; i++)
    if (get_user (location + i) == -1)
      return false;
  return true;
}

bool 
check_user_name (const char * name) {
  if (name <= 0 || name >= PHYS_BASE)
    return false;
  int i = 0;
  while (true) 
    {
      if (name + i >= PHYS_BASE)
        return false;
      int byte = get_user (name + i);
      if (byte == -1)
        return false;
      else if (byte == 0)
        break;
      i++;
    }
  return true;
}

void
halt (void) {
  shutdown_power_off ();
}

void exit (int status) {
  struct thread* cur = thread_current ();
  /* Close all open files */
  
  struct hash *file_descriptors = &cur->process.file_descriptors;
  if (!hash_empty (file_descriptors)) {
    struct hash_iterator i;
    hash_first(&i, file_descriptors);
    while (hash_next(&i)) {
      struct file_desc *file_desc1
            = hash_entry (hash_cur(&i), struct file_desc, hash_elem);
      //close (file_desc1->file);
      lock_acquire (&files_lock);
      file_close (translate_fd (file_desc1->file));
      remove_fd (file_desc1->file);
      lock_release (&files_lock);
      free (file_desc1);
    }
  }
  /*struct list *file_descriptors = &cur->process.file_descriptors;
  while (!list_empty (file_descriptors)) {
    struct list_int_container *container =
            list_entry(list_pop_front (file_descriptors), struct list_int_container, elem);
    close (container->value);
    free (container);
  }*/
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
  thread_exit();
}

int wait (pid_t pid) {
  return process_wait (pid);
}

pid_t exec (const char *cmd_line) {
  // TODO: H: Parse command line here, Abdelhakeem
  return process_execute (cmd_line);
}

void close (int fd) {
  // TODO: H: Implement this, Ayman
  printf("H says you should close file descriptor %d", fd);
}
