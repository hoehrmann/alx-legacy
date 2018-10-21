#include "alx.h"
/////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////

alx_graph_node_ix
alx_graph_find_vertex(
  struct alx_graph const* const graph,
  alx_offset const pos,
  alx_vertex const vertex,
  alx_graph_node_ix const ref_ix
);

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

size_t
alx_graph_node_size_in_uint32_ts() {
  return 2; // sizeof(struct alx_graph_node) / sizeof(uint32_t);
}

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

void
alx_graph_clear(
  struct alx_graph * const graph
) {

  alx_v32_clear(&graph->nodes);
  alx_v32_clear(&graph->links);
  alx_v32_clear(&graph->quads);
  alx_v32_clear(&graph->old2new);

  alx_v32_clear(&graph->predecessors);
  alx_v32_clear(&graph->successors);
  alx_v32_clear(&graph->lastmodified);
  alx_v32_clear(&graph->pending);

  alx_set_clear( graph->set );

}

struct alx_graph_node
alx_graph_node_from_ix(
  struct alx_graph const* const graph,
  alx_graph_node_ix const node_ix
) {

  struct alx_graph_node result = {
    .pos = alx_v32_at( &graph->nodes, node_ix + 0 ),
    .vertex = alx_v32_at( &graph->nodes, node_ix + 1 ),
    .predecessors = alx_v32_at( &graph->predecessors, node_ix / 2 ),
    .successors = alx_v32_at( &graph->successors, node_ix / 2 ),
    .lastmodified = alx_v32_at( &graph->lastmodified, node_ix / 2 ),
    .pending = alx_v32_at( &graph->pending, node_ix / 2 )
  };

  return result;

}

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

struct alx_vertexp_quad
alx_graph_quad_indices_to_quad(
  struct alx_graph const* const graph,
  struct alx_graph_quad const* const quad
) {

  struct alx_graph_node const v1 =
    alx_graph_node_from_ix( graph, quad->v1 );

  struct alx_graph_node const v2 =
    alx_graph_node_from_ix( graph, quad->v2 );

  struct alx_graph_node const v3 =
    alx_graph_node_from_ix( graph, quad->v3 );

  struct alx_graph_node const v4 =
    alx_graph_node_from_ix( graph, quad->v4 );

  struct alx_vertexp_quad const result = {
    .src_pos = v1.pos,
    .src_vertex = v1.vertex,
    .mid_src_pos = v2.pos,
    .mid_src_vertex = v2.vertex,
    .mid_dst_pos = v3.pos,
    .mid_dst_vertex = v3.vertex,
    .dst_pos = v4.pos,
    .dst_vertex = v4.vertex
  };

  return result;

}

struct alx_graph_quad
alx_graph_quad_indices_from_link_ix(
  struct alx_graph const* const graph,
  alx_graph_link_ix const link_ix
) {

  alx_graph_quad_ix const quad_ix = 
    alx_v32_at( &graph->links, link_ix + 0 );

  struct alx_graph_quad const result = {
    .v1 = alx_v32_at( &graph->quads, quad_ix + 0 ),
    .v2 = alx_v32_at( &graph->quads, quad_ix + 1 ),
    .v3 = alx_v32_at( &graph->quads, quad_ix + 2 ),
    .v4 = alx_v32_at( &graph->quads, quad_ix + 3 )
  };

  return result;

}

/////////////////////////////////////////////////////////////////////
// Node attribute setters 
/////////////////////////////////////////////////////////////////////

void
alx_graph_node_set_predecessors(
  struct alx_graph * const graph,
  alx_graph_node_ix const node_ix,
  alx_graph_link_ix const predecessors_ix
 ) {

  alx_v32_set_unsafe(
    &graph->predecessors,
    node_ix / 2,
    predecessors_ix
  );

}

void
alx_graph_node_set_successors(
  struct alx_graph * const graph,
  alx_graph_node_ix const node_ix,
  alx_graph_link_ix const successors_ix
) {

  alx_v32_set_unsafe(
    &graph->successors,
    node_ix / 2,
    successors_ix
  );

}

