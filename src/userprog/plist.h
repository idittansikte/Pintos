#ifndef _PLIST_H_
#define _PLIST_H_


/* Place functions to handle a running process here (process list).
   
   plist.h : Your function declarations and documentation.
   plist.c : Your implementation.

   The following is strongly recommended:

   - A function that given process inforamtion (up to you to create)
     inserts this in a list of running processes and return an integer
     that can be used to find the information later on.

   - A function that given an integer (obtained from above function)
     FIND the process information in the list. Should return some
     failure code if no process matching the integer is in the list.
     Or, optionally, several functions to access any information of a
     particular process that you currently need.

   - A function that given an integer REMOVE the process information
     from the list. Should only remove the information when no process
     or thread need it anymore, but must guarantee it is always
     removed EVENTUALLY.
     
   - A function that print the entire content of the list in a nice,
     clean, readable format.
     
 */
#include <stdbool.h>
#include "threads/synch.h"

#define PLIST_SIZE 300
struct lock lock_plist;
typedef struct Node node_t;
typedef int pid_t;

struct Node{
  bool free;
  pid_t process_id;
  char process_name[64];
  pid_t parent_id;
  int exit_status;
  bool alive;
  bool parent_alive;
  struct semaphore sema_wait;
};

struct plist{
  node_t content[PLIST_SIZE]; 
};

void plist_init(struct plist* l);

int plist_insert(struct plist* l, pid_t parent_id, pid_t process_id, char name[]);

pid_t plist_get_parent(struct plist* l, pid_t process_id);
bool plist_alive(struct plist* l, pid_t process_id);
bool plist_parent_alive(struct plist* l, pid_t process_id);
int plist_get_exit_status(struct plist* l, pid_t process_id);
void plist_set_exit_status(struct plist* l, pid_t process_id, int exit_status);
void plist_print(struct plist* l);

int plist_exit_status(struct plist* l, pid_t process_id);

void plist_remove(struct plist* l, pid_t process_id);

#endif
