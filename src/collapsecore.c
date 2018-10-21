#include "alx.h"

bool
alx_collapse_is_removable(
  struct alx_collapse const* const collapse,
  alx_collapse_node_ix const src_ix,
  alx_collapse_node_ix const dst_ix,
  alx_collapse_round const round,
  bool * const blocked
) {

  struct alx_vertexp const src = 
    alx_collapse_vertexp_from_node_ix( collapse, src_ix );

  struct alx_vertexp const dst = 
    alx_collapse_vertexp_from_node_ix( collapse, dst_ix );

  bool src_skippable = alx_gv_is_skippable(
    alx_g( collapse->p ),
    src.vertex
  );

  uint32_t const src_lastmod = 
    alx_collapse_get_lastmod( collapse, src_ix );

  if (!src_skippable)
    return false;

  if (src_lastmod >= round) {
    *blocked = true;
    return false;
  }

  return true;

}

void
alx_collapse_single(
  struct alx_collapse * const collapse,
  alx_collapse_node_ix const src_ix,
  alx_collapse_node_ix const dst_ix,
  alx_collapse_round const round
) {

  struct alx_vertexp_pair const victim = {
    alx_v32_at( &collapse->vertices, src_ix + 0 ),
    alx_v32_at( &collapse->vertices, src_ix + 1 ),
    alx_v32_at( &collapse->vertices, dst_ix + 0 ),
    alx_v32_at( &collapse->vertices, dst_ix + 1 )
  };

#if 0
  alx_log_pair( collapse->p, "removing ", &victim );
#endif

  for (
    uint32_t ix = alx_collapse_get_position( collapse, src_ix );
    ix < alx_v32_size( &collapse->edges );
    ix += 2
  ) {

    struct alx_collapse_node_ix_pair other = {
      alx_v32_at( &collapse->edges, ix + 0 ),
      alx_v32_at( &collapse->edges, ix + 1 )
    };

    // collapse->edges is ordered by dst_ix ascendingly and there
    // is an index on the first occurence of each node_ix as dst_ix
    // in the array, which is queried above. Once we encounter a
    // different dst_ix than what we are looking for, all relevant
    // edges have been processed and we can leave this loop.
    if ( other.dst_ix != src_ix ) {
      break;
    }

    struct alx_vertexp_pair const insertee = {
      alx_v32_at( &collapse->vertices, other.src_ix + 0 ),
      alx_v32_at( &collapse->vertices, other.src_ix + 1 ),
      alx_v32_at( &collapse->vertices, dst_ix + 0 ),
      alx_v32_at( &collapse->vertices, dst_ix + 1 )
    };

#if 0
  alx_log_pair( collapse->p, "adding ", &insertee );
#endif

    // record new edge in rhs_edges
    alx_v32_push_back( &collapse->rhs_edges, other.src_ix );
    alx_v32_push_back( &collapse->rhs_edges, dst_ix );

    // Since edge.src_ix might now have an additional successor,
    // the logic to find all predecessors of a vertex cannot be
    // applied so easily anymore. Record when this happened, and
    // filter affected edges throughout the rest of this round.
    alx_collapse_set_lastmod( collapse, other.src_ix, round );

  }

}

/////////////////////////////////////////////////////////////////////
// merging
/////////////////////////////////////////////////////////////////////

ALX_INLINE
void
alx_collapse_merge_p(
  struct alx_collapse * const collapse,
  struct alx_v32 const* const edges,
  size_t * const ix_ptr,
  alx_collapse_node_ix const dst_ix
) {

  while ( *ix_ptr < alx_v32_size(edges) ) {

    struct alx_collapse_node_ix_pair candidate = {
      alx_v32_at( edges, *ix_ptr + 0 ),
      alx_v32_at( edges, *ix_ptr + 1 )
    };

    if (candidate.dst_ix != dst_ix)
      break;
    
    *ix_ptr += 2;

    // skip duplicates
    if ( alx_collapse_get_node_index( collapse, candidate.src_ix ) )
      continue;
    
    alx_v32_push_back( &collapse->edges, candidate.src_ix );
    alx_v32_push_back( &collapse->edges, candidate.dst_ix );
    alx_collapse_set_node_index( collapse, candidate.src_ix, 1 );

  }

}

