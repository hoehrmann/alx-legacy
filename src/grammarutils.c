#include "alx.h"

/////////////////////////////////////////////////////////////////////
// Grammar properties
/////////////////////////////////////////////////////////////////////

size_t
alx_grammar_name_length_max_bytes(
  struct alx_grammar const * const grammar
) {
  // FIXME: get this from grammar
  return 1024;
}

alx_vertex
alx_grammar_min_vertex(
  struct alx_grammar const * const grammar
) {
  // TODO: use actual minimum
  return 1;
}

alx_vertex
alx_grammar_max_vertex(
  struct alx_grammar const * const grammar
) {
  // TODO: use actual maximum
  return grammar->count_vertices - 1;
}

alx_vertex
alx_grammar_start_vertex(
  struct alx_grammar const * const grammar
) {
  return grammar->start_vertex;
}

alx_vertex
alx_grammar_final_vertex(
  struct alx_grammar const * const grammar
) {
  return grammar->final_vertex;
}

/////////////////////////////////////////////////////////////////////
// Boolean flags for vertices
/////////////////////////////////////////////////////////////////////

bool
alx_gv_is_push(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].is_push;
}

bool
alx_gv_is_pop(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].is_pop;
}

bool
alx_gv_is_if(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].is_if;
}

bool
alx_gv_is_if1(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].is_if1;
}

bool
alx_gv_is_if2(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].is_if2;
}

bool
alx_gv_is_fi2(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].is_fi2;
}

bool
alx_gv_is_fi(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].is_fi;
}

bool
alx_gv_is_irregular(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].is_irregular;
}

bool
alx_gv_is_linear(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].is_linear;
}

bool
alx_gv_is_skippable(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {

#if 0
  char const* const name = grammar->static_string[
    grammar->vertex_property[ vertex ].name
  ].str;
  bool name_is_rule = strcmp("rule", name ) == 0;
  return grammar->vertex_property[ vertex ].is_skippable && !name_is_rule;
#endif

  return grammar->vertex_property[ vertex ].is_skippable;
}

bool
alx_gv_is_input(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].is_input;
}

bool
alx_gv_is_conjunction(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].is_conjunction;
}

bool
alx_gv_is_ordered_conjunction(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].is_ordered_conjunction;
}

bool
alx_gv_is_ordered_choice(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].is_ordered_choice;
}

bool
alx_gv_is_exclusion(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].is_exclusion;
}

bool
alx_gv_is_exclusion_if(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return
    grammar->vertex_property[ vertex ].is_exclusion
    &&
    grammar->vertex_property[ vertex ].is_if
    ;
}

bool
alx_gv_is_exclusion_if1(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return
    grammar->vertex_property[ vertex ].is_exclusion
    &&
    grammar->vertex_property[ vertex ].is_if1
    ;
}

/////////////////////////////////////////////////////////////////////
// Other vertex properties
/////////////////////////////////////////////////////////////////////

alx_topological_id
alx_gv_topo(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].topo;
}

alx_vertex
alx_gv_p1(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].p1;
}

alx_vertex
alx_gv_p2(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].p2;
}

alx_vertex
alx_gv_partner(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].partner;
}

alx_vertex
alx_gv_one_in_loop(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->vertex_property[ vertex ].one_in_loop;
}

char const*
alx_gv_type(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->static_string[
    grammar->vertex_property[ vertex ].type
  ].str;
}

char const*
alx_gv_name(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->static_string[
    grammar->vertex_property[ vertex ].name
  ].str;
}

char const*
alx_gv_self_loop(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->static_string[
    grammar->vertex_property[ vertex ].self_loop
  ].str;
}

char const*
alx_gv_contents_self_loop(
  struct alx_grammar const * const grammar,
  alx_vertex const vertex
) {
  return grammar->static_string[
    grammar->vertex_property[ vertex ].contents_self_loop
  ].str;
}

/////////////////////////////////////////////////////////////////////
// Pointers to zero-terminated vertex lists associated with vertices.
/////////////////////////////////////////////////////////////////////

alx_vertex const*
alx_gv_shadows_vlptr(
  struct alx_grammar const* const grammar,
  alx_vertex const vertex
) {
  return &(grammar->vertex_lists[
    grammar->vertex_shadows[vertex] ]);
}

alx_vertex const*
alx_gv_unskippable_shadows_vlptr(
  struct alx_grammar const* const grammar,
  alx_vertex const vertex
) {
  return &(grammar->vertex_lists[
    grammar->vertex_unskippable_shadows[vertex] ]);
}

alx_vertex const*
alx_gv_successors_vlptr(
  struct alx_grammar const* const grammar,
  alx_vertex const vertex
) {
  return &(grammar->vertex_lists[
    grammar->vertex_successors[vertex] ]);
}

alx_vertex const*
alx_gv_unskippable_predecessors_vlptr(
  struct alx_grammar const* const grammar,
  alx_vertex const vertex
) {
  return &(grammar->vertex_lists[
    grammar->vertex_unskippable_predecessors[vertex] ]);
}

alx_vertex const*
alx_gv_old_predecessors_vlptr(
  struct alx_grammar const* const grammar,
  alx_vertex const vertex
) {
  return &(grammar->vertex_lists[
    grammar->vertex_old_predecessors[vertex] ]);
}

alx_vertex const*
alx_grammar_vlix_to_vlptr(
  struct alx_grammar const* const grammar,
  alx_vertex_list_index const offset
) {
  return &(grammar->vertex_lists[ offset ]);
}

/////////////////////////////////////////////////////////////////////
// NFA Transitions
/////////////////////////////////////////////////////////////////////

