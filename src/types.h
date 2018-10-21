/////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////

typedef uint32_t alx_vertex;
typedef uint32_t alx_topological_id;
typedef uint32_t alx_utf8_state;
typedef uint32_t alx_count;
typedef uint32_t alx_vertex_list_index;
typedef uint32_t alx_nfa_state_id;
typedef uint32_t alx_input_class_id;
typedef uint32_t alx_offset;
typedef uint32_t alx_string_id;
typedef uint32_t alx_vertex_index_ix;

struct alx_parser_state;

struct alx_log_context {
  struct alx_parser_state const* const p;
  char const* const file;
  char const* const func;
  size_t const line;
};

struct alx_vertex_property {
  alx_string_id const name;
  alx_string_id const type;
  alx_string_id const self_loop;
  alx_string_id const contents_self_loop;

  alx_vertex const vertex;

  alx_vertex const p1;
  alx_vertex const p2;
  alx_vertex const partner;
  alx_topological_id const topo;
  alx_vertex const one_in_loop;

//  uint32_t const stack_group;
//  uint32_t const indirect_stack_group;
//  uint32_t const representative;

  uint8_t const is_input;
  uint8_t const is_skippable;

  bool const is_if : 1;
  bool const is_push : 1;
  bool const is_pop : 1;
  bool const is_stack : 1;
  bool const is_irregular : 1;
  bool const is_linear : 1;
  bool const is_start : 1;
  bool const is_final : 1;
  bool const is_fi : 1;
  bool const is_if1 : 1;
  bool const is_fi1 : 1;
  bool const is_if2 : 1;
  bool const is_fi2 : 1;
  bool const is_empty : 1;

  bool const is_ordered_choice : 1;
  bool const is_ordered_conjunction : 1;
  bool const is_conjunction : 1;
  bool const is_exclusion : 1;

  // TODO: derived property for is_if&is_exclusion
  // or at least make sure they fall into same uint32_t

};

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

struct alx_nfa_state {
  alx_vertex_list_index const vertices;
};

struct alx_static_string {
  char const* const str;
};

/////////////////////////////////////////////////////////////////////
// Structs
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// More complex positioned vertex structures
/////////////////////////////////////////////////////////////////////

struct alx_vertexp {
  alx_offset pos;
  alx_vertex vertex;
};

struct alx_vertexp_pair {
  alx_offset src_pos;
  alx_vertex src_vertex;
  alx_offset dst_pos;
  alx_vertex dst_vertex;
};

struct alx_vertexp_quad {
  alx_offset src_pos;
  alx_vertex src_vertex;
  alx_offset mid_src_pos;
  alx_vertex mid_src_vertex;
  alx_offset mid_dst_pos;
  alx_vertex mid_dst_vertex;
  alx_offset dst_pos;
  alx_vertex dst_vertex;
};

struct alx_vertex_index_ix_pair {
  alx_vertex_index_ix const src_ix;
  alx_vertex_index_ix const dst_ix;
};

struct alx_v32 {
  struct alx_parser_state * p;

  char const* const native_name;
  char const* const table_name;
  char const* const insert_template;
  size_t cardinality;

  size_t capacity;
  size_t size;
  uint32_t *d;
};

struct alx_itoa {
  char itoa3[ 4 * 1000 ];
  char mag_buf[16];
  size_t mag_len;
  uint32_t mag_val;
};

struct alx_string {
  size_t capacity;
  size_t size;
  char * data;
  struct alx_parser_state * p; // TODO: move to front?
  struct alx_itoa itoa;
};

struct alx_shadowed_inputs_cursor {
  alx_offset pos;
  size_t last_at;
};

struct alx_rng {
  uint32_t dummy;
};

/////////////////////////////////////////////////////////////////////
// C++
/////////////////////////////////////////////////////////////////////

struct alx_set;

/////////////////////////////////////////////////////////////////////
// Graph types
/////////////////////////////////////////////////////////////////////

typedef uint32_t alx_graph_node_ix;
typedef uint32_t alx_graph_link_ix;
typedef uint32_t alx_graph_quad_ix;
typedef uint32_t alx_graph_lastmod;
typedef uint32_t alx_graph_pending;

struct alx_graph {
  struct alx_parser_state * p;
  struct alx_v32 nodes;
  struct alx_v32 links;
  struct alx_v32 quads;
  struct alx_v32 old2new;

  struct alx_v32 predecessors;
  struct alx_v32 successors;
  struct alx_v32 lastmodified;
  struct alx_v32 pending;

  struct alx_set* const set;
};

struct alx_graph_quad {
  alx_graph_node_ix const v1;
  alx_graph_node_ix const v2;
  alx_graph_node_ix const v3;
  alx_graph_node_ix const v4;
};

struct alx_graph_node {
  alx_offset const pos;
  alx_vertex const vertex;
  alx_graph_link_ix const predecessors;
  alx_graph_link_ix const successors;
  alx_graph_lastmod const lastmodified;
  alx_graph_pending const pending;
};

struct alx_graph_quads_by_src {
  alx_graph_node_ix const node_ix;
  alx_graph_link_ix link_ix;
};