void
alx_collapse_merge(
  struct alx_collapse * const collapse,
  uint32_t const round,
  size_t * const new_edge_count
) {

  size_t lhs_ix = 0;
  size_t rhs_ix = 0;

  while (true) {

    // node slot zero is reserved during initialisation as sentinel.
    alx_collapse_node_ix current_dst = 0;

    if (lhs_ix + 1 < alx_v32_size( &collapse->lhs_edges )) {

      current_dst = alx_v32_at( &collapse->lhs_edges, lhs_ix + 1 );

    }

    if (rhs_ix + 1 < alx_v32_size( &collapse->rhs_edges )) {

      alx_collapse_node_ix const rhs_dst = 
        alx_v32_at( &collapse->rhs_edges, rhs_ix + 1 );

      if (rhs_dst > current_dst) {
        current_dst = rhs_dst;
      }

    }

    if (!current_dst) {
      break;
    }


    alx_collapse_merge_p(
      collapse,
      &collapse->lhs_edges,
      &lhs_ix,
      current_dst
    );

    size_t size_before = alx_v32_size( &collapse->edges );

    alx_collapse_merge_p(
      collapse,
      &collapse->rhs_edges,
      &rhs_ix,
      current_dst
    );

#if 0
    alx_logf(
      collapse->p,
      "size before %zu after %zu\n",
      size_before,
      alx_v32_size( &collapse->edges )
    );
#endif

    *new_edge_count += 
      (alx_v32_size( &collapse->edges ) - size_before) / 2;

    //
    // now walk backwards through merged edges to reset node_index,
    // update position data, and mark vertices with predecessors.
    for (
      size_t ix = alx_v32_size( &collapse->edges ) - 2;
      true;
      ix -= 2
    ) {

      struct alx_collapse_node_ix_pair const tmp = {
        alx_v32_at( &collapse->edges, alx_size_t_to_uint32_t( ix + 0 ) ),
        alx_v32_at( &collapse->edges, alx_size_t_to_uint32_t( ix + 1 ) )
      };

      if (tmp.dst_ix != current_dst)
        break;

      alx_collapse_set_seen_as_src( collapse, tmp.src_ix, round );
      alx_collapse_set_position( collapse, tmp.dst_ix, alx_size_t_to_uint32_t( ix ) );
      alx_collapse_set_node_index( collapse, tmp.src_ix, 0 );

      if (ix == 0)
        break;

    }

  }

  //
  // pretend the final vertex has successors
  alx_collapse_node_ix final_ix = alx_backtrace_index_final_ix(
    &collapse->vertices
  );

  alx_collapse_set_seen_as_src( collapse, final_ix, round );

}

/////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////

void
alx_collapse_loop(
  struct alx_collapse * const collapse
) {

  for (alx_collapse_round round = 1; round < UINT16_MAX; ++round) {

    // TODO: need prove this is always true for < 2GB inputs
    assert( round < UINT16_MAX - 1 );

    alx_logf(
      collapse->p,
      "collapse round %u with now %zu edges\n",
      round,
      alx_v32_size( &collapse->edges ) / 2
    );

    size_t skipped_edge_count = 0;

    for (
      size_t ix = 0;
      ix < alx_v32_size( &collapse->edges );
      ix += 2
    ) {

      struct alx_collapse_node_ix_pair const edge = {
        alx_v32_at( &collapse->edges, ix + 0 ),
        alx_v32_at( &collapse->edges, ix + 1 )
      };

      if (alx_collapse_get_seen_as_src(collapse, edge.dst_ix) < round - 1) {

#if 0
        struct alx_vertexp_pair const tmp = {
          alx_v32_at( &collapse->vertices, edge.src_ix + 0 ),
          alx_v32_at( &collapse->vertices, edge.src_ix + 1 ),
          alx_v32_at( &collapse->vertices, edge.dst_ix + 0 ),
          alx_v32_at( &collapse->vertices, edge.dst_ix + 1 )
        };

        alx_log_pair( collapse->p, "unused ", &tmp );
        alx_logf( collapse->p, "(dst_ix seen as src round %u but now %u)\n",
          alx_collapse_get_seen_as_src(collapse, edge.dst_ix),
          round
        );
#endif

        continue;
        
      }

      bool blocked = false;
      bool removable = alx_collapse_is_removable(
        collapse,
        edge.src_ix,
        edge.dst_ix,
        round,
        &blocked
      );

      if (blocked) {
        skipped_edge_count += 1;
      }

      if (!removable) {

        alx_v32_push_back( &collapse->lhs_edges, edge.src_ix );
        alx_v32_push_back( &collapse->lhs_edges, edge.dst_ix );

        continue;

      }

      alx_collapse_single(
        collapse,
        edge.src_ix,
        edge.dst_ix,
        round
      );

    }

    // The least significant digit of a round number indicates
    // whether the round number was set above or during merge.
    round += 1;

    alx_v32_clear( &collapse->edges );

    size_t new_edge_count = false;
    alx_collapse_merge( collapse, round, &new_edge_count );

    alx_v32_clear( &collapse->lhs_edges );
    alx_v32_clear( &collapse->rhs_edges );

#if 0
    alx_logf( collapse->p, "found_new_edges = %u\n", found_new_edges ) ;
    alx_logf( collapse->p, "has_pending_edges = %u\n", has_pending_edges ) ;
#endif

    alx_logf(
      collapse->p,
      "finished collapse round with %zu new edges and %zu skipped\n",
      new_edge_count,
      skipped_edge_count
    );

    if (!(new_edge_count || skipped_edge_count)) {
      break;
    }

  }

}

void
alx_collapse_collapse(
  struct alx_collapse * const collapse,
  struct alx_v32 * const edges_in,
  struct alx_v32 * const edges_out
) {

  alx_logf( collapse->p, "collapse start\n" );

#if 1

  // TODO: unclear when this is really needed.

  // TODO(bh): A better sorting function could be a considerable
  // win here since edges_in should already be sorted by dst_pos.
  alx_logf( collapse->p, "pre-collapse sort\n" );
  alx_v32_qsort(
    edges_in,
    0,
    alx_v32_size( edges_in ) / 2,
    sizeof(uint32_t) * 2,
    alx_cmp_uint32_ts_1d0d
  );
  alx_logf( collapse->p, "done pre-collapse sort\n" );

#endif

  alx_collapse_init(
    collapse,
    edges_in
  );

  alx_collapse_loop(
    collapse
  );

  alx_v32_clear( edges_out );

  alx_v32_import( edges_out, &collapse->edges );

  alx_logf( collapse->p, "collapse done\n" );

  alx_collapse_clear( collapse );

}

