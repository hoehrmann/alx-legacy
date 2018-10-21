
/////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////

#include "alx.h"

/////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////

bool 
alx_handle_if(
  struct alx_parser_state * const p,
  struct alx_graph * const q,
  struct alx_graph_quad const* const candidate_indices,
  bool const assume_exclusion_failed
) {

  struct alx_vertexp_quad const candidate = 
    alx_graph_quad_indices_to_quad(q, candidate_indices);

#if 0
    alx_log_pair( p, "handle_if candidate ", &candidate );
#endif

  struct alx_if_group const cond = 
    alx_gv_if_group( alx_g(p), candidate.src_vertex );

  char const* const name = alx_gv_name(alx_g(p), cond.if_vertex);

  bool const has_if1fi = (
    (candidate.mid_src_vertex == cond.if1_vertex)
    ||
    alx_graph_has_pair(
      q,
      candidate.mid_src_pos,
      cond.if1_vertex,
      candidate_indices->v2,
      candidate.mid_dst_pos,
      cond.fi1_vertex,
      candidate_indices->v3
    )
  );

  bool const has_if2fi = (
    (candidate.mid_src_vertex == cond.if2_vertex)
    ||
    alx_graph_has_pair(
      q,
      candidate.mid_src_pos,
      cond.if2_vertex,
      candidate_indices->v2,
      candidate.mid_dst_pos,
      cond.fi2_vertex,
      candidate_indices->v3
    )
  );

#if 0
  alx_logf(
    p,
    "%s has_if1fi %u has_if2fi %u assume %u\n",
    alx_gv_name(alx_g(p), cond.if_vertex),
    has_if1fi,
    has_if2fi,
    assume_exclusion_failed
  );
#endif

  if (alx_gv_is_exclusion(alx_g(p), cond.if_vertex)) {

    return (
      assume_exclusion_failed
      &&
      has_if1fi
      && 
      !has_if2fi
    );

  } else if (alx_gv_is_ordered_choice(alx_g(p), cond.if_vertex)) {

    // Ordering is respected at a later point so this test suffices

    return (
      has_if1fi
      ||
      has_if2fi
    );

  } else if (alx_gv_is_conjunction(alx_g(p), cond.if_vertex)) {

    return (
      has_if1fi
      &&
      has_if2fi
    );

  } else if (alx_gv_is_ordered_conjunction(alx_g(p), cond.if_vertex)) {

    return (
      candidate.mid_src_vertex == cond.if1_vertex
      &&
      has_if1fi
      &&
      has_if2fi
    );

  }

  return false;

}

/////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////

bool 
alx_handle_triple(
  struct alx_parser_state * const p,
  struct alx_graph * const q,
  struct alx_graph_quad const* const candidate_indices,
  bool const assume_exclusion_failed
) {

  struct alx_vertexp_quad const candidate = 
    alx_graph_quad_indices_to_quad(q, candidate_indices);

#if 0
    alx_log_quad( p, "handle_triple called for ", &candidate );
#endif


  if (alx_vertexp_quad_is_parent_child(alx_g(p), &candidate)) {

#if 0
    alx_log_quad( p, "CONSIDERING PC ", &candidate );
#endif

    if (alx_gv_is_if( alx_g(p), candidate.src_vertex )) {

      return alx_handle_if(p, q, candidate_indices, assume_exclusion_failed);

    } else {

      return true;

    }

  } else if (alx_vertexp_quad_is_siblings(alx_g(p), &candidate)) {

#if 0
    alx_log_quad( p, "CONSIDERING <> ", &candidate );
#endif

    // Enforce left-factoring

    struct alx_graph_quads_by_dst iter = alx_graph_quads_by_dst(
      q,
      candidate_indices->v1
    );

    while (alx_graph_quads_by_dst_next(q, &iter)) {

      struct alx_vertexp_quad quad =
        alx_graph_quads_by_dst_to_quad(q, &iter);

      if (alx_gv_is_push( alx_g(p), quad.src_vertex ))
        return true;

    }

  } else {

#if 0
    alx_log_quad( p, "BAD ", &candidate );
#endif

  }

  return false;
}

