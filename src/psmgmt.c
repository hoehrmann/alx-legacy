#include "alx.h"

/////////////////////////////////////////////////////////////////////
// Handle fatal errors
/////////////////////////////////////////////////////////////////////

void
alx_ps_longjmp(
  struct alx_parser_state const* const p,
  int code
) {

#if 1

  // As of October 2018, there is no need to longjmp out of here.
  // This is intended as a reminder to keep doing that an option.

  alx_logf( p, "FATAL ERROR\n" );
  exit(1);

#else

  // FIXME: unclear what could be done instead.
  struct alx_parser_state * p_rw =
    (struct alx_parser_state *)p;

  longjmp(p_rw->fatal_jmp_buf, code);

#endif

}

/////////////////////////////////////////////////////////////////////
// Parser state utility functions
/////////////////////////////////////////////////////////////////////

struct alx_vertexp_pair
alx_ps_get_endpoints(
  struct alx_parser_state const* const p
) {

  struct alx_vertexp_pair const pair = {
    1,
    alx_grammar_start_vertex( alx_g(p) ),
    p->final_index,
    alx_grammar_final_vertex( alx_g(p) )
  };

  return pair;

}

/////////////////////////////////////////////////////////////////////
// Memory management
/////////////////////////////////////////////////////////////////////

void
alx_parser_state_memrealloc(
  struct alx_parser_state * const p,
  void ** const ptr,
  size_t const new_size
) {

  void * reallocated = realloc(*ptr, new_size);

  if (reallocated == NULL) {
    alx_ps_longjmp(p, 2);
  }

  *ptr = reallocated;

}

void
alx_parser_state_memfree(
  struct alx_parser_state * const p,
  void * const ptr
) {

  free(ptr);

}

/////////////////////////////////////////////////////////////////////
// Grammar pointer retrieval
/////////////////////////////////////////////////////////////////////

struct alx_grammar const * 
alx_g(
  struct alx_parser_state const* const p
) {
  return &global_grammar;
}