void
alx_graph_node_set_lastmodified(
  struct alx_graph * const graph,
  alx_graph_node_ix const node_ix,
  alx_graph_lastmod const lastmod
) {

  alx_v32_set_unsafe(
    &graph->lastmodified,
    node_ix / 2,
    lastmod
  );

}

void
alx_graph_node_set_pending(
  struct alx_graph * const graph,
  alx_graph_node_ix const node_ix,
  alx_graph_pending const pending
) {

  alx_v32_set_unsafe(
    &graph->pending,
    node_ix / 2,
    pending
  );

}

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

alx_graph_node_ix
alx_graph_find_vertex(
  struct alx_graph const* const graph,
  alx_offset const pos,
  alx_vertex const vertex,
  alx_graph_node_ix const ref_ix
) {

  struct alx_vertexp const needle = {
    .pos = pos,
    .vertex = vertex
  };

  // TODO: this code is horrible

  // TODO: Not entirely sure if the early-abort conditions are fully
  // correct. 

  for (
    alx_graph_node_ix ix = ref_ix;
    ix < alx_v32_size(&graph->nodes);
    ix += 2
  ) {

    struct alx_vertexp const candidate = {
      alx_v32_at( &graph->nodes, ix + 0 ),
      alx_v32_at( &graph->nodes, ix + 1 )
    };

    if (candidate.pos == needle.pos && candidate.vertex == needle.vertex) {

#if 0
      alx_logf(
        graph->p,
        "found %u,%u at %u (ref_ix %u)\n",
        needle.pos,
        needle.vertex,
        ix,
        ref_ix
      );
#endif

      return ix;

    }

    // In graph.nodes for ix1, ix2 
    // if ix1 < ix2 and 
    //   abs(ix1.pos - ix2.pos) <= 1 OR (ix1.pos > ix2.pos)

    // final.pos ... needle.pos + 2 out of bounds ... candidate.pos ... needle.pos - 2 out of bounds ... end
    //     if (candidate.pos > needle.pos + 1) {

    if ( candidate.pos + 1 < needle.pos ) {

#if 0
      alx_logf(
        graph->p,
        "giving up fwd pass on %u,%u at %u (ref_ix %u)\n",
        needle.pos,
        needle.vertex,
        ix,
        ref_ix
      );
#endif

      break;

    }

  }

  // bwd

  for (
    alx_graph_node_ix ix = ref_ix;
    ix > 0;
    ix -= 2
  ) {

    struct alx_vertexp const candidate = {
      alx_v32_at( &graph->nodes, ix + 0 ),
      alx_v32_at( &graph->nodes, ix + 1 )
    };

    if (candidate.pos == needle.pos && candidate.vertex == needle.vertex) {

#if 0
      alx_logf(
        graph->p,
        "found %u,%u at %u (ref_ix %u)\n",
        needle.pos,
        needle.vertex,
        ix,
        ref_ix
      );
#endif

      return ix;

    }

    if (candidate.pos > needle.pos + 1) {

#if 0
      alx_logf(
        graph->p,
        "giving up bwd pass on %u,%u at %u (ref_ix %u)\n",
        needle.pos,
        needle.vertex,
        ix,
        ref_ix
      );
#endif

      break;

    }

  }

  return 0;

}


/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

