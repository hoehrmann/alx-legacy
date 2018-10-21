
/////////////////////////////////////////////////////////////////////
// vectorutils.c - More specialised alx_v32 (dynamic uint32_t vector)
// functions for searching, sorting, and other purposes that build on
// those implemented in vectorcore.c.
/////////////////////////////////////////////////////////////////////

#include "alx.h"

bool
alx_v32_is_sorted(
  struct alx_v32 const* const vector,
  size_t const skip_num,
  size_t const num,
  size_t const size,
  int (*cmp)(void const*, void const*)
) {

  for (size_t current = 1 + skip_num; current < num; ++current) {
    char const* const lhs = (char const*)vector->d + (current - 1)*size;
    char const* const rhs = lhs + size;

    int result = cmp(lhs, rhs);

    if (result > 0) {
      alx_debugf(stderr, "unsorted at %zu (cmp %d)\n", current, result);
      alx_dbg_break();
      return false;
    }
  }

  return true;

}

uint32_t const*
alx_v32_bsearch(
  struct alx_v32 const* const vector,
  size_t const skip_num, // uint32_ts
  void const* const key,
  size_t const num,      // 
  size_t const size,     // bytes per item
  int (*cmp)(void const*, void const*)
) {

#if ALX_DEBUG
  assert(

    alx_v32_is_sorted(
      vector,
      skip_num,
      num,
      size,
      cmp
    )
  );
#endif

  return (uint32_t *)bsearch(
    key,
    vector->d + skip_num,
    num,
    size,
    cmp
  );

}

void
alx_upper_bound(
  void const* const key,
  void const* const base,
  size_t const num,
  size_t const size,
  int (*cmp)(void const*, void const*),
  void * const ptr,
  void ** const upper
) {

  *upper = ptr;

  if (ptr == NULL)
    return;

  char const* const end = (char const*)base + num * size;

  do {

    *(char **)upper += size;

  } while ((char *)*upper < end && cmp(*upper, key) == 0);

}

void
alx_lower_bound(
  void const* const key,
  void const* const base,
  size_t const num,
  size_t const size,
  int (*cmp)(void const*, void const*),
  void * const ptr,
  void ** const lower
) {

  *lower = ptr;

  if (ptr == NULL)
    return;

  while (*lower > base) {

    *(char **)lower -= size;

    if (cmp(*lower, key) != 0) {
      *(char **)lower += size;
      break;
    }

  }

}

void
alx_equals_range(
  void const* const key,
  void const* const base,
  size_t const num,
  size_t const size,
  int (*cmp)(void const*, void const*),
  void ** const lower,
  void ** const upper
) {

  void * const ptr = bsearch(key, base, num, size, cmp);

  alx_lower_bound( key, base, num, size, cmp, ptr, lower );
  alx_upper_bound( key, base, num, size, cmp, ptr, upper );

}

uint32_t*
alx_v32_bsearch_random(
  struct alx_v32 const* const vector,
  size_t const skip_num,
  void const* const key,
  size_t const num,
  size_t const size,
  int (*cmp)(void const*, void const*),
  struct alx_rng * const rng
) {

  // TODO: this whole function needs an overhaul

  size_t const uint32_ts_per_item = size / sizeof(uint32_t);

  void * const base = vector->d + skip_num;
  void * upper;
  void * lower;
  void * const ptr = bsearch( key, base, num, size, cmp );

  alx_lower_bound( key, base, num, size, cmp, ptr, &lower );
  alx_upper_bound( key, base, num, size, cmp, ptr, &upper );

  // FIXME: casts only in special casting functions
  size_t const ptr_ix = (uint32_t*)ptr - vector->d;
  size_t const lower_ix = (uint32_t*)lower - vector->d;
  size_t const upper_ix = (uint32_t*)upper - vector->d;
  size_t const range_diff = (uint32_t*)upper - (uint32_t*)lower;

  if (!lower || !upper) {
    // ???
    assert( 0 );
    return NULL;
  }

  // FIXME: int is not a good type here?
  // FIXME: now size_t, still not perfect 
  size_t random_ix = uint32_ts_per_item * (alx_rng_minmax(
    rng,
    0,
    alx_size_t_to_int( range_diff / uint32_ts_per_item - 1 )
  )) + lower_ix;

  uint32_t * const result = vector->d + random_ix;

  return result;

}

void
alx_v32_equals_range(
  struct alx_v32 const* const vector,
  void const* const key,
  size_t const num,
  size_t const size,
  int (*cmp)(void const*, void const*),
  void ** const lower,
  void ** const upper
) {

  alx_equals_range(
    key,
    vector->d,
    num,
    size,
    cmp,
    lower,
    upper
  );

}

void
alx_v32_qsort_unless_sorted(
  struct alx_v32 const* const vector,
  size_t const skip_num,
  size_t const num,
  size_t const size,
  int (*cmp)(void const*, void const*)
) {

  if (!alx_v32_is_sorted(vector, skip_num, num, size, cmp)) {
    qsort(vector->d + skip_num, num, size, cmp);
  }

}

void
alx_v32_qsort(
  struct alx_v32 const* const vector,
  size_t const skip_num,
  size_t const num,
  size_t const size,
  int (*cmp)(void const*, void const*)
) {

  qsort(vector->d + skip_num, num, size, cmp);

}

void
alx_v32_qsort_whole(
  struct alx_v32 const* const vector,
  size_t const uint32_ts_per_item,
  int (*cmp)(void const*, void const*)
) {

  alx_v32_qsort(
    vector,
    0,
    vector->size / uint32_ts_per_item,
    sizeof(*vector->d) * uint32_ts_per_item,
    cmp
  );

}

uint32_t *
alx_v32_last_nonzero_ptr(
  struct alx_v32 const* const vector
) {

  if ( !vector->size ) {
    return NULL;
  }

  uint32_t * ptr = &(vector->d[ vector->size - 1 ]);

  while ( ptr > vector->d && !*ptr ) {
    ptr--;
  }

  if (!*ptr) {
    return NULL;
  }

  return ptr;

}
