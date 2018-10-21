#include "alx.h"

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

int
alx_cmp_uint32(
  uint32_t const lhs, 
  uint32_t const rhs
) {

  if (lhs < rhs)
    return -1;

  if (lhs > rhs)
    return +1;

  return 0;

}

int
alx_cmp_uint32_ts(
  void const* const a,
  void const* const b,
  char const* const orderby
) {

  uint32_t const* const lhs = (uint32_t const*)a;
  uint32_t const* const rhs = (uint32_t const*)b;

  for (size_t ix = 0; orderby[ix]; ix += 2) {

    int result = alx_cmp_uint32(
      lhs[ orderby[ix] - '0' ],
      rhs[ orderby[ix] - '0' ]
    );

    if (!result)
      continue;

    return orderby[ix+1] == 'a' ? result : -result;

  }

  return 0;

}

int
alx_cmp_uint32_ts_0a3a(
  void const* const a,
  void const* const b
) {

  return alx_cmp_uint32_ts(a, b, "0a3a");

}

int
alx_cmp_uint32_ts_0a(
  void const* const a,
  void const* const b
) {

  return alx_cmp_uint32_ts(a, b, "0a");

}


int
alx_cmp_uint32_ts_0d(
  void const* const a,
  void const* const b
) {

  return alx_cmp_uint32_ts(a, b, "0d");

}

int
alx_cmp_uint32_ts_2d3d0d1d(
  void const* const a,
  void const* const b
) {

  return alx_cmp_uint32_ts(a, b, "2d3d0d1d");

}

int
alx_cmp_uint32_ts_2d3d(
  void const* const a,
  void const* const b
) {

  return alx_cmp_uint32_ts(a, b, "2d3d");

}

int
alx_cmp_uint32_ts_2a3a(
  void const* const a,
  void const* const b
) {

  return alx_cmp_uint32_ts(a, b, "2a3a");

}

int
alx_cmp_uint32_ts_0a1a(
  void const* const a,
  void const* const b
) {

  return alx_cmp_uint32_ts(a, b, "0a1a");

}

int
alx_cmp_uint32_ts_0d1d(
  void const* const a,
  void const* const b
) {

  return alx_cmp_uint32_ts(a, b, "0d1d");

}

int
alx_cmp_uint32_ts_1d0d(
  void const* const a,
  void const* const b
) {

  return alx_cmp_uint32_ts(a, b, "1d0d");

}

int
alx_cmp_uint32_ts_1d0a(
  void const* const a,
  void const* const b
) {

  return alx_cmp_uint32_ts(a, b, "1d0a");

}

int
alx_cmp_uint32_ts_0a1a2a3a(
  void const* const a,
  void const* const b
) {

  return alx_cmp_uint32_ts(a, b, "0a1a2a3a");

}

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

int
alx_vertexp_quad_cmp_special(
  void const* const lhs,
  void const* const rhs
) {

  uint32_t const* const a = (uint32_t const *)lhs;
  uint32_t const* const b = (uint32_t const *)rhs;

  int32_t const depth_a = (int32_t)(a[2]);
  int32_t const depth_b = (int32_t)(b[2]);

  // TODO: special logic for 

  if (depth_a < depth_b)
    return -1;

  if (depth_a > depth_b)
    return 1;

  if (a[3] < b[3])
    return -1;

  if (a[3] > b[3])
    return 1;
    
  return 0;

}

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

int
alx_cexclusions_cmp(
  void const* const lhs,
  void const* const rhs
) {

  uint32_t const* const a = (uint32_t const *)lhs;
  uint32_t const* const b = (uint32_t const *)rhs;

  struct alx_graph const* const lhs_quads = 
    (struct alx_graph const*)alx_uint32_t_pair_to_ptr(a[2], a[3]);

  struct alx_graph const* const rhs_quads = 
    (struct alx_graph const*)alx_uint32_t_pair_to_ptr(b[2], b[3]);

  assert( lhs_quads == rhs_quads );

  // FIXME: @@@

  return 0;

#if 0

  struct alx_vertexp_pair const lhs_pair = {
    alx_quads_pos_from_base(lhs_quads, a[0]),
    alx_quads_vertex_from_base(lhs_quads, a[0]),
    alx_quads_pos_from_base(lhs_quads, a[1]),
    alx_quads_vertex_from_base(lhs_quads, a[1])
  };

  struct alx_vertexp_pair const rhs_pair = {
    alx_quads_pos_from_base(rhs_quads, b[0]),
    alx_quads_vertex_from_base(rhs_quads, b[0]),
    alx_quads_pos_from_base(rhs_quads, b[1]),
    alx_quads_vertex_from_base(rhs_quads, b[1])
  };

  if (
    (lhs_pair.dst_pos - lhs_pair.src_pos)
    <
    (rhs_pair.dst_pos - rhs_pair.src_pos)
  ) {

    return 1;

  }

  if (
    (lhs_pair.dst_pos - lhs_pair.src_pos)
    >
    (rhs_pair.dst_pos - rhs_pair.src_pos)
  ) {

    return -1;

  }

  if (lhs_pair.src_pos < rhs_pair.src_pos)
    return 1;

  if (lhs_pair.src_pos > rhs_pair.src_pos)
    return -1;

  // TODO: right order? 
  // TODO: warn if we get here?

  if (
    alx_gv_topo(
      alx_g(lhs_quads->p),
      lhs_pair.src_vertex
    )
    < 
    alx_gv_topo(
      alx_g(rhs_quads->p),
      rhs_pair.src_vertex
    )
  ) {
    return -1;
  }

  if (
    alx_gv_topo(
      alx_g(lhs_quads->p),
      lhs_pair.src_vertex
    )
    >
    alx_gv_topo(
      alx_g(rhs_quads->p),
      rhs_pair.src_vertex
    )
  ) {
    return 1;
  }

  return 0;
#endif

}

