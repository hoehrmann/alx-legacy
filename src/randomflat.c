#include "alx.h"

/////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////

void
alx_random_path_compute_endpoints(
  struct alx_parser_state * const p,
  struct alx_v32 const* const path,
  struct alx_v32 * const stack,
  struct alx_v32 * const endpoints
) {

  // TODO: does it make sense to use vertex id here instead of 
  // 2-tuples?

  assert( alx_v32_size( path ) != 0 );

  //
  // In order to be able to print out the endpoint of a given
  // push vertex in a `path`, start and corresponding final
  // vertices have to be matched up directly. So this runs over
  // the path in reverse order with a stack and writes out the
  // symbol on top of the stack next to path elements in a
  // parallel array.

  alx_v32_clear( endpoints );
  alx_v32_clear( stack );

  // sentinel
  alx_v32_push_back2(
    stack,
    0,
    0
  );

  size_t endpoints_ix = (
    alx_v32_size(path)
  );

  alx_v32_zerofill( endpoints, endpoints_ix );

  for (
    size_t ix = alx_v32_size(path);
    /**/;
    ix -= 2
  ) {

    for (size_t ax = 1; ax < 3; ++ax) {

      // copy 2-tuple from stack to result
      alx_v32_set_unsafe(
        endpoints,
        --endpoints_ix,
        alx_v32_at(
          stack,
          alx_v32_size( stack ) - ax
        )
      );

    }

    struct alx_vertexp const tmp = {
      alx_v32_at( path, ix - 2 ),
      alx_v32_at( path, ix - 1 )
    };

    if ( alx_gv_is_pop( alx_g(p), tmp.vertex ) ) {

      alx_v32_push_back2(
        stack,
        tmp.pos,
        tmp.vertex
      );

    } else if ( alx_gv_is_push( alx_g(p), tmp.vertex ) ) {

      assert( alx_v32_size(stack) >= 2 );

      alx_vertex const popped_vertex = alx_v32_pop( stack );
      alx_offset const popped_pos = alx_v32_pop( stack );

      if ( alx_gv_partner(alx_g(p), tmp.vertex) != popped_vertex ) {
        assert( "corrupt stack\n" );
      }

    }

    if (endpoints_ix == 0)
      break;

  }

  // FIXME: something does not work here 
//  assert( alx_v32_size( stack ) == 2 );

  alx_v32_clear( stack );

}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_random_path_json(
  struct alx_parser_state * const p,
  struct alx_v32 const* const path,
  struct alx_v32 * const endpoints,
  struct alx_v32 * const json_stack,
  struct alx_string * const string
) {

  size_t path_ix = 0;
  size_t endp_ix = 0;
  int depth = 0;

  struct alx_vertexp prev = {
    0,
    0
  };

  alx_v32_clear( json_stack );

  alx_v32_push_back( json_stack, 0 );

  while (path_ix < alx_v32_size( path )) {

    struct alx_vertexp const lhs = {
      alx_v32_at( path, path_ix + 0 ),
      alx_v32_at( path, path_ix + 1 )
    };

    struct alx_vertexp const rhs = {
      alx_v32_at( endpoints, endp_ix + 0 ),
      alx_v32_at( endpoints, endp_ix + 1 )
    };

#if 0
    uint32_t const nth = alx_v32_at(
      endpoints,
      endp_ix + 2
    );
#endif

    uint32_t const nth = 0;

    if ( 0 && alx_gv_is_skippable(alx_g(p), lhs.vertex) ) {

      // ...

    } else if (alx_gv_is_push(alx_g(p), lhs.vertex)) {

      if (prev.pos && prev.pos != lhs.pos) {

        alx_string_append_xml_data(
          string,
          depth,
          "_",
          prev.pos,
          lhs.pos,
          false,
          alx_v32_get_last( json_stack )
        );

        alx_v32_inc_last( json_stack );

      }

      alx_string_append_xml_push(
        string,
        depth,
        // FIXME: must ensure name is escaped for JSON string
        alx_gv_name(alx_g(p), lhs.vertex),
        lhs.pos,
        rhs.pos,
        true,
        alx_v32_get_last( json_stack )
      );

#if 0
      fprintf(
        stderr,
        "]%s\n%*s[\"%s\", %u, %u, [",
        nth ? "" : ",",
        depth*2,
        "",
        alx_gv_name(alx_g(p), lhs.vertex),
        lhs.pos,
        rhs.pos
      );
#endif

      alx_v32_push_back( json_stack, 0 );

      depth += 1;
      prev = lhs;

    } else if (alx_gv_is_pop(alx_g(p), lhs.vertex)) {

      if (prev.pos != lhs.pos) {

        alx_string_append_xml_data(
          string,
          depth,
          "_",
          prev.pos,
          lhs.pos,
          false,
          alx_v32_get_last( json_stack )
        );

        alx_v32_inc_last( json_stack );

      }

      alx_string_append_xml_pop(
        string,
        depth,
        alx_gv_name(alx_g(p), lhs.vertex),
        true
      );

      alx_v32_pop( json_stack );
      alx_v32_inc_last( json_stack );

      depth -= 1;
      prev = lhs;

    } else {

      // omit

    }

    path_ix += 2;
    endp_ix += 2;

  }

  alx_string_json_finish( string );

  alx_v32_pop( json_stack );

}


