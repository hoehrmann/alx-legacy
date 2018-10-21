#include "alx.h"

/////////////////////////////////////////////////////////////////////
// Put __LINE__ et al. into struct via macros.
/////////////////////////////////////////////////////////////////////

struct alx_log_context
alx_log_get_context(
  struct alx_parser_state const* const p,
  char const* const file,
  char const* const func,
  size_t const line
) {

  struct alx_log_context const context = {
    p,
    file,
    func,
    line
  };
  
  return context;

}

/////////////////////////////////////////////////////////////////////
// Log memory usage
/////////////////////////////////////////////////////////////////////

void
alx_log_memory_p(
  struct alx_log_context const ctx,
  struct alx_parser_state const* const p
) {

  // TODO: right align bytes 

  alx_logf_p(
    ctx,
    "mem use %-25s %zu bytes\n",
    "grammar (static)",
    sizeof(struct alx_grammar)
  );

  for ( size_t ix = 0; ix < p->vector_count; ++ix ) {

    alx_logf_p(
      ctx,
      "mem use %-25s %zu bytes\n",
      p->vector_list[ix]->native_name,
      sizeof(uint32_t) * alx_v32_size( p->vector_list[ix] )
    );

  }

}

/////////////////////////////////////////////////////////////////////
// Log various data structures
/////////////////////////////////////////////////////////////////////

void
alx_log_quad_p(
  struct alx_log_context const ctx,
  char const* const prefix,
  struct alx_vertexp_quad const* const quad
) {

  alx_logf_p(
    ctx,
    "%s %u %u %u %u %u %u %u %u\n",
    prefix,
    quad->src_pos,
    quad->src_vertex,
    quad->mid_src_pos,
    quad->mid_src_vertex,
    quad->mid_dst_pos,
    quad->mid_dst_vertex,
    quad->dst_pos,
    quad->dst_vertex
  );

}

void
alx_log_pair_p(
  struct alx_log_context const ctx,
  char const* const prefix,
  struct alx_vertexp_pair const* const pair
) {

  alx_logf_p(
    ctx,
    "%s %u %u %u %u\n",
    prefix,
    pair->src_pos,
    pair->src_vertex,
    pair->dst_pos,
    pair->dst_vertex
  );

}

void
alx_log_graph_node_p(
  struct alx_log_context const ctx,
  char const* const prefix,
  struct alx_graph_node const* const node
) {

  alx_logf_p(
    ctx,
    "%s pos %u vertex %u succ %u pred %u lm %u p %u\n",
    prefix,
    node->pos,
    node->vertex,
    node->successors,
    node->predecessors,
    node->lastmodified,
    node->pending
  );

}

/////////////////////////////////////////////////////////////////////
// Main logging function
/////////////////////////////////////////////////////////////////////

void
alx_logf_p(
  struct alx_log_context const ctx,
  ...
) {

  va_list args;
  va_start(args, ctx);

  char const* const base = strrchr(ctx.file, '/')
    ? strrchr(ctx.file, '/')
    : strrchr(ctx.file, '\\');

  //
  // 
  double const elapsed = ctx.p
    ? (double)(clock() - ctx.p->start_clock) / (double)CLOCKS_PER_SEC
    : 0;

  // TODO: indent lines after the first?

  fprintf(
    stderr,
    "t%08.0f %s (%s:%zu): ",
    elapsed * 1000,
    ctx.func,
    base ? base + 1 : ctx.file,
    ctx.line
  );

  char const* const fmt = va_arg(args, char const*);

  vfprintf(stderr, fmt, args);

  va_end(args);

}