alx_nfa_state_id
alx_grammar_nfa_next_state(
  struct alx_grammar const* const grammar,
  alx_vertex const prev_state,
  alx_input_class_id const input_class
) {

  return grammar->nfad[
    prev_state
    * grammar->count_input_classes
    + input_class
  ];

}

/////////////////////////////////////////////////////////////////////
// Conditional structures
/////////////////////////////////////////////////////////////////////

struct alx_if_group
alx_gv_if_group(
  struct alx_grammar const* const grammar,
  alx_vertex const if_vertex
) {

  assert(
    alx_gv_is_if( grammar, if_vertex )
  );

  alx_vertex const fi_vertex =
    alx_gv_partner( grammar, if_vertex );

  struct alx_if_group const group = {
    .if_vertex = if_vertex,
    .if1_vertex = alx_gv_p1( grammar, if_vertex ),
    .if2_vertex = alx_gv_p2( grammar, if_vertex ),
    .fi2_vertex = alx_gv_p2( grammar, fi_vertex ),
    .fi1_vertex = alx_gv_p1( grammar, fi_vertex ),
    .fi_vertex = fi_vertex
  };

  return group;

}

/////////////////////////////////////////////////////////////////////
// Computed pair properties
/////////////////////////////////////////////////////////////////////

bool
alx_vertexp_pair_is_pushpush(
  struct alx_grammar const* const g,
  struct alx_vertexp_pair const* const pair
) {

  return (
    alx_gv_is_push( g, pair->src_vertex )
    &&
    alx_gv_is_push( g, pair->dst_vertex )
  );

}

bool
alx_vertexp_pair_is_poppop(
  struct alx_grammar const* const g,
  struct alx_vertexp_pair const* const pair
) {

  return (
    alx_gv_is_pop( g, pair->src_vertex )
    &&
    alx_gv_is_pop( g, pair->dst_vertex )
  );

}

bool
alx_vertexp_pair_is_poppush(
  struct alx_grammar const* const g,
  struct alx_vertexp_pair const* const pair
) {

  return (
    alx_gv_is_pop( g, pair->src_vertex )
    &&
    alx_gv_is_push( g, pair->dst_vertex )
  );

}

bool
alx_vertexp_pair_is_pushpush_or_poppop(
  struct alx_grammar const* const g,
  struct alx_vertexp_pair const* const pair
) {

  return (
    (
      alx_gv_is_push( g, pair->src_vertex )
      &&
      alx_gv_is_push( g, pair->dst_vertex )
    )
    ||
    (
      alx_gv_is_pop( g, pair->src_vertex )
      &&
      alx_gv_is_pop( g, pair->dst_vertex )
    )
  );

}

/////////////////////////////////////////////////////////////////////
// Computed quad properties
/////////////////////////////////////////////////////////////////////

bool
alx_vertexp_quad_is_poppush(
  struct alx_grammar const* const g,
  struct alx_vertexp_quad const* const quad
) {

  return (
    alx_gv_is_pop( g, quad->src_vertex )
    &&
    alx_gv_is_push( g, quad->dst_vertex )
  );

}

bool
alx_vertexp_quad_is_pushpop(
  struct alx_grammar const* const g,
  struct alx_vertexp_quad const* const quad
) {

  return (
    alx_gv_is_push( g, quad->src_vertex )
    &&
    alx_gv_is_pop( g, quad->dst_vertex )
  );

}

bool 
alx_vertexp_quad_is_parent_child(
  struct alx_grammar const* const g,
  struct alx_vertexp_quad const* const quad
) {

  return (
    alx_gv_is_push( g, quad->src_vertex )
    && 
    alx_gv_is_push( g, quad->mid_src_vertex )
    &&
    alx_gv_is_pop( g, quad->mid_dst_vertex )
    &&
    alx_gv_partner( g, quad->dst_vertex )
      == quad->src_vertex
  );

}

bool
alx_vertexp_quad_is_siblings(
  struct alx_grammar const* const g,
  struct alx_vertexp_quad const* const quad
) {

  return (
    alx_gv_is_push( g, quad->src_vertex )
    &&
    alx_gv_is_pop( g, quad->mid_src_vertex )
    &&
    alx_gv_is_push( g, quad->mid_dst_vertex )
    &&
    alx_gv_is_pop( g, quad->dst_vertex )
  );

}

bool
alx_vertexp_quad_is_unbalanced_pushpop(
  struct alx_grammar const* const g,
  struct alx_vertexp_quad const* const quad
) {

  return (
    alx_gv_is_push( g, quad->src_vertex )
    &&
    alx_gv_is_pop( g, quad->dst_vertex )
    &&
    alx_gv_partner( g, quad->src_vertex )
      != quad->dst_vertex
  );

}

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

bool
alx_vertexp_pair_is_unbalanced_pushpop(
  struct alx_grammar const* const g,
  struct alx_vertexp_pair const* const pair
) {

  return (
    alx_gv_is_push( g, pair->src_vertex )
    &&
    alx_gv_is_pop( g, pair->dst_vertex )
    &&
    alx_gv_partner( g, pair->src_vertex )
      != pair->dst_vertex
  );

}

/////////////////////////////////////////////////////////////////////
// UTF-8 character class transitions
/////////////////////////////////////////////////////////////////////

alx_utf8_state
alx_grammar_utf8_next_state(
  struct alx_grammar const* const grammar,
  alx_utf8_state const prev_state,
  uint8_t const byte
) {

  return grammar->utf8d[ prev_state * 256 + byte ];

}