void
alx_handle_new_exteriors(
  struct alx_parser_state * const p,
  struct alx_graph * const q,
  bool * const changed_at_all,
  uint32_t const round,
  struct alx_vertexp_quad const* const tmp,
  struct alx_graph_quad const* const tmp_indices,
  struct alx_v32 * const cexclusions
) {

  if (alx_gv_is_exclusion_if1(alx_g(p), tmp->src_vertex)) {

#if 0
    alx_logf( p, "ADDING EXCLUSION\n" );
#endif

    uint32_t first;
    uint32_t second;
    alx_ptr_to_uint32_t_pair(q, &first, &second);

    // FIXME: cexclusions
    alx_v32_push_back4(
      cexclusions,
      tmp_indices->v1,
      tmp_indices->v4,
      first,
      second
    );

  }

#if 0
  alx_logf( p, "recursing\n" );
#endif

  alx_handle_single(
    p,
    q,
    tmp_indices->v1, // left.src
    tmp_indices->v4, // right.dst
    changed_at_all,
    round,
    false,
    cexclusions
  );

#if 0
  alx_logf( p, "done recursing\n" );
#endif

}

void
alx_handle_single(
  struct alx_parser_state * const p,
  struct alx_graph * const q,
  uint32_t const src_ix,
  uint32_t const dst_ix,
  bool * const changed_at_all,
  uint32_t const round,
  bool const assume_exclusion_failed,
  struct alx_v32 * const cexclusions
) {

  struct alx_graph_quads_by_dst lhs_iter =
    alx_graph_quads_by_dst(q, src_ix);

#if 0

  struct alx_graph_node const tmp_src = 
    alx_graph_node_from_ix( q, src_ix );

  struct alx_graph_node const tmp_dst = 
    alx_graph_node_from_ix( q, dst_ix );

  alx_log_graph_node( p, "src_node ", &tmp_src );
  alx_log_graph_node( p, "dst_node ", &tmp_dst );

#endif

  while (alx_graph_quads_by_dst_next(q, &lhs_iter)) {

    struct alx_vertexp_quad const lhs_quad =
      alx_graph_quads_by_dst_to_quad(q, &lhs_iter); 

#if 0
    alx_logf(p, "lhs_iter %u %u\n", lhs_iter.node_ix, lhs_iter.link_ix);

    alx_log_quad( p, "lhs_quad ", &lhs_quad );
#endif

#if 0
    alx_logf(p, "found a alx_graph_quads_by_dst_next\n");
#endif

    struct alx_graph_quads_by_src rhs_iter =
      alx_graph_quads_by_src(q, dst_ix);

    while (alx_graph_quads_by_src_next(q, &rhs_iter)) {

      struct alx_graph_quad const left_indices =
        alx_graph_quads_by_dst_to_quad_indices(q, &lhs_iter); 

      struct alx_graph_quad const right_indices =
        alx_graph_quads_by_src_to_quad_indices(q, &rhs_iter);

      struct alx_graph_quad const tmp_indices = {
        left_indices.v1,
        left_indices.v4,
        right_indices.v1,
        right_indices.v4
      };

      struct alx_vertexp_quad const tmp =
        alx_graph_quad_indices_to_quad( q, &tmp_indices );

#if 0
      alx_logf( p, "handle_triple...\n" );
#endif

      bool const triple_ok = alx_handle_triple(
        p,
        q,
        &tmp_indices,
        assume_exclusion_failed
      );

      if (!triple_ok)
        continue;

      bool changed_this_time = false;

#if 0
      alx_logf( p, "insert...\n" );
#endif

      bool const had_exteriors = 
        alx_graph_has_quad_exteriors(q, &tmp_indices);

      alx_graph_insert_quad(
        q,
        &tmp_indices,
        &changed_this_time,
        round
      );

      if (!changed_this_time)
        continue;

      *changed_at_all = true;

      // TODO: write comment
      if (had_exteriors) {

        continue;

      }

      alx_handle_new_exteriors(
        p,
        q,
        changed_at_all,
        round,
        &tmp,
        &tmp_indices,
        cexclusions
      );

    }
  }

#if 0
  alx_logf(p, "END lhs_iter %u %u\n", lhs_iter.node_ix, lhs_iter.link_ix);
#endif

}

/////////////////////////////////////////////////////////////////////
// random paths
/////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////

