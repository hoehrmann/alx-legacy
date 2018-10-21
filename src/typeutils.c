#include "alx.h"

/////////////////////////////////////////////////////////////////////
// Utilities for uint32_t
/////////////////////////////////////////////////////////////////////

uint32_t 
alx_htobe32(uint32_t const value) {

  // FIXME(bh): must be no-op when host is big-endian
  // FIXME(bh): also bad to rely on platform-specific functions

#ifdef _MSC_VER
  return _byteswap_ulong(value);
#else
  return __builtin_bswap32(value);
#endif
			
}

uint32_t
alx_uint32_lsb31(uint32_t const value) {

  return value & UINT32_C( 0x7FFFFFFF );

}

uint32_t
alx_uint32_lsb16(uint32_t const value) {

  return value & UINT32_C( 0x0000FFFF );

}

uint32_t
alx_uint32_msb16(uint32_t const value) {

  return value >> 16;

}

uint32_t
alx_uint32_msb1(uint32_t const value) {

  return value >> 31;

}

/////////////////////////////////////////////////////////////////////
// min/max utilities
/////////////////////////////////////////////////////////////////////

size_t
alx_max_size_t(size_t const lhs, size_t const rhs) {

  return lhs < rhs ? rhs : lhs;

}

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

bool
alx_offset_between_inclusive(
  alx_offset const pos,
  alx_offset const min_inclusive,
  alx_offset const max_inclusive
) {

  return pos <= max_inclusive && pos >= min_inclusive;

}

/////////////////////////////////////////////////////////////////////
// Casts
/////////////////////////////////////////////////////////////////////

uint8_t
alx_char_to_uint8_t(char const value) {

  return (uint8_t)value;

}

uint8_t
alx_int_to_uint8_t(int const value) {

  return (uint8_t)value;

}

uint32_t
alx_size_t_to_uint32_t(size_t const value) {

  assert( value <= UINT32_MAX );

  return (uint32_t)value;

}

int64_t
alx_size_t_to_int64_t(size_t const value) {

  assert( value <= INT64_MAX );

  return (uint32_t)value;

}

int
alx_size_t_to_int(size_t const value) {

  assert( value <= INT_MAX );

  return (int)value;

}

int64_t
alx_uint32_t_to_int64_t(size_t const value) {

  return value;

}

uint32_t
alx_ptrdiff_t_to_int(ptrdiff_t const value) {

  return (int)value;

}

uint32_t
alx_ptrdiff_t_to_size_t(ptrdiff_t const value) {

  return (uint32_t)value;

}

void
alx_ptr_to_uint32_t_pair(
  void const* const ptr,
  uint32_t * const first,
  uint32_t * const second
) {

  static_assert(
    sizeof(uintptr_t) <= 8,
    "pointers must be at most 64 bits"
  );

  uintptr_t const both = (uintptr_t)ptr;
  *first = both & UINT32_C( 0xFFFFFFFF );

  if (sizeof(uintptr_t) > 4) {
    *second = both >> UINT32_C( 32 );
  } else {
    *second = 0;
  }

}

void*
alx_uint32_t_pair_to_ptr(
  uint32_t const first,
  uint32_t const second
) {

  uintptr_t const fst = first;
  uintptr_t const snd = second;
  uintptr_t const both = sizeof(uintptr_t) > 4
    ? (snd << 32) | fst
    : fst;

  return ((void*)both);

}

char*
alx_void_ptr_to_char_ptr(
  void const* const void_ptr
) {

  return ((char*)void_ptr);

}

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

alx_input_class_id
alx_utf8_state_to_input_class_id(alx_utf8_state const state) {

  return (alx_input_class_id)state;

}

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

void
alx_validate_types(void) {

  static_assert(
    sizeof(struct alx_vertexp) == 8,
    "expected struct alx_vertexp size"
  );

  static_assert(
    sizeof(struct alx_vertexp_pair) == 16,
    "expected struct alx_vertexp_pair size"
  );

  static_assert(
    sizeof(struct alx_vertexp_quad) == 32,
    "expected struct alx_vertexp_quad size"
  );

}

