#include <stddef.h>

#include "flist.h"
  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void map_init(struct map* m){
  unsigned i;
  for(i = 0; i < MAP_SIZE; ++i){
    m->content[i] = NULL;
  }
}

key_t map_insert(struct map* m, value_t v){

  unsigned i;
  for(i = 0; i < MAP_SIZE; ++i){ // If value dont exist
    if(m->content[i] == NULL){
      m->content[i] = v; // Insert it
      return i+2;
    }
  }
  return -1;
}

value_t map_find(struct map* m, key_t k){
  
  if(k-2 < MAP_SIZE && k-2 >= 0 && m->content[k-2] != NULL){
    return m->content[k-2];
  }
  return NULL;
}

value_t map_remove(struct map* m, key_t k){
  if(k-2 < 0 || k-2 > MAP_SIZE){
    return NULL;
  }
  value_t value = m->content[k-2];
  m->content[k-2] = NULL;
  return value;
  //value.free();
}

void map_for_each(struct map* m, void (*exec)(key_t k, value_t v, int aux), int aux){
  int i;
  for(i = 0; i < MAP_SIZE; ++i){
    if(m->content[i] != NULL){
      exec(i, m->content[i], aux);
    }
  }
}

void map_remove_if(struct map* m, bool (*cond)(key_t k, value_t v, int aux), int aux){
  int i;
  for(i = 0; i < MAP_SIZE; ++i){
    if(m->content[i] != NULL){
      if(cond(i, m->content[i], aux)){
	//free(m->content[i]);
	m->content[i] = NULL;
      }
    }
  }
}

void map_clean(struct map* m){
  int i;
  for(i = 0; i < MAP_SIZE; ++i){
    if(m->content[i] != NULL){
      //free(m->content[i]);
      m->content[i] = NULL;
    }
  }
}
