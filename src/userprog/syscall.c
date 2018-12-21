#include "userprog/syscall.h"
#include <stdio.h>
#include <hash.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

static void syscall_handler (struct intr_frame *);
static int generate_fd (struct file *);
static struct file * translate_fd (int fd);
static void remove_fd (int fd);

static struct lock files_lock;

void
syscall_init (void) 
{
  lock_init (&files_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
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
      halt();
      break;
    }
    case SYS_EXIT:  /* Terminate this process. */
    {
      //call exit
      printf("exit\n");
      break;
    }
    case SYS_EXEC:  /* Start another process. */
    {
      //call exec
      printf("exec\n");
      break;
    }
    case SYS_WAIT:  /* Wait for a child process to die. */
    {
      //call wait
      printf("wait\n");
      break;
    }
    case SYS_CREATE:  /* Create a file. */
    {
      // Todo: Check for const char *file in arg1
      lock_acquire (&files_lock);
      f->eax = filesys_create ((const char *) arg1, (off_t) arg2);
      lock_release (&files_lock);
      break;
    }
    case SYS_REMOVE:  /* Delete a file. */
    {
      // Todo: Check for const char *file in arg1
      lock_acquire (&files_lock);
      f->eax = filesys_remove ((const char *) arg1);
      lock_release (&files_lock);
      break;
    }
    case SYS_OPEN:  /* Open a file. */
    {
      // Todo: Check for const char *file in arg1
      lock_acquire (&files_lock);
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
      // Todo: Check for buffer* in arg2
      if(arg1 == 0)
        // Todo
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
      // Todo: Check for buffer* in arg2
      if(arg1 == 1)
        putbuf(arg2,arg3);
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
  printf ("system call!\n");
  thread_exit ();
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
  return &cur->process.next_file_fd++;
}

static struct file * 
translate_fd (int fd) {
  struct file_desc p;
  struct hash_elem *e;
  p.fd = fd;
  e = hash_find (&cur->process.file_descriptors, &p.hash_elem);
  return e != NULL ? hash_entry (e, struct file_desc, hash_elem) : NULL;
}

void
remove_fd (int fd) {
  struct file_desc p;
  p.fd = fd;
  hash_delete (&cur->process.file_descriptors, &p.hash_elem);
}

void
halt (void) {
    shutdown_power_off ();
}