bool
alx_graph_has_pair(
  struct alx_graph* const graph,
  alx_offset const src_pos,
  alx_vertex const src_vertex,
  alx_graph_node_ix const src_ref_ix,
  alx_offset const dst_pos,
  alx_vertex const dst_vertex,
  alx_graph_node_ix const dst_ref_ix
) {

#if 1

  struct alx_graph_quad const needle = {
    alx_graph_find_vertex( graph, src_pos, src_vertex, src_ref_ix ),
    0,
    0,
    alx_graph_find_vertex( graph, dst_pos, dst_vertex, dst_ref_ix )
  };

  return alx_set_has_exteriors( graph->set, &needle );

#else
  struct alx_graph_quads_by_src by_src =
    alx_graph_quads_by_src_vertexp(graph, src_pos, src_vertex);

  while (alx_graph_quads_by_src_next(graph, &by_src)) {

    struct alx_vertexp_quad const quad =
      alx_graph_quads_by_src_to_quad(graph, &by_src);

    if (quad.src_pos != src_pos)
      continue;

    if (quad.src_vertex != src_vertex)
      continue;

    if (quad.dst_pos != dst_pos)
      continue;

    if (quad.dst_vertex != dst_vertex)
      continue;

    return true;

  }

  return false;
#endif

}

int
alx_graph_quad_indices_memcmp(
  void const* const lhs,
  void const* const rhs
) {
  return memcmp(lhs, rhs, sizeof(struct alx_graph_quad));
}

bool
alx_graph_has_quad_indices(
  struct alx_graph const* const graph,
  struct alx_graph_quad const* const needle
) {

  return alx_set_has(graph->set, needle);

#if 0

  struct alx_graph_quads_by_src by_src =
    alx_graph_quads_by_src(graph, needle->v1);

  while (alx_graph_quads_by_src_next(graph, &by_src)) {

    struct alx_graph_quad const quad =
      alx_graph_quads_by_src_to_quad_indices(graph, &by_src);

    if (alx_graph_quad_indices_memcmp(&quad, needle) == 0) {
      return true;
    }

  }

  return false;

#endif

}

bool
alx_graph_has_quad_exteriors(
  struct alx_graph const* const graph,
  struct alx_graph_quad const* const needle
) {

  return alx_set_has_exteriors(graph->set, needle);

}

void
alx_graph_insert_quad(
  struct alx_graph* const graph,
  struct alx_graph_quad const* const quad,
  bool * const changed,
  alx_graph_lastmod const lastmod
) {

  assert( quad->v1 != 0 );
  assert( quad->v4 != 0 );

  if ( alx_graph_has_quad_indices( graph, quad ) ) {
    *changed = false;
    return;
  }

  alx_set_insert( graph->set, quad );

  uint32_t const quad_ix = alx_size_t_to_uint32_t(
    alx_v32_size( &graph->quads )
  );

  alx_v32_push_back4(
    &graph->quads,
    quad->v1,
    quad->v2,
    quad->v3,
    quad->v4
  );

  alx_graph_node_ix const src_node_ix = quad->v1;

  struct alx_graph_node const src_node =
    alx_graph_node_from_ix( graph, src_node_ix );

  alx_graph_node_ix const dst_node_ix = quad->v4;

  struct alx_graph_node const dst_node =
    alx_graph_node_from_ix( graph, dst_node_ix );

#if 0
  alx_logf(
    graph->p,
    "inserting %u,%u ... %u,%u\n",
    src_node.pos,
    src_node.vertex,
    dst_node.pos,
    dst_node.vertex
  );
#endif

  alx_graph_link_ix const pred_link = 
    alx_v32_size( &graph->links ) + 0;

  alx_graph_link_ix const succ_link = 
    alx_v32_size( &graph->links ) + 2;

  alx_v32_push_back4(
    &graph->links,
    quad_ix,
    dst_node.predecessors,
    quad_ix,
    src_node.successors
  );

  alx_graph_node_set_successors( graph, src_node_ix, succ_link );
  alx_graph_node_set_predecessors( graph, dst_node_ix, pred_link );

  alx_graph_node_set_lastmodified( graph, src_node_ix, lastmod );
  alx_graph_node_set_lastmodified( graph, dst_node_ix, lastmod );

  *changed = true;

}

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

