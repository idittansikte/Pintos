/* MAP for filestructure */
#include "map.h"
#include <stdlib.h>

void map_init(struct map* m){
  unsigned i;
  for(i = 0; i < MAP_SIZE; ++i){
    m->content[i] = NULL;
  }
}

key_t map_insert(struct map* m, value_t v){
  unsigned i;
  for(i = 0; i < MAP_SIZE; ++i){
    if(m->content[i] == NULL){
      m->content[i] = v;
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
  value_t value = m->content[k-2];
  m->content[k-2] = NULL;
  return value;
  //value.free();
}
