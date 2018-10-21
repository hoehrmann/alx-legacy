#include "alx.h"

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_collapse_clear(
  struct alx_collapse * const collapse
) {

  alx_v32_clear( &collapse->vertices );
  alx_v32_clear( &collapse->lastround );
  alx_v32_clear( &collapse->position );
  alx_v32_clear( &collapse->edges );
  alx_v32_clear( &collapse->lhs_edges );
  alx_v32_clear( &collapse->rhs_edges );
  alx_v32_clear( &collapse->node_index );

  alx_v32_resize( &collapse->vertices, 1024 );
  alx_v32_resize( &collapse->lastround, 1024 );
  alx_v32_resize( &collapse->position, 1024 );
  alx_v32_resize( &collapse->edges, 1024 );
  alx_v32_resize( &collapse->lhs_edges, 1024 );
  alx_v32_resize( &collapse->rhs_edges, 1024 );
  alx_v32_resize( &collapse->node_index, 1024 );

}

/////////////////////////////////////////////////////////////////////
// vertex properties
/////////////////////////////////////////////////////////////////////

uint32_t
alx_collapse_get_position(
  struct alx_collapse * const collapse,
  alx_collapse_node_ix const node_ix
) {

  return alx_v32_at(
    &collapse->position,
    node_ix / 2
  );

}

uint32_t
alx_collapse_get_node_index(
  struct alx_collapse * const collapse,
  alx_collapse_node_ix const node_ix
) {

  return alx_v32_at(
    &collapse->node_index,
    node_ix / 2
  );

}

alx_collapse_round
alx_collapse_get_seen_as_src(
  struct alx_collapse * const collapse,
  alx_collapse_node_ix const node_ix
) {

  return alx_uint32_msb16(
    alx_v32_at( &collapse->lastround, node_ix / 2 )
  );

}

uint32_t
alx_collapse_get_lastmod(
  struct alx_collapse const* const collapse,
  alx_collapse_node_ix const node_ix
) {

  uint32_t const lastround = 
    alx_v32_at( &collapse->lastround, node_ix / 2 );

  return alx_uint32_lsb16( lastround );

}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_collapse_set_lastmod(
  struct alx_collapse * const collapse,
  alx_collapse_node_ix const node_ix,
  alx_collapse_round const round  
) {

  assert( round <= UINT16_MAX );

  alx_collapse_round const seen_as_src =
    alx_collapse_get_seen_as_src( collapse, node_ix );

  alx_v32_set_unsafe(
    &collapse->lastround,
    node_ix / 2,
    (seen_as_src << 16) | (round)
  );

}

void
alx_collapse_set_node_index(
  struct alx_collapse * const collapse,
  alx_collapse_node_ix const node_ix,
  uint32_t const value
) {

  alx_v32_set_unsafe(
    &collapse->node_index,
    node_ix / 2,
    value
  );

}

void
alx_collapse_set_position(
  struct alx_collapse * const collapse,
  alx_collapse_node_ix const node_ix,
  uint32_t const index
) {

  assert(
    alx_v32_at( &collapse->edges, index + 1 )
      == 
    node_ix
  );

  alx_v32_set_unsafe(
    &collapse->position,
    node_ix / 2,
    index
  );

}

void
alx_collapse_set_seen_as_src(
  struct alx_collapse * const collapse,
  alx_collapse_node_ix const node_ix,
  alx_collapse_round const round  
) {

  assert( round <= UINT16_MAX );

  alx_collapse_round const lastmod =
    alx_collapse_get_lastmod( collapse, node_ix );

  alx_v32_set_unsafe(
    &collapse->lastround,
    node_ix / 2,
    (round << 16) | (lastmod)
  );

}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_collapse_init(
  struct alx_collapse * const collapse,
  struct alx_v32 const* const in_edges
) {

  alx_collapse_clear( collapse );

  // TODO: these copies are not actually needed?
  // TODO: bad form to peel out the vertex_index
  alx_v32_import( &collapse->vertices, &collapse->p->bottom_up.vertex_index );
  alx_v32_import( &collapse->edges, in_edges );

  //
  // vertex properties are held in separate vectors of the same
  // size as the vertex vector except the latter uses two items

  alx_v32_zerofill(
    &collapse->lastround,
    alx_v32_size( &collapse->vertices ) / 2
  );

  alx_v32_zerofill(
    &collapse->position,
    alx_v32_size( &collapse->vertices ) / 2
  );

  alx_v32_zerofill(
    &collapse->node_index,
    alx_v32_size( &collapse->vertices ) / 2
  );

  struct alx_vertexp prev = {
    0,
    0
  };

  alx_vertex_index_ix prev_vix = 0;

  // 
  // index 

  for (size_t ix = 0; ix < alx_v32_size(&collapse->edges); ix += 2) {

    alx_vertex_index_ix const src_vix = 
      alx_v32_at( &collapse->edges, ix + 0 );

    alx_vertex_index_ix const dst_vix = 
      alx_v32_at( &collapse->edges, ix + 1 );

    if ( dst_vix != prev_vix ) {

      // this is the first with this dst_pos/dst_vertex
      // record this in the node structure

      alx_collapse_set_position(
        collapse,
        dst_vix,
        alx_size_t_to_uint32_t( ix )
      );

      prev_vix = dst_vix;

    }

  }

}

struct alx_vertexp 
alx_collapse_vertexp_from_node_ix(
  struct alx_collapse const* const collapse,
  alx_collapse_node_ix const node_ix
) {

  struct alx_vertexp const result = {
    alx_v32_at( &collapse->vertices, node_ix + 0 ),
    alx_v32_at( &collapse->vertices, node_ix + 1 ),
  };

  return result;

}

void
alx_collapse_export(
  struct alx_collapse * const collapse,
  struct alx_v32 * const dst
) {

  for (
    size_t ix = 0;
    ix < alx_v32_size( &collapse->edges );
    ix += 2
  ) {

    struct alx_collapse_node_ix_pair const edge = {
      alx_v32_at( &collapse->edges, ix + 0 ),
      alx_v32_at( &collapse->edges, ix + 1 )
    };

    struct alx_vertexp_pair const insertee = {
      alx_v32_at( &collapse->vertices, edge.src_ix + 0 ),
      alx_v32_at( &collapse->vertices, edge.src_ix + 1 ),
      alx_v32_at( &collapse->vertices, edge.dst_ix + 0 ),
      alx_v32_at( &collapse->vertices, edge.dst_ix + 1 )
    };
  
    alx_v32_push_back_pair( dst, &insertee );

  }

}
