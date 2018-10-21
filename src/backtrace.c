#include "alx.h"

void
alx_foo_one_vertex(
  struct alx_backtrace_state * const bt,
  alx_vertex const vertex
) {

  if (alx_gv_is_input( alx_g(bt->p), vertex )) {

    alx_unordered_vertex_set_insert(bt->vprev, vertex);

  } else {

    alx_unordered_vertex_set_insert(bt->vnext, vertex);

  }

  // FIXME: this logic needs documentation

  alx_vertex const* input_shadows_ptr =
    &( alx_g(bt->p)->vertex_lists[ bt->shadows[ vertex ] ] );

  for (; *input_shadows_ptr; ++input_shadows_ptr) {

    alx_unordered_vertex_set_insert(bt->vprev, *input_shadows_ptr);

  }

  alx_vertex const* successors_vlptr =
    alx_gv_successors_vlptr(alx_g(bt->p), vertex);
  
  for (; *successors_vlptr; ++successors_vlptr) {

#if 0
    // Not sure that this is needed?

    alx_unordered_vertex_set_insert(
      bt->vnext,
      *successors_vlptr
    );
#endif

    alx_vertex const* successor_shadows_ptr =
      &( alx_g(bt->p)->vertex_lists[ bt->shadows[ *successors_vlptr ] ] );

    for (; *successor_shadows_ptr; ++successor_shadows_ptr) {
      alx_unordered_vertex_set_insert(
        bt->vnext,
        *successor_shadows_ptr
      );
    }
  }


}

ALX_INLINE
void
alx_foo_one_pos(
  struct alx_backtrace_state * const bt,
  alx_vertex_list_index const* * const m_ptr
) {

  while (true) {

    alx_vertex const* input_vlptr = 
      alx_grammar_vlix_to_vlptr(
        alx_g(bt->p),
        alx_g(bt->p)->nfa_states[ alx_uint32_lsb31( **m_ptr ) ].vertices
      );

    for (; *input_vlptr; ++input_vlptr) {
      alx_foo_one_vertex(bt, *input_vlptr);
    }

    if (alx_uint32_msb1(**m_ptr) == 0) {
      --*m_ptr;
      break;
    }

    --*m_ptr;

  }

}

void
alx_indexed_push_back(
  struct alx_backtrace_state * const bt,
  struct alx_unordered_vertex_set * const src_set,
  struct alx_vertexp_pair const* const insertee
) {

  // FIXME: use alx_backtrace_index_final_ix instead of `2`

  // dst_pos/dst_vertex MUST have vertex_id in bt->vnext
  assert(
    bt->vnext->index[ insertee->dst_vertex ]
    >=
    2
  );

  // src_pos/src_vertex MAY have vertex_id in src_set
  if ( src_set->index[ insertee->src_vertex ] < 2 ) {

    src_set->index[ insertee->src_vertex ] = 
      alx_size_t_to_uint32_t( alx_v32_size( bt->vertex_index ) );

    alx_v32_push_back2(
      bt->vertex_index,
      insertee->src_pos,
      insertee->src_vertex
    );

  }

  alx_v32_push_back2(
    bt->edges,
    src_set->index[ insertee->src_vertex ],
    bt->vnext->index[ insertee->dst_vertex ]
  );

}

ALX_INLINE
void
alx_ps_add_edge(
  struct alx_backtrace_state * const bt,
  alx_vertex const src,
  alx_vertex const dst,
  size_t const current_index
) {

  if (alx_gv_is_input( alx_g(bt->p), src )) {

    if (alx_unordered_vertex_set_has(bt->vprev, src)) {

      struct alx_vertexp_pair const insertee = {
        alx_size_t_to_uint32_t( current_index - 1 ),
        src,
        alx_size_t_to_uint32_t( current_index ),
        dst
      };

      alx_indexed_push_back(
        bt,
        bt->vprev,
        &insertee
      );

      alx_unordered_vertex_set_insert(bt->prevtodo, src);

    } else {
//      alx_debugf(stderr, "vprev does not have %u\n", src);
    }

  } else {

    // TODO(bh): This cannot work unless we always do the mapping?
    // TODO(bh): Why not?

    alx_vertex const one =
      alx_gv_one_in_loop(alx_g(bt->p), src);

    if (alx_unordered_vertex_set_has(bt->vnext, one)) {

      struct alx_vertexp_pair const insertee = {
        alx_size_t_to_uint32_t( current_index ),
        src,
        alx_size_t_to_uint32_t( current_index ),
        dst
      };

      alx_indexed_push_back(
        bt,
        bt->vnext,
        &insertee
      );

      alx_unordered_vertex_set_insert(bt->nexttodo, src);

    } else {
      
#if 0
      alx_debugf(stderr, "vnext does not have %u (mapped to %u)\n", src, one);
      for (size_t d = 0; d < alx_g(p)->count_vertices; ++d) {
        if (alx_unordered_vertex_set_has(bt->vnext, d)) {
          alx_debugf(stderr, "  vnext does have %zu\n", d);
        }
      }
#endif

    }
  }


}