void
alx_handle_cexclusions(
  struct alx_parser_state * const p,
  struct alx_graph * const q,
  bool * const changed_at_all,
  uint32_t const round,
  struct alx_v32 * const cexclusions
) {

  size_t change_counter = 0;

  alx_logf( p, "HANDLE EXCUSIONS\n" );

#if 0
  // TODO: why?
  alx_v32_qsort_whole(
    cexclusions,
    4,
    alx_cexclusions_cmp // FIXME: cmp is not implemented
  );
#endif

  size_t const size_before = alx_v32_size( cexclusions );

  for (
    size_t cexclusions_ix = 0;
    cexclusions_ix < alx_v32_size( cexclusions );
    cexclusions_ix += 4
  ) {

    bool changed = false;
    alx_handle_single(
      p,
      q,
      alx_v32_at( cexclusions, cexclusions_ix + 0 ),
      alx_v32_at( cexclusions, cexclusions_ix + 1 ),
      &changed,
      round,
      true,
      cexclusions
    );

    if (changed) {
      *changed_at_all = true;
      change_counter += 1;
    }

    // TODO: This requires some thinking. Can we resolve all stalled
    // exclusions at once? Then the check below ought to go. If not,
    // why not? And in that case, it makes no sense to keep a change
    // counter.

    if (alx_v32_size( cexclusions ) != size_before) {
      break;
    }

  }

  alx_logf( p, "finished cexclusion handling, changed %zu\n", change_counter);

}

void
alx_finish_forest_init(
  struct alx_parser_state * const p,
  struct alx_graph * const q,
  struct alx_v32 * const cexclusions
) {

  struct alx_graph_quads_by_any by_any2 = alx_graph_quads_by_any(q);
  size_t changed_total = 0;

  alx_logf(
    p,
    "Handling all edges (excluding siblings)\n"
  );

//  alx_log_memory(p);

  size_t log_counter = 0;

  while (alx_graph_quads_by_any_next(q, &by_any2)) {

    struct alx_graph_quad const middle_indices = 
      alx_graph_quads_by_any_to_quad_indices(q, &by_any2);

    struct alx_vertexp_quad const middle_quad = 
      alx_graph_quad_indices_to_quad( q, &middle_indices );

    struct alx_graph_node const middle_v1 = 
      alx_graph_node_from_ix( q, middle_indices.v1 );

    struct alx_graph_node const middle_v4 = 
      alx_graph_node_from_ix( q, middle_indices.v4 );

    struct alx_vertexp_pair const middle = {
      middle_v1.pos,
      middle_v1.vertex,
      middle_v4.pos,
      middle_v4.vertex
    };

    // skip csiblings, they will be handled seperately later.
    if (alx_vertexp_pair_is_poppush(alx_g(p), &middle)) {

      continue;

    }

    // skip bad edge
    if (alx_vertexp_pair_is_unbalanced_pushpop(alx_g(p), &middle)) {
      // TODO: this can no longer happen 2021bug
      alx_logf(
        p,
        "skipping bad edge (%u,%u) -> (%u,%u)\n",
        middle_v1.pos,
        middle_v1.vertex,
        middle_v4.pos,
        middle_v4.vertex
      );
      continue;

    }

    bool changed = false;
    alx_handle_single(
      p,
      q,
      middle_indices.v1,
      middle_indices.v4,
      &changed,
      1,
      false,
      cexclusions
    );

    changed_total += changed;

  }

  alx_logf(
    p,
    "Done Handling all edges (changed_total %zu)\n",
    changed_total
  );

}