struct alx_graph_quads_by_dst {
  alx_graph_node_ix const node_ix;
  alx_graph_link_ix link_ix;
};

struct alx_graph_quads_by_any {
  alx_graph_node_ix node_ix;
  alx_graph_link_ix link_ix;
};

/////////////////////////////////////////////////////////////////////
// collapse types
/////////////////////////////////////////////////////////////////////

typedef uint32_t alx_collapse_node_ix;
typedef uint32_t alx_collapse_round;

struct alx_collapse_node_ix_pair {
  alx_collapse_node_ix src_ix;
  alx_collapse_node_ix dst_ix;
};

struct alx_collapse {

  struct alx_parser_state * p;

  struct alx_v32 vertices;
  struct alx_v32 lastround;
  struct alx_v32 position;

  struct alx_v32 edges;
  struct alx_v32 lhs_edges;
  struct alx_v32 rhs_edges;

  struct alx_v32 node_index;

};

/////////////////////////////////////////////////////////////////////
// random matches
/////////////////////////////////////////////////////////////////////

struct alx_random_match {
  struct alx_v32 sorted_quads;
  struct alx_v32 quad_path;
  struct alx_v32 quad_stack;
  struct alx_v32 gap_index;
  struct alx_v32 gap_edges;
  struct alx_v32 full_path;
  struct alx_v32 stack;
  struct alx_v32 endpoints;
  struct alx_v32 json_stack;
};

/////////////////////////////////////////////////////////////////////
// Conditionals
/////////////////////////////////////////////////////////////////////

struct alx_if_group {
  alx_vertex const if_vertex;
  alx_vertex const if1_vertex;
  alx_vertex const if2_vertex;
  alx_vertex const fi2_vertex;
  alx_vertex const fi1_vertex;
  alx_vertex const fi_vertex;
};

/////////////////////////////////////////////////////////////////////
// vector meta data
/////////////////////////////////////////////////////////////////////

struct alx_ps_v32_meta {
  struct alx_v32 * const vector;
  char const* const native_name;
  char const* const table_name;
  char const* const insert_template;
  size_t cardinality;
};

/////////////////////////////////////////////////////////////////////
// ...
/////////////////////////////////////////////////////////////////////

#include "grammar.h"

#define ALX_MEMBER_SIZE(type, member) sizeof(((type *)0)->member)

struct alx_unordered_vertex_set {
  size_t capacity;
  size_t size;
  // FIXME: probably needs to divide by sizeof()
  alx_vertex vector[
    ALX_MEMBER_SIZE(struct alx_grammar, vertex_shadows)
  ];
  uint32_t index[
    ALX_MEMBER_SIZE(struct alx_grammar, vertex_shadows)
  ];
};

#undef ALX_MEMBER_SIZE

/////////////////////////////////////////////////////////////////////
// command line argument data
/////////////////////////////////////////////////////////////////////
struct alx_cmd_args {
  bool help;
  char const* in_path;
  char const* random_path;
  char const* db_path;
  char const* query;
  char const* query_args;
  bool all_edges;
  unsigned int seed;
  int argc;
  char const* const* const argv;
  struct alx_unordered_vertex_set random_omit;
};

/////////////////////////////////////////////////////////////////////
// backtrace
/////////////////////////////////////////////////////////////////////

struct alx_backtrace_state {

  struct alx_parser_state * p;
  struct alx_v32 * vertex_index;
  struct alx_v32 * edges;
  bool unskippables;

  struct alx_unordered_vertex_set * vprev;
  struct alx_unordered_vertex_set * vnext;
  struct alx_unordered_vertex_set * nexttodo;
  struct alx_unordered_vertex_set * prevtodo;

  // TODO: naming...
  struct alx_unordered_vertex_set vprev_;
  struct alx_unordered_vertex_set vnext_;
  struct alx_unordered_vertex_set nexttodo_;
  struct alx_unordered_vertex_set prevtodo_;

  // TODO: do these belong here?
  alx_vertex_list_index const* predecessors;
  alx_vertex_list_index const* shadows;

};

/////////////////////////////////////////////////////////////////////
// main struct
/////////////////////////////////////////////////////////////////////

struct alx_parser_state {

//  struct alx_grammar const grammar;

  jmp_buf fatal_jmp_buf;

  struct alx_cmd_args const args;

  clock_t const start_clock;

  alx_offset final_index;

  struct alx_string out_json;

  struct alx_rng rng;

  struct alx_shadowed_inputs_cursor shadowed_inputs_cursor;

  size_t vector_count;
  struct alx_v32 * vector_list[64];

  struct alx_v32 input_unicode;
  struct alx_v32 shadowed_inputs;

  struct {
    struct alx_v32 vertex_index;
    struct alx_v32 cedges;
    struct alx_v32 cfinals;
    struct alx_v32 csiblings;
    struct alx_v32 cexclusions;
  } bottom_up;

  struct alx_collapse collapse;
  struct alx_graph graph;

  struct {
    struct alx_v32 index;
    struct alx_v32 edges;
  } all_edges;

  struct alx_random_match random;

  size_t prev_block_begin;

  struct alx_backtrace_state backtrace_state;
};

