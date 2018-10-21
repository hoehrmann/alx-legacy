#include "alx.h"

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

void
alx_unordered_vertex_set_insert(
  struct alx_unordered_vertex_set * const self,
  alx_vertex const vertex
) {

  if ( !self->index[ vertex ] ) {

    self->vector[ self->size++ ] = vertex;
    self->index[ vertex ] = 1;

  }

}

uint32_t
alx_unordered_vertex_set_has(
  struct alx_unordered_vertex_set const* const self,
  alx_vertex const vertex
) {

  return self->index[ vertex ];

}

void
alx_unordered_vertex_set_clear(
  struct alx_unordered_vertex_set * const self
) {

  for (size_t ix = 0; ix < self->size; ++ix) {
    self->index[ self->vector[ ix ] ] = 0;
  }

  self->size = 0;
}

void 
alx_unordered_vertex_set_swap(
  struct alx_unordered_vertex_set ** const x,
  struct alx_unordered_vertex_set ** const y
) {

  // TODO: looks like some const modifiers are missing.

  struct alx_unordered_vertex_set * const tmp = *x;
  *x = *y;
  *y = tmp;

}
