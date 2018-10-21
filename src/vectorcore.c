
/////////////////////////////////////////////////////////////////////
// vectorcore.c - Most data is managed through alx_v32 structures,
// dynamic uint32_t vectors used for various purposes. Implements
// basic functions like adding elements to the vector; augmented by
// more specialised functions in vectorutils.c. 
/////////////////////////////////////////////////////////////////////

#include "alx.h"

/////////////////////////////////////////////////////////////////////
// alx_v32, dynamic vector of uint32_t elements
/////////////////////////////////////////////////////////////////////

void
alx_v32_clear(
  struct alx_v32 * const vector
) {

  vector->size = 0;

}

uint32_t*
alx_v32_data(
  struct alx_v32 const* const vector
) {
  return vector->d;
}

size_t
alx_v32_size(
  struct alx_v32 const* const vector
) {
  return vector->size;
}

uint32_t
alx_v32_at(
  struct alx_v32 const* const vector,
  size_t const index
) {

  assert( index < vector->size );

  return vector->d[ index ];
}

void
alx_v32_set_unsafe(
  struct alx_v32 const* const vector,
  size_t const index,
  uint32_t const value
) {
   vector->d[ index ] = value;
}

uint32_t *
alx_v32_last_ptr(
  struct alx_v32 const* const vector
) {
  return &(vector->d[ vector->size - 1 ]);
}

uint32_t *
alx_v32_first_ptr(
  struct alx_v32 const* const vector
) {
  return &(vector->d[ 0 ]);
}

uint32_t
alx_v32_get_last(
  struct alx_v32 const* const vector
) {

  assert( vector->size != 0 );
  
  return alx_v32_at(
    vector,
    alx_v32_size( vector ) - 1
  );

}

void
alx_v32_inc_last(
  struct alx_v32 const* const vector
) {

  assert( vector->size != 0 );

  alx_v32_set_unsafe(
    vector,
    alx_v32_size( vector ) - 1,
    alx_v32_at(
      vector,
      alx_v32_size( vector ) - 1
    ) + 1
  );

}

void
alx_v32_resize(
  struct alx_v32 *vector,
  size_t const size
) {

  assert( vector->capacity != 0 );

  size_t const new_size = size ? size : vector->capacity * 2;

  alx_parser_state_memrealloc(
    vector->p,
    (void **)&(vector->d),
    sizeof(*vector->d) * new_size
  );

  vector->capacity = new_size;

}


void
alx_v32_zerofill(
  struct alx_v32 *vector,
  size_t const size
) {

  alx_v32_resize(vector, size);
  memset(vector->d, 0, sizeof(*vector->d) * size);
  vector->size = size;

}

void
alx_v32_grow_and_push_back(
  struct alx_v32 *vector,
  uint32_t const value
) {

  alx_v32_resize(vector, 0);

  vector->d[ vector->size++ ] = value;

}

bool
alx_v32_empty(
  struct alx_v32 *vector
) {

  return vector->size == 0;  

}

uint32_t
alx_v32_pop(
  struct alx_v32 *vector
) {
  
  assert( vector->size != 0 );

  uint32_t result = alx_v32_at(vector, 
    alx_v32_size(vector) - 1);

  vector->size -= 1;

  return result;

}

void
alx_v32_push_back(
  struct alx_v32 *vector,
  alx_vertex const vertex
) {
  
  if (vector->size == vector->capacity) {

    alx_v32_grow_and_push_back(
      vector,
      vertex
    );

    return;

  }

  vector->d[ vector->size++ ] = vertex;

}

void
alx_v32_push_back2(
  struct alx_v32 * const vector,
  uint32_t const v1,
  uint32_t const v2
) {

  if (2 > vector->capacity - vector->size) {
    alx_v32_resize(vector, 0);
  }

  vector->d[ vector->size + 0 ] = v1;
  vector->d[ vector->size + 1 ] = v2;

  vector->size += 2;
}

void
alx_v32_push_back3(
  struct alx_v32 * const vector,
  uint32_t const v1,
  uint32_t const v2,
  uint32_t const v3
) {

  if (3 > vector->capacity - vector->size) {
    alx_v32_resize(vector, 0);
  }

  vector->d[ vector->size + 0 ] = v1;
  vector->d[ vector->size + 1 ] = v2;
  vector->d[ vector->size + 2 ] = v3;

  vector->size += 3;
}

void
alx_v32_push_back4(
  struct alx_v32 * const vector,
  uint32_t const v1,
  uint32_t const v2,
  uint32_t const v3,
  uint32_t const v4
) {

  if (4 > vector->capacity - vector->size) {
    alx_v32_resize(vector, 0);
  }

  vector->d[ vector->size + 0 ] = v1;
  vector->d[ vector->size + 1 ] = v2;
  vector->d[ vector->size + 2 ] = v3;
  vector->d[ vector->size + 3 ] = v4;

  vector->size += 4;
}

void
alx_v32_import(
  struct alx_v32 * const dst,
  struct alx_v32 const* const src
) {

  while (dst->capacity < src->size) {
    alx_v32_resize(dst, 0);
  }

  memcpy(dst->d, src->d, src->size * sizeof(*dst->d));
  dst->size = src->size;

}

void
alx_v32_push_back_pair(
  struct alx_v32 * const vector,
  struct alx_vertexp_pair const* const pair
) {

  alx_v32_push_back4(
    vector,
    pair->src_pos,
    pair->src_vertex,
    pair->dst_pos,
    pair->dst_vertex
  );

}

void
alx_v32_push_back_quad(
  struct alx_v32 * const vector,
  struct alx_vertexp_quad const* const quad
) {

  alx_v32_push_back4(
    vector,
    quad->src_pos,
    quad->src_vertex,
    quad->mid_src_pos,
    quad->mid_src_vertex
  );
  alx_v32_push_back4(
    vector,
    quad->mid_dst_pos,
    quad->mid_dst_vertex,
    quad->dst_pos,
    quad->dst_vertex
  );

}

struct alx_v32
alx_v32_new(
  struct alx_parser_state * const p,
  struct alx_v32 * const self,
  char const* const native_name,
  char const* const table_name,
  char const* const insert_template
) {

  struct alx_v32 vector = {
    .p = p,
    .native_name = native_name,
    .table_name = table_name,
    .insert_template = insert_template,
    .cardinality = 0, // TODO: use this
    .capacity = 16*1024,
    .size = 0,
    .d = NULL
  };

  alx_parser_state_memrealloc(
    p,
    (void**)&(vector.d),
    sizeof(*vector.d) * vector.capacity
  );

  p->vector_list[ p->vector_count++ ] = self;

  return vector;
}

void
alx_v32_free(
  struct alx_v32 * const vector
) {

  if (vector->d != NULL) {
    free( vector->d );
  }

}

