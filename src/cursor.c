#include "alx.h"

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

void
alx_shadowed_inputs_cursor_reset(
  struct alx_shadowed_inputs_cursor * const cursor,
  struct alx_v32 const* const shadowed_inputs
) {

  cursor->pos = 1;
  cursor->last_at = 0;

  while (alx_uint32_msb1(alx_v32_at(shadowed_inputs, cursor->last_at))) {
    cursor->last_at += 1;
  }

}

void
alx_shadowed_inputs_cursor_move_to(
  struct alx_shadowed_inputs_cursor * const cursor,
  struct alx_v32 const* const shadowed,
  alx_offset const to
) {

  // assumes `to` within bounds

  if (to > cursor->pos) {

    while (cursor->pos != to) {

      cursor->last_at += 1;
      cursor->pos += 1;

      do {
        cursor->last_at += 1;
      } while (
        (alx_v32_size(shadowed) > cursor->last_at)
        &&
        alx_uint32_msb1(alx_v32_at(shadowed, cursor->last_at))
      );

      cursor->last_at -= 1;
      
    }
  }

  if (to < cursor->pos) {

    while (cursor->pos != to) {

      while (alx_uint32_msb1(alx_v32_at(shadowed, cursor->last_at))) {
        cursor->last_at -= 1;
      }

      cursor->last_at -= 1; // Was -2 ???
      cursor->pos -= 1;

    }

  }

}


