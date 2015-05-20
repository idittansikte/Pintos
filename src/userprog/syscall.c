#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

/* header files you probably need, they are not used yet */
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/input.h"
#include "devices/timer.h"

#define DBG(format, ...) printf(format "\n", ##__VA_ARGS__);

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


/* This array defined the number of arguments each syscall expects.
   For example, if you want to find out the number of arguments for
   the read system call you shall write:
   
   int sys_read_arg_count = argc[ SYS_READ ];
   
   All system calls have a name such as SYS_READ defined as an enum
   type, see `lib/syscall-nr.h'. Use them instead of numbers.
 */
const int argc[] = {
  /* basic calls */
  0, 1, 1, 1, 2, 1, 1, 1, 3, 3, 2, 1, 1, 
  /* not implemented */
  2, 1,    1, 1, 2, 1, 1,
  /* extended */
  0
};


/* Kontrollera alla adresser från och med start till och inte med
 * (start+length). */
static bool 
verify_fix_length(const char* start, unsigned length)
{
  if( start + length >= (char*)PHYS_BASE)
    return false;

  char* cur_page = (char*)pg_round_down(start);

  for(;;)
    {
      if(pagedir_get_page(thread_current()->pagedir, cur_page) == NULL)
	return false;

      cur_page += PGSIZE;
      if(cur_page >= start + length)
	return true;
    }
}

/* Kontrollera alla adresser från och med start till och med den
 * adress som först innehåller ett noll-tecken, `\0'. (C-strängar
 * lagras på detta sätt.) */
static bool 
verify_variable_length(const char* start)
{
  char* cur_page = (char*)pg_round_down((void*)start);
  char* cur_char = (char*)start;
  for(;;)
    {
      if(pagedir_get_page(thread_current()->pagedir, (void*)cur_page) == NULL)
	return false;
      
      for(;;)
	{
	  if(cur_char >= (char*)PHYS_BASE)
	    return false;

	  if(*cur_char != '\0')
	    ++cur_char;
	  else
	    return true;
	  
	  if(cur_char == cur_page+PGSIZE)
	    break;
	}
      cur_page += PGSIZE;
    }
}


