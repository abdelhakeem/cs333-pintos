#include "userprog/syscall.h"
#include <stdio.h>
#include <hash.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

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
          arg2[i] = input_getc ();
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

bool 
check_n_user_bytes (void * location, int n) {
  if (location <= 0 || location + n > PHYS_BASE)
    return false;
  int i;
  for (i = 0;i < n; i++)
    if (get_user () == -1)
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
      if (get_user () == -1)
        return false;
      else if (get_user () == 0)
        break;
      i++;
    }
  return true;
}

void
halt (void) {
    shutdown_power_off ();
}