void
alx_finish_forest(
  struct alx_parser_state * const p,
  struct alx_graph * const q,
  struct alx_v32 * const csiblings,
  struct alx_v32 * const cexclusions
) {

  alx_finish_forest_init(p, q, cexclusions);

  for (uint32_t round = 1; ; ++round) {

    size_t changes_this_round = 0;

    alx_logf(
      p,
      "applying %u siblings, round %u\n",
      alx_v32_size( csiblings ),
      round
    );

    for (
      size_t csiblings_ix = 0;
      csiblings_ix < alx_v32_size( csiblings );
      csiblings_ix += 4
    ) {

      uint32_t const src_ix = alx_v32_at( csiblings, csiblings_ix + 0);
      uint32_t const dst_ix = alx_v32_at( csiblings, csiblings_ix + 1);

      struct alx_graph_node const src_node = 
        alx_graph_node_from_ix( q, src_ix );

      struct alx_graph_node const dst_node = 
        alx_graph_node_from_ix( q, dst_ix );

      uint32_t const src_changed_round = 
        src_node.lastmodified;

      uint32_t const dst_changed_round = 
        dst_node.lastmodified;

      alx_vertex const src_vertex =
        src_node.vertex;

#if 0
      alx_logf( p, "---> %u %u %u %u\n",
        src_node.pos,
        src_node.vertex,
        dst_node.pos,
        dst_node.vertex
      );
#endif

      if (dst_changed_round < round - 1 && src_changed_round < round - 1) {
        continue;
      }

      bool changed = false;
      alx_handle_single(
        p,
        q, 
        src_ix,
        dst_ix,
        &changed,
        round,
        false,
        cexclusions
      );

      if (changed) {
        changes_this_round += 1;
      }

#if 0
      if (
        dst_changed_round < round - 1
        &&
        src_changed_round < round - 1
        &&
        changed
      ) {

        alx_debugf(
          stderr,
          "BAD round %u changed %u left %u right %u\n",
          round,
          changed,
          src_changed_round,
          dst_changed_round
        );

      }
#endif

    }

    alx_logf(
      p,
      "%zu csiblings touched this round (cexclusions %zu)\n",
      changes_this_round,
      alx_v32_size( cexclusions )
    );

    bool const has_exclusions = 
      alx_v32_size( cexclusions ) > 0;

#if 0
    if (!has_exclusions) {
      alx_logf(p, "done after 1 round because no discontinuity\n");
      break;
    }
#endif

    if (changes_this_round) {
      continue;
    }

    bool changed_by_exclusions = false;
    alx_handle_cexclusions(
      p,
      q,
      &changed_by_exclusions,
      round,
      cexclusions
    );

    if (!changed_by_exclusions) {
      break;
    }

  }

}

bool
alx_log_matches_graph(
  struct alx_parser_state const* const p,
  struct alx_graph const* const q
) {

  alx_logf( p, "LOG MATCHES:\n" );

  // TODO: function to get alx_vertex_index_ix for 1,start
  // TODO: which iterator would then be used?
  // DONE: ^ alx_graph_quads_by_src

  struct alx_graph_quads_by_src mermaid =
    alx_graph_quads_by_src_vertexp(
      q,
      1,
      alx_grammar_start_vertex(alx_g(p)),
      alx_size_t_to_uint32_t( alx_v32_size( &q->nodes ) - 2 )
    );

  bool result = false;

  while (alx_graph_quads_by_src_next(q, &mermaid)) {

    struct alx_vertexp_quad r = alx_graph_quads_by_src_to_quad(q, &mermaid);

    if (r.dst_pos != p->final_index) {
      continue;
    }

    if (r.dst_vertex != alx_grammar_final_vertex(alx_g(p))) {
      continue;
    }

    alx_log_quad( p, "match root quad: ", &r );

    result = true;

  }

  return result;

}

void
alx_bottom_up(
  struct alx_parser_state * const p,
  struct alx_graph * const q
) {

  alx_collapse_collapse(
    &p->collapse,
    &p->bottom_up.cedges,
    &p->bottom_up.cfinals
  );

  alx_graph_init_from_edges(
    q,
    &p->bottom_up.vertex_index,
    &p->bottom_up.cfinals
  );

  alx_log_memory( p );
  alx_logf( p, "creating csiblings\n" );
  alx_graph_to_siblings_and_exclusions( q, &p->bottom_up.csiblings );
  alx_logf( p, "done creating csiblings\n" );

  // TODO: deduplicate cfinals?
  // TODO: deduplicate csiblings?
  // No, they can't have duplicates anymore

  // Among other things, the loop above attempts to approximate for
  // each edge in `p->csiblings` the nesting depth within the
  // parse graph by counting push and pop vertices in the sorted
  // `p->cfinals` vector. For matches with many branches the result can
  // be far off, but for unambiguous matches this would establish
  // proper topological order, minimising the number of times edges
  // have to be retried.

  // TODO: is there actually still a point to that ^?

#if 1

  alx_logf(
    p,
    "sorting %zu csiblings...\n",
    alx_v32_size( &p->bottom_up.csiblings )
  );

  alx_v32_qsort_whole(
    &p->bottom_up.csiblings,
    4,
    alx_vertexp_quad_cmp_special
  );
  alx_logf(p, "done\n");

#endif

//return;

  alx_finish_forest(
    p,
    q,
    &p->bottom_up.csiblings,
    &p->bottom_up.cexclusions
  );

  alx_graph_cleanup_ordered_choices( q );

}

