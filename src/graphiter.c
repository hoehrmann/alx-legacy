#include "alx.h"

/////////////////////////////////////////////////////////////////////
// Iterator constructors
/////////////////////////////////////////////////////////////////////

struct alx_graph_quads_by_src
alx_graph_quads_by_src(
  struct alx_graph const* const graph,
  alx_graph_node_ix const src_ix
) {

  struct alx_graph_quads_by_src iterator = {
    .node_ix = src_ix, 
    .link_ix = 0
  };

  return iterator;
}

struct alx_graph_quads_by_dst
alx_graph_quads_by_dst(
  struct alx_graph const* const graph,
  alx_graph_node_ix const dst_ix
) {

  struct alx_graph_quads_by_dst iterator = {
    .node_ix = dst_ix, 
    .link_ix = 0
  };

  return iterator;
}

struct alx_graph_quads_by_any
alx_graph_quads_by_any(
  struct alx_graph const* const graph
) {

  struct alx_graph_quads_by_any iterator = {
    .node_ix = 0, 
    .link_ix = 0
  };

  return iterator;
}

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

struct alx_graph_quads_by_src
alx_graph_quads_by_src_vertexp(
  struct alx_graph const* const graph,
  alx_offset const pos,
  alx_vertex const vertex,
  alx_graph_node_ix const ref_ix
) {

  alx_graph_node_ix const node_ix = alx_graph_find_vertex(
    graph,
    pos,
    vertex,
    ref_ix
  );

#if 0
  alx_logf( graph->p, "found %u\n", node_ix );
#endif

  struct alx_graph_quads_by_src iterator = {
    .node_ix = node_ix, 
    .link_ix = 0
  };

  return iterator;
}

/////////////////////////////////////////////////////////////////////
// Iterator next
/////////////////////////////////////////////////////////////////////

int
alx_graph_quads_by_src_next(
  struct alx_graph const* const graph,
  struct alx_graph_quads_by_src * const iter 
) {

  if (iter->link_ix == 0) {

    struct alx_graph_node const node = 
      alx_graph_node_from_ix( graph, iter->node_ix );

    iter->link_ix = node.successors;
   
  } else {

    iter->link_ix = alx_v32_at(
      &graph->links,
      iter->link_ix + 1
    );

  }

#if 0

  if (iter->link_ix != 0) {

    struct alx_graph_quad const tmp = 
      alx_graph_quad_indices_from_link_ix( graph, iter->link_ix );

    struct alx_vertexp_quad const q =
      alx_graph_quad_indices_to_quad( graph, &tmp );

    alx_log_quad( graph->p, "quad ", &q );

  }

#endif

  return iter->link_ix != 0;

}

int
alx_graph_quads_by_dst_next(
  struct alx_graph const* const graph,
  struct alx_graph_quads_by_dst * const iter 
) {

  if (iter->link_ix == 0) {

    struct alx_graph_node const node = 
      alx_graph_node_from_ix( graph, iter->node_ix );

    iter->link_ix = node.predecessors;
   
  } else {

    iter->link_ix = alx_v32_at(
      &graph->links,
      iter->link_ix + 1
    );

  }

#if 0

  if (iter->link_ix != 0) {

    struct alx_graph_quad const tmp = 
      alx_graph_quad_indices_from_link_ix( graph, iter->link_ix );

    struct alx_vertexp_quad const q =
      alx_graph_quad_indices_to_quad( graph, &tmp );

    alx_log_quad( graph->p, "quad ", &q );

  }

#endif

  return iter->link_ix != 0;

}

int
alx_graph_quads_by_any_next(
  struct alx_graph const* const graph,
  struct alx_graph_quads_by_any * const iter 
) {

  assert( alx_v32_size( &graph->links ) > 0 );

  do {

    iter->link_ix = alx_v32_at(
      &graph->links, iter->link_ix + 1);

    if (iter->link_ix == 0) {
      iter->node_ix += alx_graph_node_size_in_uint32_ts();

      if (iter->node_ix >= alx_v32_size(&graph->nodes))
        return 0;

      struct alx_graph_node const tmp = 
        alx_graph_node_from_ix( graph, iter->node_ix );

      iter->link_ix = tmp.successors;
     
    }

  } while (
    iter->link_ix == 0
    &&
    alx_v32_size(&graph->nodes) > iter->node_ix
  );

#if ALX_TRACE
//  alx_debugf(stderr, "any2 iter->current %u iter->vertex %u\n", iter->current, iter->base);
#endif

  return iter->link_ix != 0;

}

/////////////////////////////////////////////////////////////////////
// Data from iterators
/////////////////////////////////////////////////////////////////////

struct alx_graph_quad
alx_graph_quads_by_src_to_quad_indices(
  struct alx_graph const* const graph,
  struct alx_graph_quads_by_src const* const iter 
) {

  return alx_graph_quad_indices_from_link_ix(
    graph,
    iter->link_ix
  );

}

struct alx_graph_quad
alx_graph_quads_by_dst_to_quad_indices(
  struct alx_graph const* const graph,
  struct alx_graph_quads_by_dst const* const iter 
) {

  return alx_graph_quad_indices_from_link_ix(
    graph,
    iter->link_ix
  );

}

struct alx_graph_quad
alx_graph_quads_by_any_to_quad_indices(
  struct alx_graph const* const graph,
  struct alx_graph_quads_by_any const* const iter 
) {

  return alx_graph_quad_indices_from_link_ix(
    graph,
    iter->link_ix
  );

}

/////////////////////////////////////////////////////////////////////
// Data from iterators
/////////////////////////////////////////////////////////////////////

struct alx_vertexp_quad
alx_graph_quads_by_src_to_quad(
  struct alx_graph const* const graph,
  struct alx_graph_quads_by_src const* const iter 
) {

  struct alx_graph_quad const tmp =
    alx_graph_quad_indices_from_link_ix(
      graph,
      iter->link_ix
    );

  return alx_graph_quad_indices_to_quad( graph, &tmp );
}

struct alx_vertexp_quad
alx_graph_quads_by_dst_to_quad(
  struct alx_graph const* const graph,
  struct alx_graph_quads_by_dst const* const iter 
) {

  struct alx_graph_quad const tmp =
    alx_graph_quad_indices_from_link_ix(
      graph,
      iter->link_ix
    );

  return alx_graph_quad_indices_to_quad( graph, &tmp );

}

struct alx_vertexp_quad
alx_graph_quads_by_any_to_quad(
  struct alx_graph const* const graph,
  struct alx_graph_quads_by_any const* const iter 
) {

  struct alx_graph_quad const tmp =
    alx_graph_quad_indices_from_link_ix(
      graph,
      iter->link_ix
    );

  return alx_graph_quad_indices_to_quad( graph, &tmp );

}
