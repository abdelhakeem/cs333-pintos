#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"


static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
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
      //call halt
      printf("halt\n");
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
