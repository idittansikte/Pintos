#include <stdlib.h>
#include "pagedir.h"
#include "thread.h"


/* verfy_*_lenght är tänkta att användas i systemanrop som får in
 * opålitliga adresser från user mode. Operativsystemet ska inte kunna
 * luras att å processens vägnar använda adresser processen inte har
 * tillgång till. I pagedir.h finns funktioner som kan hjälpa dig slå
 * upp adresser i pagetable. Fundra hur du kan göra så få slagningar
 * som möjligt.
 *
 * Rekommenderat kompileringskommando:
 *
 *  gcc -Wall -Wextra -std=gnu99 -pedantic -g pagedir.o verify_adr.c
 */


/* Kontrollera alla adresser från och med start till och inte med
 * (start+length). */
bool verify_fix_length(void* start, int length)
{

  char* cur_page = (char*)pg_round_down(start);
  length += (int)((char*)start - cur_page)-1;
  for(;;)
    {
      if(pagedir_get_page(thread_current()->pagedir, (void*)cur_page) == NULL)
	return false;

      if(length < PGSIZE)
	return true;
      length -= PGSIZE;
      cur_page += PGSIZE;
    }
}

/* Kontrollera alla adresser från och med start till och med den
 * adress som först innehåller ett noll-tecken, `\0'. (C-strängar
 * lagras på detta sätt.) */
bool verify_variable_length(char* start)
{
  char* cur_page = (char*)pg_round_down((void*)start);
  char* cur_char = start;
  for(;;)
    {
      if(pagedir_get_page(thread_current()->pagedir, (void*)cur_page) == NULL)
	return false;
      
      for(;;)
	{

	  if(is_end_of_string(cur_char) == false)
	    ++cur_char;
	  else
	    return true;
	  
	  if(cur_char == cur_page+PGSIZE)
	    break;
	}
      cur_page += PGSIZE;
    }
}

/* Definition av testfall */
struct test_case_t
{
  void* start;
  unsigned length;
};

#define TEST_CASE_COUNT 6

const struct test_case_t test_case[TEST_CASE_COUNT] =
{
  {(void*)100, 100}, /* one full page */
  {(void*)199, 102},
  {(void*)101, 98},
  {(void*)250, 190},
  {(void*)250, 200},
  {(void*)250, 210}
};

/* Huvudprogrammet utvärderar din lösning. */
int main(int argc, char* argv[])
{
  int i;
  bool result;

  if ( argc == 2 )
  {
    simulator_set_pagefault_time( atoi(argv[1]) );
  }
  thread_init();
  
  /* Testa algoritmen med ett givet intervall (en buffer). */
  for (i = 0; i < TEST_CASE_COUNT; ++i)
  {
    start_evaluate_algorithm(test_case[i].start, test_case[i].length);
    result = verify_fix_length(test_case[i].start, test_case[i].length);
    evaluate(result);
    end_evaluate_algorithm();
  }
    
  //  Testa algoritmen med en sträng.
  for (i = 0; i < TEST_CASE_COUNT; ++i)
  {
    start_evaluate_algorithm(test_case[i].start, test_case[i].length);
    result = verify_variable_length(test_case[i].start);
    evaluate(result);
    end_evaluate_algorithm();
  }
  return 0;
}