void
alx_random_flat_path(
  struct alx_parser_state * const p,
  alx_vertex_index_ix const src_ix,
  alx_vertex_index_ix const dst_ix,
  struct alx_v32 const* const vertex_index,
  struct alx_v32 const* const sorted_edges,
  struct alx_v32 * const full_path
) {

  alx_vertex_index_ix end_ix = src_ix;

  while (true) {

    uint32_t const* const ptr = alx_v32_bsearch_random(
      sorted_edges,
      0,
      &end_ix,
      alx_v32_size( sorted_edges ) / 2,
      sizeof(*sorted_edges->d) * 2,
      alx_cmp_uint32_ts_0a,
      &p->rng
    );

    size_t const ptr_ix = alx_ptrdiff_t_to_size_t(
      ptr - sorted_edges->d
    );
   
    if ( ptr == NULL ) {

      alx_logf(
        p,
        "ptr is null, was looking for end_ix %u (src_ix %u dst_ix %u)\n",
        end_ix,
        src_ix,
        dst_ix
      );

      return;

    }

#if 0
    alx_logf( p, "appending %u,%u (skippable: %u)\n",
      alx_v32_at( vertex_index, alx_v32_at(sorted_edges, ptr_ix + 1) + 0 ),
      alx_v32_at( vertex_index, alx_v32_at(sorted_edges, ptr_ix + 1) + 1 ),
      alx_gv_is_skippable(alx_g(p), alx_v32_at( vertex_index, alx_v32_at(sorted_edges, ptr_ix + 1) + 1 ))
    );
#endif

    alx_vertex_index_ix const new_ix = 
      alx_v32_at(sorted_edges, ptr_ix + 1);

    alx_vertex const new_vertex = alx_v32_at(
      vertex_index, new_ix + 1 );

    bool const omit = alx_unordered_vertex_set_has(
      &p->args.random_omit,
      new_vertex
    );

    bool skippable = alx_gv_is_skippable(
      alx_g(p),
      new_vertex
    );

    if (!skippable) {

#if 0
      alx_logf(
        p,
        "not skippable (src_ix %u dst_ix %u end_ix %u new_ix %u)\n",
        src_ix,
        dst_ix,
        end_ix,
        new_ix
      );
#endif

      if (new_ix != dst_ix) {
        continue;
      }
    }

    if (!omit) {

#if 0
      alx_logf(
        p,
        "pb %u,%u\n",
        alx_v32_at( vertex_index, new_ix + 0 ),
        alx_v32_at( vertex_index, new_ix + 1 )
      );
#endif

      alx_v32_push_back2(
        full_path,
        alx_v32_at( vertex_index, new_ix + 0 ),
        alx_v32_at( vertex_index, new_ix + 1 )
      );

    }

    end_ix = new_ix;

    if (end_ix == dst_ix) {
#if 0
      alx_logf(
        p,
        "break\n"
      );
#endif
      break;
    }

  }

}