void
alx_graph_push_back_node(
  struct alx_graph * const graph,
  struct alx_graph_node const* const node
) {

  alx_v32_push_back(
    &graph->nodes,
    node->pos
  );

  alx_v32_push_back(
    &graph->nodes,
    node->vertex
  );

  alx_v32_push_back(
    &graph->predecessors,
    node->predecessors
  );

  alx_v32_push_back(
    &graph->successors,
    node->successors
  );

  alx_v32_push_back(
    &graph->lastmodified,
    node->lastmodified
  );

  alx_v32_push_back(
    &graph->pending,
    node->pending
  );

}

void
alx_graph_init_from_edges(
  struct alx_graph * const graph,
  struct alx_v32 const* const index,
  struct alx_v32 const* const edges
) {

  alx_graph_clear( graph );

  // link_ix zero is used as linked list end marker, reserve it here.
  alx_v32_push_back2(
    &graph->links,
    0,
    0
  );

  alx_logf( graph->p, "copying vertices\n" );

  alx_v32_zerofill(
    &graph->old2new,
    alx_v32_size( index )
  );

  for (
    alx_vertex_index_ix ix = 0;
    ix < alx_v32_size( index );
    ix += 2
  ) {

    struct alx_vertexp const vertex = {
      alx_v32_at( index, ix + 0 ),
      alx_v32_at( index, ix + 1 )
    };

    if (alx_gv_is_skippable(alx_g(graph->p), vertex.vertex)) {
      continue;
    }

    struct alx_graph_node const insertee = {
      .pos = vertex.pos,
      .vertex = vertex.vertex,
      .predecessors = 0,
      .successors = 0,
      .lastmodified = 1,
      .pending = 1
    };

    alx_v32_set_unsafe(
      &graph->old2new,
      ix,
      alx_size_t_to_uint32_t( alx_v32_size( &graph->nodes ) )
    );

    alx_graph_push_back_node( graph, &insertee );

  }

  for (size_t ix = 0; ix < alx_v32_size(edges); ix += 2) {

    struct alx_vertex_index_ix_pair const old = {
      alx_v32_at( edges, ix + 0 ),
      alx_v32_at( edges, ix + 1 )
    };

    // TODO: vertexp_pair from vix_pair?
    struct alx_vertexp_pair const dbg = {
      alx_v32_at( index, old.src_ix + 0 ),
      alx_v32_at( index, old.src_ix + 1 ),
      alx_v32_at( index, old.dst_ix + 0 ),
      alx_v32_at( index, old.dst_ix + 1 )
    };

    assert(
      !alx_gv_is_skippable(
        alx_g(graph->p), dbg.src_vertex )
    );

    assert(
      !alx_gv_is_skippable(
        alx_g(graph->p), dbg.dst_vertex )
    );

    alx_graph_node_ix const src_ix = 
      alx_v32_at( &graph->old2new, old.src_ix );

    alx_graph_node_ix const dst_ix = 
      alx_v32_at( &graph->old2new, old.dst_ix );

    assert( src_ix != 0 && dst_ix != 0 );

    struct alx_graph_quad const insertee = {
      src_ix,
      0,
      0,
      dst_ix
    };

#if 1
    struct alx_graph_node const src_node = 
      alx_graph_node_from_ix( graph, src_ix );

    struct alx_graph_node const dst_node = 
      alx_graph_node_from_ix( graph, dst_ix );

    struct alx_vertexp_pair const middle = {
      src_node.pos,
      src_node.vertex,
      dst_node.pos,
      dst_node.vertex
    };

    // skip bad edge
    if (alx_vertexp_pair_is_unbalanced_pushpop(alx_g(graph->p), &middle)) {
      continue;
    }
#endif

    bool changed = false;
    alx_graph_insert_quad( graph, &insertee, &changed, 00000000 );

  }

  alx_logf( graph->p, "Done graph init\n" );

}

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

