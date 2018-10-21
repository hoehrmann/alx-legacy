#include "alx.h"

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

bool
alx_graph_omit_quad(
  struct alx_graph const* const graph,
  struct alx_graph_quad const* const indices,
  struct alx_vertexp_quad const* const quad
) {

  if (!alx_gv_is_if2(alx_g(graph->p), quad->mid_src_vertex))
    return false;

  assert(
    alx_gv_is_if(
      alx_g(graph->p),
      quad->src_vertex
    )
  );

  assert(
    alx_gv_is_fi2(
      alx_g(graph->p),
      quad->mid_dst_vertex
    )
  );

  assert(
    alx_gv_is_fi(
      alx_g(graph->p),
      quad->dst_vertex
    )
  );

  assert(
    alx_gv_partner(
      alx_g(graph->p),
      quad->src_vertex
    ) == quad->dst_vertex
  );

  assert(
    alx_gv_partner(
      alx_g(graph->p),
      quad->mid_src_vertex
    ) == quad->mid_dst_vertex
  );

  if (!alx_gv_is_ordered_choice(
    alx_g(graph->p),
    quad->mid_src_vertex
  )) {
    return false;
  }

  alx_vertex const if1_vertex = 
    alx_gv_p1( alx_g(graph->p), quad->src_vertex );

  alx_vertex const fi1_vertex = 
    alx_gv_partner( alx_g(graph->p), if1_vertex );

  alx_graph_node_ix const if1_ix =
    alx_graph_find_vertex(
      graph,
      quad->mid_src_pos,
      if1_vertex,
      indices->v2
    );

  alx_graph_node_ix const fi1_ix =
    alx_graph_find_vertex(
      graph,
      quad->mid_dst_pos,
      fi1_vertex,
      indices->v3
    );

  if (!if1_ix || !fi1_ix) {
    return false;
  }

  struct alx_graph_quad const needle = {
    indices->v1, // if_ix
    if1_ix,
    fi1_ix,
    indices->v4 // fi_ix
  };

  if (alx_graph_has_quad_indices(graph, &needle)) {
//    alx_logf( graph->p, "omitting something\n" );
    return true;
  }

  return false;

}

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

void
alx_graph_remove_quad_p(
  struct alx_graph * const graph,
  alx_graph_node_ix const node_ix,
  alx_graph_quad_ix const removee_ix,
  size_t const foo_ix
) {

  size_t preceding_ix = 0;
  size_t link_ix = 0;

  for (
    link_ix = foo_ix;
    link_ix
    &&
    alx_v32_at( &graph->links, link_ix + 0 ) != removee_ix;
    link_ix = alx_v32_at( &graph->links, link_ix + 1 )
  ) {

    preceding_ix = link_ix;

  }

  if ( link_ix == 0 ) {

    alx_logf( graph->p, "unable to find removee\n" );

  } else {

    alx_v32_set_unsafe(
      &graph->links,
      preceding_ix + 1,
      alx_v32_at( &graph->links, link_ix + 1 )
    );

  }

}

void
alx_graph_remove_quad(
  struct alx_graph * const graph,
  alx_graph_quad_ix const removee_ix
) {

  struct alx_graph_quad const removee = {
    alx_v32_at( &graph->quads, removee_ix + 0 ),
    alx_v32_at( &graph->quads, removee_ix + 1 ),
    alx_v32_at( &graph->quads, removee_ix + 2 ),
    alx_v32_at( &graph->quads, removee_ix + 3 )
  };

  alx_logf( graph->p, "removing quad\n" );

  struct alx_graph_node const src_node = 
    alx_graph_node_from_ix( graph, removee.v1 );
  
  struct alx_graph_node const dst_node = 
    alx_graph_node_from_ix( graph, removee.v4 );

#if 0
  alx_log_graph_node( graph->p, "src_node ", &src_node );
  alx_log_graph_node( graph->p, "dst_node ", &dst_node );
#endif

  if (alx_v32_at( &graph->links, src_node.successors + 0 ) == removee_ix) {

    alx_graph_node_set_successors(
      graph,
      removee_ix,
      alx_v32_at( &graph->links, src_node.successors + 1 )
    );

  } else {

    alx_graph_remove_quad_p(
      graph,
      removee_ix, // removee.v1
      removee_ix,
      src_node.successors
    );
  
  }

  if (alx_v32_at( &graph->links, dst_node.predecessors + 0 ) == removee_ix) {

    alx_graph_node_set_successors(
      graph,
      removee_ix,
      alx_v32_at( &graph->links, dst_node.predecessors + 1 )
    );

  } else {

    alx_graph_remove_quad_p(
      graph,
      removee_ix, // removee.v4
      removee_ix,
      dst_node.predecessors
    );
  
  }

  alx_v32_set_unsafe( &graph->quads, removee_ix + 0, 0 );
  alx_v32_set_unsafe( &graph->quads, removee_ix + 0, 1 );
  alx_v32_set_unsafe( &graph->quads, removee_ix + 0, 2 );
  alx_v32_set_unsafe( &graph->quads, removee_ix + 0, 3 );

}

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

void
alx_graph_cleanup_ordered_choices(
  struct alx_graph * const graph
) {

  for (alx_graph_quad_ix ix = 0; ix < alx_v32_size(&graph->quads); ix += 4) {

    struct alx_graph_quad const candidate = {
      alx_v32_at( &graph->quads, ix + 0 ),
      alx_v32_at( &graph->quads, ix + 1 ),
      alx_v32_at( &graph->quads, ix + 2 ),
      alx_v32_at( &graph->quads, ix + 3 )
    };

    struct alx_vertexp_quad const candidate_p =
      alx_graph_quad_indices_to_quad( graph, &candidate );

    if (!alx_graph_omit_quad( graph, &candidate, &candidate_p )) {
      continue;
    }

#if 0
    alx_log_quad( graph->p, "removing quad ", &candidate_p );
#endif

    alx_graph_remove_quad( graph, ix );

  }

}