void
alx_random_full_path(
  struct alx_parser_state * const p,
  struct alx_graph const* const graph
) {

  // TODO: needs to be split into several functions
  // TODO: do not directly reference p->*, use args instead.

  alx_logf( p, "finding random quad path\n" );

  alx_random_quad_path(
    p,
    &p->graph,
    &p->random.sorted_quads,
    &p->random.quad_path
  );

  alx_shadowed_inputs_cursor_reset(
    &p->shadowed_inputs_cursor,
    &p->shadowed_inputs
  );

  struct alx_v32 const* const qp = 
    &p->random.quad_path;

  if (!alx_v32_size( qp )) {
    alx_logf( p, "qp is empty\n" );
    return;
  }

  struct alx_graph_node const start_node = 
    alx_graph_node_from_ix(
      graph, 
      alx_v32_at( qp, 0 )
     );

  alx_v32_push_back2(
    &p->random.full_path,
    start_node.pos,
    start_node.vertex
  );

  alx_logf( p, "filling gaps between quads with random paths\n" );

  for (
    size_t ix = 1;
    ix < alx_v32_size( qp );
    ++ix
  ) {

 return;

    alx_graph_node_ix const src_ix = 
      alx_v32_at( qp, ix - 1 );

    alx_graph_node_ix const dst_ix = 
      alx_v32_at( qp, ix - 0 );

    struct alx_graph_node const src_node = 
      alx_graph_node_from_ix( graph, src_ix );

    struct alx_graph_node const dst_node = 
      alx_graph_node_from_ix( graph, dst_ix );

    struct alx_vertexp_pair const bounds = {
      src_node.pos,
      src_node.vertex,
      dst_node.pos,
      dst_node.vertex
    };

    // TODO: the cursor does not have to be global, does it?

    alx_shadowed_inputs_cursor_move_to(
      &p->shadowed_inputs_cursor,
      &p->shadowed_inputs,
      dst_node.pos
    );

    alx_vertex_list_index const* m_ptr =
      &( p->shadowed_inputs.d[ p->shadowed_inputs_cursor.last_at ] );

    alx_v32_clear( &p->random.gap_edges );
    alx_v32_clear( &p->random.gap_index );

    struct alx_vertexp const src_point = {
      bounds.src_pos,
      bounds.src_vertex
    };

    struct alx_vertexp const dst_point = {
      bounds.dst_pos,
      bounds.dst_vertex
    };

    alx_backtrace(
      p,
      &p->random.gap_index,
      &p->random.gap_edges,
      &src_point,
      &dst_point,
      m_ptr,
      false
    );

#if 0
    alx_logf( p, "gap_edges size %zu\n", alx_v32_size( &p->random.gap_edges ) );
#endif

#if 1
    alx_v32_qsort_whole(
      &p->random.gap_edges,
      2,
      alx_cmp_uint32_ts_0a
    );
#endif

#if 0
    alx_log_pair( p, "flat_path ", &bounds );
#endif

    alx_vertex_index_ix gap_src_ix = 0;

    // TODO: make function for the code below

    for (
      alx_vertex_index_ix gap_ix = 2;
      gap_ix < alx_v32_size(&p->random.gap_index);
      gap_ix += 2
    ) {

      struct alx_vertexp const here = {
        alx_v32_at(&p->random.gap_index, gap_ix + 0),
        alx_v32_at(&p->random.gap_index, gap_ix + 1)
      };

      // TODO: alx_vertexp_pair_equals(&here, &bounds)

      if (
        here.pos == bounds.src_pos
        &&
        here.vertex == bounds.src_vertex
      ) {

        gap_src_ix = gap_ix;
        break;

      }

    }

#if 0
    alx_logf( p, "gap_src_ix %u\n", gap_src_ix );
#endif

#if 0
    struct alx_vertexp const dbgSrc = {
      alx_v32_at(&p->random.gap_index, gap_src_ix + 0),
      alx_v32_at(&p->random.gap_index, gap_src_ix + 1)
    };

    struct alx_vertexp const dbgDst = {
      alx_v32_at(&p->random.gap_index, alx_backtrace_index_final_ix(
        &p->random.gap_index
      ) + 0),
      alx_v32_at(&p->random.gap_index, alx_backtrace_index_final_ix(
        &p->random.gap_index
      ) + 1)
    };

    alx_logf(
      p,
      "about to call alx_random_flat_path(p, { %u,%u -> %u,%u } ...) gap_edges.size = %zu = %u,%u,... aka %u,%u,%u,%u\n",
      dbgSrc.pos,
      dbgSrc.vertex,
      dbgDst.pos,
      dbgDst.vertex,
      alx_v32_size(&p->random.gap_edges),
      alx_v32_at(&p->random.gap_edges, 0),
      alx_v32_at(&p->random.gap_edges, 1),
      alx_v32_at(&p->random.gap_index, alx_v32_at(&p->random.gap_edges, 0) + 0),
      alx_v32_at(&p->random.gap_index, alx_v32_at(&p->random.gap_edges, 0) + 1),
      alx_v32_at(&p->random.gap_index, alx_v32_at(&p->random.gap_edges, 1) + 0),
      alx_v32_at(&p->random.gap_index, alx_v32_at(&p->random.gap_edges, 1) + 1)
    );

    if (!gap_src_ix) {
      // alx_sqldump_dump_parser_state( p, "RNDBUG.sqlite" );
    }
#endif

    alx_random_flat_path(
      p,
      gap_src_ix,
      alx_backtrace_index_final_ix(
        &p->random.gap_index
      ),
      &p->random.gap_index,
      &p->random.gap_edges,
      &p->random.full_path
    );

  }

}