static void
syscall_handler (struct intr_frame *f) 
{
  int32_t* esp = (int32_t*)f->esp;
  
  if( verify_fix_length((char*)esp, 4) == false){
    process_exit(-1);
  }
      
  switch ( esp[0] /* retrive syscall number */ )
    {
    case SYS_HALT:
      {
	DBG("# Running power_off() %i %s", __LINE__, __FILE__);
	power_off();
	break;
      }
    case SYS_EXIT:
      {
	if( verify_fix_length((char*)esp+1, 4) == false){
	  process_exit(-1);
	}
	DBG("# Running sys_call exit() Return value: %i %i %s", esp[1], __LINE__, __FILE__);
        process_exit((int)esp[1]);
	break;
      }
    case SYS_READ:
      {
	//DBG("# Running sys_call SYS_REAED() %i %s", __LINE__, __FILE__);

	if( verify_fix_length((char*)esp+1, 12) == false){
	  process_exit(-1);
	}

	int fd = (int)esp[1];
        uint8_t *buffer = (uint8_t*)esp[2];
	unsigned buffersize = (unsigned)esp[3];

	if(verify_fix_length((char*)buffer, buffersize) == false){
	  process_exit(-1);
	}

	if(fd == STDIN_FILENO){
	  unsigned i;
	  for(i = 0; i < buffersize; ++i){
	    uint8_t input = input_getc();
	    if(input == '\r'){
	      input = '\n';
	    }
	    buffer[i] = input;
	    //DBG("# Buffer: %i %i %s", buffer[i], __LINE__, __FILE__);
	    //printf("%s", i);
	    putbuf((const char*)&buffer[i], (size_t)1);
	  }
	  f->eax = esp[3];
	}
	else if(fd >= 2){
	  struct thread* curr_t = thread_current();
	  struct file* file = map_find(&curr_t->file_table, fd);
	  if(file != NULL){
	    int32_t bytes = file_read(file, buffer, buffersize);
	    f->eax = bytes;
	  }
	  else{
	    f->eax = -1;
	  }
	}
	else{
	  f->eax = -1;
	}
	break;
      }
    case SYS_WRITE:
      {
	if( verify_fix_length((char*)esp+1, 12) == false){
	  process_exit(-1);
	}
    	int fd = (int)esp[1];
	uint8_t *buffer = (uint8_t*)esp[2];
	unsigned buffersize = (unsigned)esp[3];

	if( verify_variable_length((char*)buffer) == false )
	  process_exit(-1);
	
    	if(fd == STDOUT_FILENO)
	  {
	    putbuf((const char*)buffer, (size_t)buffersize);
	    f->eax = esp[3];
	  }
	else if(fd >= 2 ){
	  struct thread* curr_t = thread_current();
	  struct file* file = map_find(&curr_t->file_table, fd);
	  if(file != NULL){
	    int32_t bytes = file_write(file, buffer, buffersize);
	    f->eax = bytes;
	  }
	  else{
	    f->eax = -1;
	  }
	}
	else{
	  f->eax = -1;
	}
    	break;
      }
    case SYS_OPEN:
      {
	if(verify_fix_length((char*)esp+1, 4) == false){
	  process_exit(-1);
	}

      	struct file* fp;
	const char* file_name = (char*)esp[1];

	if( verify_variable_length(file_name) == false )
	  process_exit(-1);

      	fp = filesys_open(file_name);
	if(fp != NULL){
	  struct thread* curr_t = thread_current();
	  int fd = map_insert(&curr_t->file_table, fp);
	  //DBG("# open fd = %i | %i %s |", fd, __LINE__, __FILE__);
	  f->eax = fd;
	}
	else{
	  f->eax = -1;
	}
	break;
      }
    case SYS_CLOSE:
      {
	if(verify_fix_length((char*)esp+1, 4) == false){
	  process_exit(-1);
	}

	int fd = (int)esp[1];
	
	if(fd <= 1){
	  f->eax = -1;
	  break;
	}
	
	struct file* fp;
	struct thread* curr_t = thread_current();
      	fp = map_remove(&curr_t->file_table, fd);
	if(fp != NULL)
	  filesys_close(fp);
	break;
      }
    case SYS_CREATE:
      {
	if(verify_fix_length((char*)esp+1, 8) == false){
	  process_exit(-1);
	}
	const char* name = (char*)esp[1];
	unsigned init_size = (unsigned)esp[2];

	if( verify_variable_length(name) == false )
	  process_exit(-1);

	bool error = filesys_create(name, init_size);
	f->eax = error;
	break;
      }
    case SYS_REMOVE:
      {
	if(verify_fix_length((char*)esp+1, 4) == false){
	  process_exit(-1);
	}

	const char* file_name = (char*)esp[1];
	
	if( verify_variable_length(file_name) == false )
	  process_exit(-1);

	bool error = filesys_remove(file_name);
	f->eax = error;
	break;
      }
    case SYS_SEEK:
      {
	if(verify_fix_length((char*)esp+1, 8) == false){
	  process_exit(-1);
	}

	int fd = (int)esp[1];
	int32_t new_pos = (int32_t)esp[2];
	struct thread* curr_t = thread_current();
	struct file* file = map_find(&curr_t->file_table, fd);
	int32_t file_len = file_length(file);
	if(new_pos <= file_len && new_pos >= 0)
	  file_seek(file, new_pos);
	break;
      }
    case SYS_TELL:
      {
	if(verify_fix_length((char*)esp+1, 4) == false){
	  process_exit(-1);
	}

	int fd = (int)esp[1];
	struct thread* curr_t = thread_current();
	struct file* file = map_find(&curr_t->file_table, fd);
	int32_t pos = file_tell(file);
	f->eax = pos;
	break;
      }
    case SYS_FILESIZE:
      {
	if(verify_fix_length((char*)esp+1, 4) == false){
	  process_exit(-1);
	}
	int fd = (int)esp[1];
	struct thread* curr_t = thread_current();
	struct file* file = map_find(&curr_t->file_table, fd);
	int32_t file_len = file_length(file);
	f->eax = file_len;
	break;
      }
    case SYS_PLIST:
      {
	process_print_list();
	break;
      }
    case SYS_SLEEP:
      {
	if(verify_fix_length((char*)esp+1, 4) == false){
	  process_exit(-1);
	}
	int64_t millis = (int64_t)esp[1];
	timer_sleep(millis);
	break;
      }
    case SYS_EXEC:
      {
	if(verify_fix_length((char*)esp+1, 4) == false){
	  process_exit(-1);
	}
	const char* cmd = (char*)esp[1];

	if( verify_variable_length(cmd) == false )
	  process_exit(-1);

	int cid = process_execute(cmd);
	f->eax = cid;
	break;
      }
    case SYS_WAIT:
      {
	if(verify_fix_length((char*)esp+1, 4) == false){
	  process_exit(-1);
	}
	int child_id = (int)esp[1];
	int status = process_wait(child_id);
	f->eax = status;
	break;
      }
    default:
      {
	printf ("Executed an unknown system call!\n");
      
	printf ("Stack top + 0: %d\n", esp[0]);
	printf ("Stack top + 1: %d\n", esp[1]);
	
	process_exit(-1);
	//thread_exit ();
      }
    }
}
