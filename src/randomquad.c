#include "alx.h"

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

uint32_t const*
alx_random_quad_path_step_p(
  struct alx_parser_state * const p,
  struct alx_graph const* const graph,
  struct alx_v32 const* const sorted_quads,
  alx_graph_node_ix const src_ix,
  alx_graph_node_ix const dst_ix,
  struct alx_graph_quad * const found,
  bool * const is_pop
) {

  if (src_ix == 0 || dst_ix == 0)
    return NULL;

  struct alx_graph_quad const needle = {
    src_ix,
    0,
    0,
    dst_ix
  };

#if 0
  alx_logf(
    p,
    "looking for src_id = %u, dst_id = %u\n",
    needle.v1,
    needle.v4
  );
#endif

  uint32_t const* const ptr = alx_v32_bsearch_random(
    sorted_quads,
    0,
    &needle,
    alx_v32_size( sorted_quads ) / 4,
    sizeof(*sorted_quads->d) * 4,
    alx_cmp_uint32_ts_0a3a,
    &p->rng
  );

  if ( ptr == NULL ) {

    alx_logf( p, "fatal error trying to find random quad\n" );

    return NULL;

  } else {

#if 0
    alx_logf( p, "found quad %p (%zu)\n", ptr, ptr_ix );
#endif

  }

  size_t ptr_ix = alx_ptrdiff_t_to_size_t( ptr - sorted_quads->d );

  struct alx_graph_quad const tmp = {
    alx_v32_at( sorted_quads, ptr_ix + 0 ),
    alx_v32_at( sorted_quads, ptr_ix + 1 ),
    alx_v32_at( sorted_quads, ptr_ix + 2 ),
    alx_v32_at( sorted_quads, ptr_ix + 3 )
  };

  // TODO: not terribly happy with this
  memcpy( found, &tmp, sizeof(struct alx_graph_quad) );

  struct alx_graph_node const mid_src_node = 
    alx_graph_node_from_ix( graph, tmp.v2 );

  *is_pop = alx_gv_is_pop( alx_g(p), mid_src_node.vertex );

  return ptr;

}

void
alx_random_quad_path_step(
  struct alx_parser_state * const p,
  struct alx_graph const* const graph,
  struct alx_v32 const* const sorted_quads,
  alx_graph_node_ix const src_ix,
  alx_graph_node_ix const dst_ix,
  struct alx_v32 * const path
) {

  // TODO: stack as argument?
  struct alx_v32 * const stack = &p->random.quad_stack;

  alx_v32_push_back2( stack, src_ix, dst_ix );

  while ( !alx_v32_empty( stack ) ) {

    alx_graph_node_ix const popped_dst = alx_v32_pop( stack );
    alx_graph_node_ix const popped_src = alx_v32_pop( stack );

    // There are two cases to handle here, one is essentially a
    // recursive step to handle siblings and children, the other
    // is adding a vertex to the path. The latter is encoded by
    // setting the popped_src value on the stack to zero. It can
    // happen that the sentinel value zero is also pushed onto
    // the stack for popped_dst, in that case the process has 
    // reached a leaf node and the vertex is not added to path.

    if ( popped_src == 0 ) {

      if ( popped_dst != 0 ) { 
        alx_v32_push_back( path, popped_dst );
      }

      continue;

    }

    struct alx_graph_quad found;
    bool is_pop = false;

    uint32_t const* const ptr = alx_random_quad_path_step_p(
      p,
      graph,
      sorted_quads,
      popped_src,
      popped_dst,
      &found,
      &is_pop
    );

    if ( ptr == NULL ) {
      continue; // ??
    }

    if (is_pop) {

      alx_v32_push_back2( stack, found.v3, found.v4 );
      alx_v32_push_back2( stack, found.v1, found.v2 );

    } else {

      alx_v32_push_back2( stack, 0, found.v4 );
      alx_v32_push_back2( stack, found.v2, found.v3 );
      alx_v32_push_back2( stack, 0, found.v1 );

    } 

  }

}

void
alx_random_quad_path(
  struct alx_parser_state * const p,
  struct alx_graph const* const graph,
  struct alx_v32 * const tmp,
  struct alx_v32 * const path
 ) {

  alx_v32_clear( tmp );
  alx_v32_import( tmp, &graph->quads );

  alx_v32_qsort_whole(
    tmp,
    4,
    // ORDER BY src_id ASC, dst_id ASC
    alx_cmp_uint32_ts_0a3a
  );

  // At this point it would be possible to randomly eliminate 
  // duplicate (equivalent under alx_cmp_uint32_ts_0a3a) quads
  // so there would only be a single option left when searching
  // for quads later on. Iff it is possible to encounter loops 
  // in the quad graph, then that would be a good way to avoid 
  // going into any. But it is not clear that that can happen.

  alx_graph_node_ix const src_ix = alx_graph_find_vertex(
    graph,
    1,
    alx_grammar_start_vertex(alx_g(p)),
    alx_v32_size( &graph->nodes ) - 2
  );

  alx_graph_node_ix const dst_ix = alx_graph_find_vertex(
    graph,
    p->final_index,
    alx_grammar_final_vertex(alx_g(p)),
    alx_backtrace_index_final_ix( &graph->nodes )
  );

  if (dst_ix == 0 || src_ix == 0) {

    alx_logf( p, "cannot compute path, not a match\n" );
    return;

  }

  // TODO: now that the function is no longer recursive,
  // _step is a bit of a misnomer.
  alx_random_quad_path_step(
    p,
    graph,
    tmp,
    src_ix,
    dst_ix,
    path
  );

  alx_v32_clear( &p->random.stack );

}