void
alx_backtrace_init(
  struct alx_parser_state * const p,
  struct alx_backtrace_state * const bt,
  struct alx_v32 * const vertex_index,
  struct alx_v32 * const edges,
  alx_offset const dst_pos,
  struct alx_unordered_vertex_set const* const dst_set,
  bool const unskippables
) {

  // to avoid passing very many arguments all the time
  // TODO: but initialisation should be done differently
  // There is basically no need to keep this state data
  // in the parser_state to begin with, so the main 
  // backtrace function should probably just allocate it
  // on the stack and pass it around.

  bt->p = p;
  bt->vertex_index = vertex_index;
  bt->edges = edges;
  bt->unskippables = unskippables;

  alx_v32_clear( vertex_index );

  alx_v32_push_back2(
    vertex_index,
    0,
    0
  );

  alx_unordered_vertex_set_clear( bt->vprev );
  alx_unordered_vertex_set_clear( bt->vnext );
  alx_unordered_vertex_set_clear( bt->nexttodo );
  alx_unordered_vertex_set_clear( bt->prevtodo );

  for (size_t ix = 0; ix < dst_set->size; ++ix) {

    alx_vertex const dst_vertex = dst_set->vector[ix];

    alx_unordered_vertex_set_insert(
      bt->vnext,
      dst_vertex
    );

    bt->vnext->index[ dst_vertex ] = 
      alx_size_t_to_uint32_t(alx_v32_size( vertex_index ));

    alx_v32_push_back2(
      vertex_index,
      dst_pos,
      dst_vertex
    );

#if 0
    // TODO: why disabled? remove if not needed?

    alx_unordered_vertex_set_insert(
      bt->nexttodo,
      0
    );
#endif

    alx_unordered_vertex_set_insert(
      bt->nexttodo,
      dst_vertex
    );

  }

}

void
alx_backtrace_swap(
  struct alx_backtrace_state * const bt
) {

  alx_unordered_vertex_set_clear(bt->nexttodo);
  alx_unordered_vertex_set_swap(&bt->nexttodo, &bt->prevtodo);

  alx_unordered_vertex_set_clear(bt->vnext);
  alx_unordered_vertex_set_swap(&bt->vnext, &bt->vprev);

}

void
alx_backtrace_one_index(
  struct alx_backtrace_state * const bt,
  struct alx_vertexp const* const src,
  alx_vertex_list_index const* * const m_ptr,
  size_t * const current_index
) {

#if 0
  alx_debugf(stderr, "Initial nexttodo size %zu (first: %u, m_ptr %u)\n", bt->nexttodo->size,
    bt->nexttodo->vector[0], **m_ptr);
#endif

  alx_foo_one_pos(bt, m_ptr);

  for (
    size_t nexttodo_ix = 0;
    nexttodo_ix < bt->nexttodo->size;
    nexttodo_ix++
  ) {


    alx_vertex const dst_vertex =
      bt->nexttodo->vector[nexttodo_ix];

    alx_vertex const* predecessors_ptr =
      &( alx_g(bt->p)->vertex_lists[ bt->predecessors[ dst_vertex ] ] );

    for (; *predecessors_ptr; ++predecessors_ptr) {

      alx_vertex const src_vertex = *predecessors_ptr;

      // main calls with unskippables = true;
      // random* calls with unskippables = false;

      if (
        !bt->unskippables
        &&
        !alx_gv_is_skippable(alx_g(bt->p), src_vertex)
        &&
        !(*current_index == src->pos && src_vertex == src->vertex)
      ) {

        continue;

      } else {
        
      }

      alx_ps_add_edge(
        bt,
        src_vertex,
        dst_vertex,
        *current_index
      );

    }
    
  }

#if 0
  alx_debugf(stderr, "    ... prevtodo.size %zu (first: %u, m_ptr %u ~~ %zu)\n", bt->prevtodo->size,
    bt->prevtodo->vector[0], **m_ptr, bt->vprev->size);
#endif

}

void
alx_backtrace_p(
  struct alx_parser_state * const p,
  struct alx_v32 * const vertex_index,
  struct alx_v32 * const edges,
  struct alx_vertexp const* const src,
  alx_offset const dst_pos,
  struct alx_unordered_vertex_set const* const dst_set,
  alx_vertex_list_index const* m_ptr,
  bool const unskippables
) {

  // FIXME: evil
  struct alx_backtrace_state * const bt = &p->backtrace_state;

  alx_backtrace_init(
    p,
    bt,
    vertex_index,
    edges,
    dst_pos,
    dst_set,
    unskippables
  );

  for (
    size_t current_index = dst_pos;
    current_index >= src->pos;
    current_index--
  ) {

    alx_backtrace_one_index(
      bt,
      src,
      &m_ptr,
      &current_index
    );

    alx_backtrace_swap(bt);

  }
  
#if 0
  // FIXME: broken if no edges
  alx_logf( p, "final edge: %u %u %u %u\n",
    alx_v32_at( edges, alx_v32_size(edges) - 4 ),
    alx_v32_at( edges, alx_v32_size(edges) - 3 ),
    alx_v32_at( edges, alx_v32_size(edges) - 2 ),
    alx_v32_at( edges, alx_v32_size(edges) - 1 )
  );
#endif

}