void
alx_graph_to_siblings_and_exclusions(
  struct alx_graph * const graph,
  struct alx_v32 * const siblings
) {

  // TODO: pass out-exclusions-list as parameter

  struct alx_graph_quads_by_any by_any =
    alx_graph_quads_by_any( graph );

  int32_t ancestors_estimate = 0;

  while (alx_graph_quads_by_any_next( graph, &by_any )) {

    struct alx_graph_quad const any_q =
      alx_graph_quads_by_any_to_quad_indices( graph, &by_any );

    assert( any_q.v2 == 0 );
    assert( any_q.v3 == 0 );

    struct alx_vertexp_quad const quad = 
      alx_graph_quad_indices_to_quad( graph, &any_q );

    struct alx_vertexp_pair const pair = {
      quad.src_pos,
      quad.src_vertex,
      quad.dst_pos,
      quad.dst_vertex
    };

    if ( alx_gv_is_skippable(alx_g(graph->p), quad.src_vertex) )
      continue;

    if ( alx_gv_is_skippable(alx_g(graph->p), quad.dst_vertex) )
      continue;

    ///////////////////////////////////////////////////////////////
    //
    // TODO: make function for this
    if (alx_gv_is_exclusion_if1(alx_g(graph->p), quad.src_vertex)) {

#if 0
      alx_logf( graph->p, "ADDING EXCLUSION\n" );
#endif

      uint32_t first;
      uint32_t second;
      alx_ptr_to_uint32_t_pair(graph, &first, &second);

      alx_v32_push_back4(
        &graph->p->bottom_up.cexclusions,
        any_q.v1,
        any_q.v4,
        first,
        second
      );

    }
    //
    ///////////////////////////////////////////////////////////////

    // Optimisation for relatively unambiguous parse graphs.
    if (alx_gv_is_push(alx_g(graph->p), pair.src_vertex)) {

      ancestors_estimate++;

    } else if (alx_gv_is_pop(alx_g(graph->p), pair.src_vertex)) {

      ancestors_estimate--;

    }

    if ( !alx_vertexp_pair_is_poppush( alx_g(graph->p), &pair ) )
      continue;

#if 0
    alx_logf( graph->p, "csibling %u %u %u %u\n",
      quad.src_pos,
      quad.src_vertex,
      quad.dst_pos,
      quad.dst_vertex
    );
#endif

    alx_v32_push_back4(siblings,
      any_q.v1,
      any_q.v4,
      ancestors_estimate,
      pair.src_pos
    );

  }

}

void
alx_graph_to_edges(
  struct alx_graph * const graph,
  struct alx_v32 * const edges,
  bool const only_pending
) {

  // unused. makes me think, though: if this filters out edges
  // with vertices that are not pending, and this function is 
  // not used anywhere, if filtering out !pending vertices missing
  // elsewhere?

  // TODO: clear edges?

  // TODO: by_any_dst to force sort order?

  struct alx_graph_quads_by_any by_any =
    alx_graph_quads_by_any( graph );

  while (alx_graph_quads_by_any_next( graph, &by_any )) {

    struct alx_graph_quad const any_q =
      alx_graph_quads_by_any_to_quad_indices( graph, &by_any );

    assert( any_q.v2 == 0 );
    assert( any_q.v3 == 0 );

    struct alx_vertexp_quad const quad = 
      alx_graph_quad_indices_to_quad( graph, &any_q );

    struct alx_vertexp_pair const pair = {
      quad.src_pos,
      quad.src_vertex,
      quad.dst_pos,
      quad.dst_vertex
    };

    if (only_pending) {

      struct alx_graph_node const src_node = 
        alx_graph_node_from_ix( graph, any_q.v1 );

      struct alx_graph_node const dst_node = 
        alx_graph_node_from_ix( graph, any_q.v4 );

      if (!src_node.pending || !dst_node.pending)
        continue;

#if 0
      if ( alx_gv_is_skippable(alx_g(graph->p), quad.src_vertex) )
        continue;

      if ( alx_gv_is_skippable(alx_g(graph->p), quad.dst_vertex) )
        continue;
#endif

    }

    alx_v32_push_back_pair( edges, &pair );

  }

}

