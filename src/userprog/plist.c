#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "threads/malloc.h"
#include "plist.h"

void plist_init(struct plist* l){
  unsigned i;
  for(i=0; i<PLIST_SIZE; ++i){
    l->content[i].free = true;
    printf("Init %i\n", i);
  }
}

int plist_insert(struct plist* l, pid_t parent_id, pid_t process_id, char name[]){
  unsigned i;
  for(i=0; i<PLIST_SIZE;++i){
    if(l->content[i].free){
      l->content[i].free = false;
      l->content[i].process_id = process_id;
      l->content[i].parent_id = parent_id;
      strlcpy(l->content[i].process_name, name, 64);
      l->content[i].exit_status = -1;
      l->content[i].alive = true;
      l->content[i].parent_alive = true;
      return process_id;
    }
  }
  return -1;
}

static 
int plist_find(struct plist* l, pid_t process_id){
  unsigned i;
  for(i=0; i<PLIST_SIZE; ++i){
    if(l->content[i].free == false && l->content[i].process_id == process_id){
      return i;
    }
  }
  return -1;
}

pid_t plist_get_parent(struct plist* l, pid_t process_id){
  int index = plist_find(l, process_id);
  if(index != -1){
    return l->content[index].parent_id;
  }
  return -1;
}

bool plist_alive(struct plist* l, pid_t process_id){
int index = plist_find(l, process_id);
  if(index != -1){
    return l->content[index].alive;
  }
  return false;
}

bool plist_parent_alive(struct plist* l, pid_t process_id){
int index = plist_find(l, process_id);
  if(index != -1){
    return l->content[index].parent_alive;
  }
  return false;
}

int plist_get_exit_status(struct plist* l, pid_t process_id){
  int index = plist_find(l, process_id);
  if(index != -1){
    int exit_status = l->content[index].exit_status;
    /* If someone (parent) take exit_status, nothing more to be done.*/
    l->content[index].free = true;
    return exit_status;
  }
  return -1;
}

void plist_set_exit_status(struct plist* l, pid_t process_id, int exit_status){
  int index = plist_find(l, process_id);
  if(index != -1){
    l->content[index].exit_status = exit_status;
  }
}

void plist_print(struct plist* l){
  printf("---------------------------------------------------------------------------------------\n");
  printf("%-15s%-15s%-15s%-18s%-12s%s\n", "ProcessID", "Process Name","ParentID", "Exit Status", "Alive", "Parent Alive" );
  printf("---------------------------------------------------------------------------------------\n");
  unsigned i;
  unsigned occupied = 0;
  for(i=0; i<PLIST_SIZE; ++i){
    if(l->content[i].free == false){
      ++occupied;
      printf("%4s%-12i%-18s%-14i%-15i%-17s%s\n", "", l->content[i].process_id, l->content[i].process_name, l->content[i].parent_id, l->content[i].exit_status, l->content[i].alive ? "true" : "false", l->content[i].parent_alive ? "true" : "false");
    }
  }
  printf("---------------------------------------------------------------------------------------\n");
  printf("Occupied slots in list: %i/%i\n", occupied, PLIST_SIZE );
  printf("---------------------------------------------------------------------------------------\n");
}

void plist_remove(struct plist* l, pid_t process_id){
int index = plist_find(l, process_id);
  if(index != -1){
    /*1. Set process as dead but keep him in list*/
    l->content[index].alive = false;
    /*2. Find and set all childs for process and set parent dead*/
    unsigned i;
    for(i=0;i<PLIST_SIZE;++i){
      if(l->content[i].parent_id == process_id){ //If child found
	l->content[i].parent_alive = false;
	if(l->content[i].alive == false){ //If child dead
	  l->content[i].free = true; //No reason to keep her in list, parent can't retrive exit_status
	}
      }
    }
    /*3. If parent dead, there is no reason to keep process, nobody will need exit_status*/
    if(l->content[index].parent_alive == false){
      l->content[index].free = true;
    }
  }
}