void
alx_backtrace(
  struct alx_parser_state * const p,
  struct alx_v32 * const vertex_index,
  struct alx_v32 * const edges,
  struct alx_vertexp const* const src,
  struct alx_vertexp const* const dst,
  alx_vertex_list_index const* m_ptr,
  bool const unskippables
) {
  
  struct alx_unordered_vertex_set dst_set = { 0 };

  alx_unordered_vertex_set_insert( &dst_set, dst->vertex );

  alx_backtrace_p(
    p,
    vertex_index,
    edges,
    src,
    dst->pos,
    &dst_set,
    m_ptr,
    unskippables
  );

}

void
alx_backtrace_add_candidates(
  struct alx_parser_state const* const p,
  alx_vertex_list_index const six,
  struct alx_unordered_vertex_set * const result
) {

  // FIXME: this is fairly insane

  alx_vertex const* input_vlptr = 
    alx_grammar_vlix_to_vlptr(
      alx_g(p),
      alx_g(p)->nfa_states[
        alx_uint32_lsb31( six ) ].vertices
    );


  for (; *input_vlptr; ++input_vlptr) {

#if 0
    alx_logf(p, "input_vlptr %u\n", *input_vlptr);
#endif

    alx_vertex const* successors_vlptr =
      alx_gv_successors_vlptr(
        alx_g(p),
        *input_vlptr
      );

    for (; *successors_vlptr; ++successors_vlptr) {

#if 0
      alx_logf(p, "successors_vlptr %u\n", *successors_vlptr);
#endif

      for (
        uint32_t input_class = 1;
        input_class < alx_g(p)->count_input_classes;
        ++input_class
      ) {

#if 0
        alx_logf(p, "input_class %u\n", input_class);
#endif

        alx_nfa_state_id const dst_id =
          alx_grammar_nfa_next_state(
            alx_g(p),
            *successors_vlptr,
            input_class
          );

#if 0
        alx_logf(p, "dst_id %u\n", dst_id);
#endif

        alx_vertex const* dst_vlptr = 
          alx_grammar_vlix_to_vlptr(
            alx_g(p),
            alx_g(p)->nfa_states[ dst_id ].vertices
          );

        for (; *dst_vlptr; ++dst_vlptr) {

          alx_vertex const* next_shadows_ptr =
            &( alx_g(p)->vertex_lists[ p->backtrace_state.shadows[ *dst_vlptr ] ] );

          for (; *next_shadows_ptr; ++next_shadows_ptr) {

#if 0
            alx_logf(p, "add %u\n", *next_shadows_ptr);
#endif

            alx_unordered_vertex_set_insert(
              result,
              *next_shadows_ptr
            );

          }

        }
      }
    }
  }
}

void
alx_backtrace_from_failure(
  struct alx_parser_state * const p,
  struct alx_v32 * const vertex_index,
  struct alx_v32 * const edges
) {
  
  struct alx_unordered_vertex_set dst_set = { 0 };

  assert( alx_v32_size( &p->shadowed_inputs ) );

  alx_vertex_list_index const* last_ptr =
    alx_v32_last_ptr( &p->shadowed_inputs );

  alx_vertex_list_index const* last_nz_ptr =
    alx_v32_last_nonzero_ptr( &p->shadowed_inputs );

  alx_offset const dst_pos = (
    p->final_index
    -
    alx_ptrdiff_t_to_size_t(last_ptr - last_nz_ptr)
  );

  for (
    alx_vertex_list_index const* ptr = last_nz_ptr;
    ptr >= p->shadowed_inputs.d;
    /**/
  ) {

    alx_backtrace_add_candidates( p, *ptr, &dst_set );
    ptr -= 1;

    if (!alx_uint32_msb1(*ptr)) {
      break;
    }

  }

  struct alx_vertexp_pair const end_pair = alx_ps_get_endpoints( p );

  struct alx_vertexp const src = {
    end_pair.src_pos,
    end_pair.src_vertex
  };

  alx_backtrace_p(
    p,
    vertex_index,
    edges,
    &src,
    dst_pos,
    &dst_set,
    last_nz_ptr,
    true
  );

}


/////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////

alx_collapse_node_ix
alx_backtrace_index_final_ix(
  struct alx_v32 const* const vertex_index
) {

  return 2;

}

