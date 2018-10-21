#include "alx.h"

void
alx_process_file(
  struct alx_parser_state * const p
) {

  alx_utf8_state state = alx_g(p)->utf8_start_state;

  alx_v32_push_back(
    &p->shadowed_inputs,
    alx_g(p)->nfa_start_index
  );

  p->prev_block_begin = 0;

  FILE *f = fopen(p->args.in_path, "rb");

  if (!f) {
    alx_logf( p, "fopen(%s) failed\n", p->args.in_path );
    return;
  }

  alx_logf(p, "Beginning to read %s...\n", p->args.in_path);

  alx_v32_push_back( &p->input_unicode, UINT32_C(0xFFFFFFFF) );

  uint_fast32_t decoder_state = 0;
  uint_fast32_t decoder_value = 0;

  while (!feof(f) && !ferror(f)) {
    char buf[4096];

    size_t read = fread(buf, 1, 4096, f);

    // TODO: feof and ferror checks vs read size check

    for (size_t buf_ix = 0; buf_ix < read; ++buf_ix) {

      bool const got_unicode = !alx_utf8_decode(
        &decoder_state,
        &decoder_value,
        alx_char_to_uint8_t( buf[buf_ix] )
      );

      if (got_unicode) {

        alx_v32_push_back( &p->input_unicode, decoder_value );

      }

      if (state < alx_g(p)->utf8_start_state) {

        alx_process_one_input_class(
          p,
          alx_utf8_state_to_input_class_id(state)
        );

        p->final_index++;

      }

      state = alx_grammar_utf8_next_state(
        alx_g(p),
        state,
        alx_char_to_uint8_t( buf[buf_ix] )
      );

    }
  }

  alx_process_one_input_class(
    p,
    alx_utf8_state_to_input_class_id(state)
  );

  p->final_index++;

  fclose(f);
}

void
alx_cmd_print_help(
  char const* const program_name
) {

  fprintf(stderr, "\
  USAGE                                                        \n\
                                                               \n\
    %s --in=./input.data --out-random=./random_parse_tree.json \n\
                                                               \n\
  OPTIONS                                                      \n\
                                                               \n\
    --in=path                                                  \n\
                                                               \n\
        Path to UTF-8 encoded input document.                  \n\
                                                               \n\
    --random=path                                              \n\
                                                               \n\
        Compute a random parse tree and write it in JSON       \n\
        encoded form to this path.                             \n\
                                                               \n"
#if ALX_SQLITE
  "    --db=path                                               \n\
                                                               \n\
        Path for SQLite DB.                                    \n\
                                                               \n\
    --query=name                                               \n\
                                                               \n\
        Execute named query.                                   \n\
                                                               \n"
#endif

  "    --all-edges                                             \n\
                                                               \n\
        Include all edges in the DB.                           \n\
                                                               \n\
    --srand=uint (default: 0)                                  \n\
                                                               \n\
        The seed used when calling srand() once before making  \n\
        the first random choice.                               \n\
                                                               \n\
    --help                                                     \n\
                                                               \n\
        Prints help page.                                      \n\
                                                               \n\
  -------------------------------------------------------------\n\
                                                               \n\
    Useful queries:                                            \n\
                                                               \n\
      --query=all_edges.tail_edges.edges_as_dot                \n\
      --query=cedges.tail_edges.edges_as_dot -Dtail_length=100 \n\
      --query=grammar_edges.edges_as_dot                       \n\
      ...                                                      \n\
                                                               \n\
  -------------------------------------------------------------\n\
    Copyright (c) 2018 Bjoern Hoehrmann <bjoern@hoehrmann.de>  \n"
    , program_name
  );

}

void
alx_cmdline_add_omit(
  struct alx_cmd_args * const args,
  char const* const value
) {

  struct alx_grammar const *const g = alx_g( NULL );

  assert( g != NULL );

  // make vertices to be omitted based on comma-separated list of
  // non-terminal names in command line argument value `value`.

  for ( char const* start = value; true; ) {

    char * final = strchr( start, ',' );
    
    if ( final == NULL ) {
      final = strchr( start, '\0' );
    }

    for (
      alx_vertex v = alx_grammar_min_vertex(g);
      v <= alx_grammar_max_vertex(g);
      v += 1
    ) {

      char const* const name = alx_gv_name( g, v );

      if ( strlen(name) != final - start ) {
        continue;
      }

      if (strncmp( start, name, final - start ) != 0) {
        continue;
      }

      if ( !alx_gv_is_skippable( g, v ) ) {
        continue;
      }

#if 0
      alx_logf(
        NULL,
        "marking vertex %u named %s to be stripped\n",
        v,
        name
      );
#endif

      alx_unordered_vertex_set_insert(
        &args->random_omit,
        v
      );

    }

    if ( *final == '\0' ) {

      break;

    } else if ( *final == ',' ) {

      start = final + 1;

    }

  }

}

struct alx_cmd_args
alx_parse_cmd_args(
  int const argc,
  char const* const argv[]
) {

  struct alx_cmd_args args = {
    .help = false,
    .in_path = NULL,
    .random_path = NULL,
    .db_path = NULL,
    .query = NULL,
    .query_args = NULL,
    .all_edges = false,
    .seed = 0,
    .argc = argc,
    .argv = argv,
    .random_omit = { 0 }
  };

  // Parse command line parameters
  for (int ax = 1; ax < argc; ++ax) {

    char const* const s = argv[ax];

    // boolean options
    if (strcmp("--help", s) == 0) {

      args.help = true;
      continue;

    } else if (strcmp("--all-edges", s) == 0) {

      args.all_edges = true;
      continue;

    }

    char const* value = strchr(s, '=');

    if (value == NULL) {

      alx_logf( NULL, "unknown command line argument %s\n", s );
      args.help = true;
      break;

    }

    size_t const nlen = value++ - s;

    // -Dkey=value for query processing
    if ( alx_starts_with(s, "-D") ) {

      // processed later?
      continue;

    }

    // --name=value options
    if (strncmp("--in=", s, nlen) == 0) {

      args.in_path = value;

    } else if (strncmp("--random=", s, nlen) == 0) {

      args.random_path = value;

    } else if (strncmp("--db=", s, nlen) == 0) {

      args.db_path = value;

    } else if (strncmp("--query=", s, nlen) == 0) {

      args.query = value;

    } else if (strncmp("--query-args=", s, nlen) == 0) {

      args.query_args = value;

    } else if (strncmp("--srand=", s, nlen) == 0) {

      // TODO: maybe error handling?
      sscanf( value, "%u", &args.seed );
      srand(args.seed);

    } else if (strncmp("--omit=", s, nlen) == 0) {

      alx_cmdline_add_omit(&args, value);

    } else {

      alx_logf( NULL, "unknown command line argument %s\n", s );
      args.help = true;

    }

  }

  return args;
}

void
alx_cmdline_release(
  struct alx_parser_state * const p
) {

  ///////////////////////////////////////////////////////////////////
  // close and free everything
  ///////////////////////////////////////////////////////////////////

  alx_logf( p, "Cleanup resources\n" );

  alx_string_free(&p->out_json);

  for (size_t ix = 0; ix < p->vector_count; ++ix) {

    alx_v32_free( p->vector_list[ix] );

  }

  alx_set_destroy(p->graph.set);

  alx_logf(p, "Done.\n");

}

void
alx_cmdline_query(
  struct alx_parser_state * const p,
  struct alx_cmd_args const* const args
) {

  alx_sqlquery_something( p, args->db_path );

}

void
alx_cmdline_generate_random(
  struct alx_parser_state * const p,
  struct alx_cmd_args const* const args
) {

  alx_logf( p, "finding random match\n" );

  p->backtrace_state.predecessors = &alx_g(p)->vertex_old_predecessors[0];
  p->backtrace_state.shadows = &alx_g(p)->vertex_shadows[0];

  alx_random_full_path(
    p,
    &p->graph
  );

  alx_logf( p, "traversing path to match up non-terminals\n" );

  // FIXME: segfaults if not a match

  alx_random_path_compute_endpoints(
    p,
    &p->random.full_path,
    &p->random.stack,
    &p->random.endpoints
  );

  alx_logf( p, "serialising data as JSON tree\n" );

  alx_random_path_json(
    p, 
    &p->random.full_path,
    &p->random.endpoints,
    &p->random.json_stack,
    &p->out_json
  );

  alx_logf(
    p,
    "done finding random match, writing to %s\n",
    args->random_path
  );

  FILE* const f = fopen(args->random_path, "wb");

  if (f) {

    if (p->out_json.size) {
      fwrite(p->out_json.data + 1, p->out_json.size - 1, 1, f);
    }

    fclose(f);

  } else {

    alx_logf( p, "failed to open %s\n", args->random_path );

  }

}

void
alx_cmdline_main_backtrace(
  struct alx_parser_state * const p,
  struct alx_cmd_args const* const args,
  struct alx_vertexp const* const src_point,
  struct alx_vertexp const* const dst_point  
) {

  alx_backtrace(
    p,
    &p->bottom_up.vertex_index,
    &p->bottom_up.cedges,
    src_point,
    dst_point,
    alx_v32_last_ptr(&p->shadowed_inputs),
    true
  );

  alx_logf(
    p,
    "finished backtrace, edge count %zu\n",
    alx_v32_size(&p->bottom_up.cedges) / 2
  );

  if ( !alx_v32_size(&p->bottom_up.cedges) ) {

    alx_logf(
      p,
      "input does not match\n"
    );

  }

}

void
alx_cmdline_all_edges_backtrace(
  struct alx_parser_state * const p,
  struct alx_cmd_args const* const args,
  struct alx_vertexp const* const src_point,
  struct alx_vertexp const* const dst_point  
) {

  if ( !args->all_edges ) {
    return;
  }

  // TODO: do this in a more principled manner
  p->backtrace_state.predecessors = &alx_g(p)->vertex_old_predecessors[0];
  p->backtrace_state.shadows = &alx_g(p)->vertex_shadows[0];

  if ( alx_v32_size( &p->bottom_up.cedges ) ) {

    alx_logf( p, "full backtrace...\n" );

    alx_backtrace(
      p,
      &p->all_edges.index,
      &p->all_edges.edges,
      src_point,
      dst_point,
      alx_v32_last_ptr(&p->shadowed_inputs),
      true
    );

    alx_logf( p, "done\n" );

  } else {

    alx_backtrace_from_failure(
      p,
      &p->all_edges.index,
      &p->all_edges.edges
    );

    alx_logf(
      p,
      "finished backtrace_from_failure, edge count %zu\n",
      alx_v32_size(&p->bottom_up.cedges) / 2
    );
    
  }

}

void
alx_cmdline_parse(
  struct alx_parser_state * const p,
  struct alx_cmd_args const* const args
) {

  ///////////////////////////////////////////////////////////////////
  // Read file UTF-8 bytes and report NFA transitions
  ///////////////////////////////////////////////////////////////////

  alx_process_file(p);

  alx_logf(
    p,
    "finished forward pass, final_index %" PRIu32 "\n",
    p->final_index
  );

  ///////////////////////////////////////////////////////////////////
  // Backtrace
  ///////////////////////////////////////////////////////////////////

  struct alx_vertexp_pair const pair = alx_ps_get_endpoints( p );

  // There should be logic here, if the grammar's final vertex has
  // not been reached, backtrace from all vertices at the final
  // index. That would populate the relevant data structures to aid
  // in debugging why there was no match. Needs code to find non-
  // input vertices at last position in shadowed_inputs that has 
  // any vertices at all. Then pass a vector to backtrace instead.

  struct alx_vertexp const src_point = {
    pair.src_pos,
    pair.src_vertex
  };

  struct alx_vertexp const dst_point = {
    pair.dst_pos,
    pair.dst_vertex
  };

  alx_cmdline_main_backtrace(
    p,
    args,
    &src_point,
    &dst_point
  );

  ///////////////////////////////////////////////////////////////////
  // Collapse edges and build graph bottom-up.
  ///////////////////////////////////////////////////////////////////

  alx_bottom_up(
    p,
    &p->graph
  );

  bool found_matches = alx_log_matches_graph(
    p,
    &p->graph
  );

  ///////////////////////////////////////////////////////////////////
  // JSON output
  ///////////////////////////////////////////////////////////////////

  if (args->random_path != NULL && found_matches) {

    alx_cmdline_generate_random( p, args );

  }

  ///////////////////////////////////////////////////////////////////
  // Backtrace for all_edges
  ///////////////////////////////////////////////////////////////////

  // fixme: check cmdline args here instead of inside the function
  alx_cmdline_all_edges_backtrace(
    p,
    args,
    &src_point,
    &dst_point
  );

  ///////////////////////////////////////////////////////////////////
  // SQLite output
  ///////////////////////////////////////////////////////////////////

#if ALX_SQLITE

  if (args->db_path != NULL) {

    alx_logf( p, "dump to sqlite\n" );
    alx_sqldump_dump_parser_state( p, args->db_path );
    alx_logf( p, "done dump to sqlite\n" );

  }

#endif

}

bool
alx_cmdline_bad_args(
  struct alx_cmd_args const* const args
) {

  if (args->help || (!args->in_path && !args->query)) {
    return true;
  }

  if (args->in_path && args->query) {
    return true;
  }

  if (args->query && !args->db_path) {
    return true;
  }

  return false;

}

int
main(
  int const argc,
  char const* const argv[]
) {

  ///////////////////////////////////////////////////////////////////
  // Validate internal structures
  ///////////////////////////////////////////////////////////////////

  alx_validate_types();

  ///////////////////////////////////////////////////////////////////
  // Command line argument processing
  ///////////////////////////////////////////////////////////////////

  struct alx_cmd_args const args = alx_parse_cmd_args(argc, argv);

  if (alx_cmdline_bad_args( &args )) {

    alx_cmd_print_help(argv[0]);

    exit(0);

  }

  ///////////////////////////////////////////////////////////////////
  // Main data structure initialisation
  ///////////////////////////////////////////////////////////////////

  struct alx_parser_state p = {

//    .grammar = global_grammar,

    .fatal_jmp_buf = {
      // Currently unused.
      0
    },

    .args = args,

    .start_clock = clock(),

    .final_index = 1,

    .out_json = alx_string_new(&p),

    .rng = {
      0
    },

    .shadowed_inputs_cursor = {
      0
    },

    // This is written to before initialisation of the struct
    // is complete, but that for all I can tell, that is okay.
    .vector_count = 0,
    .vector_list = {
      0
    },

    .input_unicode = alx_v32_new(
      &p,
      &p.input_unicode,
      "input_unicode",
      "input_unicode",
      "?,NULLIF(?,0xFFFFFFFF)"
    ),

    .shadowed_inputs = alx_v32_new(
      &p,
      &p.shadowed_inputs,
      "shadowed_inputs",
      NULL,
      NULL
    ),

    .bottom_up = {
      .vertex_index = alx_v32_new(
        &p,
        &p.bottom_up.vertex_index,
        "bottom_up.vertex_index",
        "vertex_index",
        "?,NULLIF(?,0),NULLIF(?,0)"
      ),
      .cedges = alx_v32_new(
        &p,
        &p.bottom_up.cedges,
        "bottom_up.cedges",
        "cedges",
        "?,NULLIF(?/2,0),NULLIF(?/2,0)"
      ),
      .cfinals = alx_v32_new(
        &p,
        &p.bottom_up.cfinals,
        "bottom_up.cfinals",
        "cfinals",
        "?,NULLIF(?/2,0),NULLIF(?/2,0)"
      ),
      .csiblings = alx_v32_new(
        &p,
        &p.bottom_up.csiblings,
        "bottom_up.csiblings",
        "csiblings",
        "?,?/2,?/2,?,?"
      ),
      .cexclusions = alx_v32_new(
        &p,
        &p.bottom_up.cexclusions,
        "bottom_up.cexclusions",
        "cexclusions",
        "?,?/2,?/2,?,?"
      )
    },

    .collapse = {
      .p = &p,
      .vertices = alx_v32_new(
        &p,
        &p.collapse.vertices,
        "collapse.vertices",
        "collapse_vertices",
        "?,?,?"
      ),
      .lastround = alx_v32_new(
        &p,
        &p.collapse.lastround,
        "collapse.lastround",
        "collapse_lastround",
        "?,?"
      ),
      .position = alx_v32_new(
        &p,
        &p.collapse.position,
        "collapse.position",
        "collapse_position",
        "?,?"
      ),
      .edges = alx_v32_new(
        &p,
        &p.collapse.edges,
        "collapse.edges",
        "collapse_edges",
        "?,?/2,?/2"
      ),
      .lhs_edges = alx_v32_new(
        &p,
        &p.collapse.lhs_edges,
        "collapse.lhs_edges",
        "collapse_lhs_edges",
        "?,?/2,?/2"
      ),
      .rhs_edges = alx_v32_new(
        &p,
        &p.collapse.rhs_edges,
        "collapse.rhs_edges",
        "collapse_rhs_edges",
        "?,?/2,?/2"
      ),
      .node_index = alx_v32_new(
        &p,
        &p.collapse.node_index,
        "collapse.node_index",
        "collapse_node_index",
        "?,?"
      )
    },

    .graph = {
      .p = &p,
      .nodes = alx_v32_new(
        &p,
        &p.graph.nodes,
        "graph.nodes",
        "graph_nodes",
        "?,NULLIF(?,0),NULLIF(?,0)"
      ),
      .links = alx_v32_new(
        &p,
        &p.graph.links,
        "graph.links",
        "graph_links",
        "?,?/4,?/2"
      ),
      .quads = alx_v32_new(
        &p,
        &p.graph.quads,
        "graph.quads",
        "graph_quads",
        "?,NULLIF(?/2,0),NULLIF(?/2,0),NULLIF(?/2,0),NULLIF(?/2,0)"
      ),
      .old2new = alx_v32_new(
        &p,
        &p.graph.old2new,
        "graph.old2new",
        "graph_old2new",
        "?,NULLIF(?,0)"
      ),
      .predecessors = alx_v32_new(
        &p,
        &p.graph.predecessors,
        "graph.predecessors",
        "graph_predecessors",
        "?,?/2"
      ),
      .successors = alx_v32_new(
        &p,
        &p.graph.successors,
        "graph.successors",
        "graph_successors",
        "?,?/2"
      ),
      .lastmodified = alx_v32_new(
        &p,
        &p.graph.lastmodified,
        "graph.lastmodified",
        "graph_lastmodified",
        "?,?"
      ),
      .pending = alx_v32_new(
        &p,
        &p.graph.pending,
        "graph.pending",
        "graph_pending",
        "?,?"
      ),
      .set = alx_set_create(
        &p
      )
    },

    .all_edges = {
      .index = alx_v32_new(
        &p,
        &p.all_edges.index,
        "all_edges.index",
        "all_edges_index",
        "?,NULLIF(?,0),NULLIF(?,0)"
      ),
      .edges = alx_v32_new(
        &p,
        &p.all_edges.edges,
        "all_edges.edges",
        "all_edges_edges",
        "?,NULLIF(?/2,0),NULLIF(?/2,0)"
      )
    },

    .random = {
      .sorted_quads = alx_v32_new(
        &p,
        &p.random.sorted_quads,
        "random.sorted_quads",
        "random_sorted_quads",
        "?,NULLIF(?/2,0),NULLIF(?/2,0),NULLIF(?/2,0),NULLIF(?/2,0)"
      ),
      .quad_path = alx_v32_new(
        &p,
        &p.random.quad_path,
        "random.quad_path",
        "random_quad_path",
        "?,?/2"
      ),
      .quad_stack = alx_v32_new(
        &p,
        &p.random.quad_stack,
        "random.quad_stack",
        "random_quad_stack",
        "?,NULLIF(?/2,0),NULLIF(?/2,0)"
      ),
      .gap_index = alx_v32_new(
        &p,
        &p.random.gap_index,
        "random.gap_index",
        "random_gap_index",
        "?,NULLIF(?,0),NULLIF(?,0)"
      ),
      .gap_edges = alx_v32_new(
        &p,
        &p.random.gap_edges,
        "random.gap_edges",
        "random_gap_edges",
        "?,?/2,?/2"
      ),
      .full_path = alx_v32_new(
        &p,
        &p.random.full_path,
        "random.full_path",
        "random_full_path",
        "?,?,?"
      ),
      .stack = alx_v32_new(
        &p,
        &p.random.stack,
        "random.stack",
        "random_stack",
        "?,?,?,?"
      ),
      .endpoints = alx_v32_new(
        &p,
        &p.random.endpoints,
        "random.endpoints",
        "random_endpoints",
        "?,NULLIF(?,0),NULLIF(?,0)"
      ),
      .json_stack = alx_v32_new(
        &p,
        &p.random.json_stack,
        "random.json_stack",
        "random_json_stack",
        "?,?"
      )
    },

    .prev_block_begin = 0,

    // TODO: all below backtrace

    .backtrace_state = {

      // TODO: this is not great
      .vprev = &p.backtrace_state.vprev_,
      .vnext = &p.backtrace_state.vnext_,
      .nexttodo = &p.backtrace_state.nexttodo_,
      .prevtodo = &p.backtrace_state.prevtodo_,

#if 1
      .vprev_ = { 0 },
      .vnext_ = { 0 },
      .nexttodo_ = { 0 },
      .prevtodo_ = { 0 },
#endif

#if 0
      .predecessors = &alx_g(&p)->vertex_old_predecessors[0],
      .shadows = &alx_g(&p)->vertex_shadows[0],
#else
      .predecessors = &alx_g(&p)->vertex_unskippable_predecessors[0],
      .shadows = &alx_g(&p)->vertex_unskippable_shadows[0],
#endif

    }

  };

  if (args.in_path) {

    alx_cmdline_parse( &p, &args );

  } else if (args.query) {

    alx_cmdline_query( &p, &args );

  }

  alx_cmdline_release( &p );

  return 0;

}

/*

QUERY ARGS CANDIDATES:

  * tail_length instead of 100 hardcoded
  * max_edges instead of limit by offset
  * 

  -Dtail_length=1212

WINDOWS BUILD NOTES

  % perl -e 'local $/; local $_ = <>; s/\{\s*\}/{0}/gs; s/,\s*\}/}/gs; print $_' /c/Users/bjoern/home/parselov/alx/src/grammar.c > /c/Users/bjoern/alx/alx/gitlab/alx/src/grammar.c ; cp /c/Users/bjoern/home/parselov/alx/src/grammar.h /c/Users/bjoern/alx/alx/gitlab/alx/src/grammar.h
  % bash -x alx/alx/gitlab/alx/src/generate-files.sh && devenv alx/alx.sln /Build
  % bash -x alx/alx/gitlab/alx/src/generate-files.sh && msbuild alx/alx.sln /p:Configuration="Release" /p:Platform="x64"

NEW TODO:

  [ ] The graph_quads and all_edges structures can contain data that
      cannot be part of a random match, e.g. because the bottom-up
      process generated some data that ultimately failed. It would be
      nice to be able to remove such data.
      
      It would be much easier to do this in SQL rather than C and at
      the moment there is little reason to do it in C.

      A first step would be view_reachable_quad VIEW that walks the
      quad structure top-down collecting all quads reached. Quads not
      covered by the VIEW could then be removed.

      This would also give us a set of unskippable vertices that can
      stay in `all_edges` and, conversely, a set of vertices that can
      be removed from all_edges (and all similar data structures if 
      so desired).

      The problem then is cleaning up all_edges, or generating it in
      a way that omits ultimately unsuccessful vertices. This should 
      not be difficult to do with even more control logic during the 
      backtrace step, but adding that is not attractive for now.

      That leaves doing it as SQL graph traversal ... or a separate 
      function in C code. Identifying reachable quads in C should be
      fairly easy, and reachable vertices based on that too, mainly
      because now we have a vertex index.

      So... in the interest of making progress, let's prototype this
      in SQL, and see how that goes.

      Okay...

      So the above is partly done. view_graph_quads_reachable_nodes
      implements one part. Missing is the second part (apply this
      data to backtrace all_edges by removing bad vertices and then
      all vertices that have become unreachable as a result).

  [ ] scrub unreachable quads?
        * find root quads -> todo
        * while todo
            * pop
            * mark here
            * add successors (not yet seen?) to todo
        * remove quads not marked

  [ ] Unify alx_collapse_node_ix and alx_graph_node_ix

  [ ] rename c* arrays 
  [ ] rename old_edge

  [x] add column parser_state.start_index=1 for symmetry reasons and
      fix view_graph_quads_roots to use that.

  [x] view_graph_quads: rename type to rel

  [x] make random path generation non-recursive?

  [ ] Would be nice to have some visualisation of where bottom-up
      process failed

  [ ] bottom_up_state thing

  [ ] better error handling in various places (file not found, no match)

  [ ] store last_successful_pos which is final_index in case of 
      (obvious?) match and less than that in case of early match
      failure

  [ ] Some kind of name field for grammars

  [ ] Make check function to validate graph_find behavior.
        * dst +/- 1 prev_dst

  [ ] drop alx_graph_quads_by_any in favour of iterating over 
      quads directly?

  [ ] Is qsort before collapse needed? Make check function?

  [ ] see how badly broken mega and no-dfa are these days

  [ ] Command line option to select additional non-terminals 
      that are considered non-skippable and thus end up in the
      quads data structure.

  [ ] What's up with exclusions sort?

  [ ] It is very unlikely that it is possible that vprev 
      nexttodo same vertex unorderly blah ...


  [x] fix the strcmp vs strncmp bug?

  [x] restructure parser_state children (code hygiene)
  [x] Refactor some logging code to use data structure dumper subs
  [x] Rename everything uint32_vector to v32
  [x] Avoid code duplication in log_memory
  [x] address cast/type conversion TODOs and FIXMEs 
  [x] Rename everything _grammar_vertex_ _gv_
  [x] add assertion on stack item mismatch during random tree gen
  [x] implement graph_has_exteriors function to avoid redundancy

  [x] Implement graph_find function that takes pos,vertex,ref_ix 
  [x] Use that in handle_if, omit_quad, log_matches
  [x] re-implement "unskippable" data (old code seems broken)
  [x] Finish moving random match generation to index based functions

  [x] backtrace_index_final_ix() = 4 and use throughout
  [x] remove has_discontinuity optimisation
  [x] global table for frees?
  [x] merge those fore log_memory, sqldump, free?
  [x] remove shortcut code
  [x] look through code for utility functions to implement

  [x] put vector meta data on the vectors?
  [x] reorganise everything into more files
  [x] repair directory structure, .git* files, etc.
  [x] push to gitlab privat repository (ditch old history)
  [x] Implement graph rewriting function based on omit_quad

  [x] cmake stuff for emcc
  [x] more checks for struct sizes
  [x] make log_memory log from perspective of caller
  [x] Properly cleanup sqlite resources
  [x] backtrace_state struct
  [x] Emscripten licensing blah

  [/] Finally fix that longjmp jumps into nowhere


TODO:

  [ ] Is it possible to land on final without having gone over the 
      corresponding start? In any case, bugfix for recursiveness...
  

  [x] Prioritise existing TODO: and FIXME: comments
  [x] Generate CBOR object with named keys for parse result parts
  [x] Print out minimal grammar data to parse result data
  [x] Command line parameters (naming is hard, etc.)
  [x] Rename that cxx_ function that's not cxx anymore
  [x] Write --help documentation sufficient to generate random trees
  [x] Make generation of certain data optional (see cmdline params)

  [x] JS implementation of random tree generation
  [x] Make a static string index in the grammar data 
  [x] Use that for all static strings
  [x] pre-compute CBOR representation of those strings

  [x] Make a tool that post-processes parse trees, option to rewrite
      single-child nodes from ["foo", ["bar", [ ... into ["foo/bar",
      [ ... and option to elide certain named nodes (for debugging)

  [x] Logging (if that were not a timesink)
  [x] remove causes for -Wgnu-empty-initializer warnings
  [/] For random trees, omit anonymous elements (#exclusion etc)?
  [/] For random trees, vertex IDs instead of strcpy'd vertex names?

  [x] investigate regular ordered_choice/exclusion fuckup
  [x] filter if2 in ordered_choices where if1 is a match
  [x] do that ^ for irregular and regular parts (?)
  [x] finish code for irregular "A but not B"
  [x] anything to do for *regular* "A but not B"?
  [x] Check test suite cleanup stuff, find what is really needed in C

  [ ] Make it possible to purge largest append-only structure
      no

  [ ] How to handle grammars that produce zero-size grammar data arrays?

  [ ] pre <ref> expansion graph rewrite optimisation for `value`

  [ ] make reftest data such that it does not matter which vertices
      are skippable
      -> probably not 

  [x] remove C++ data structures
  [x] ... and then implement handle_if
  [x] implement string builder
  [x] rename local push_back4
  [x] make sbp test things
  [x] wrappers for bsearch/qsort on uint32_vectors
  [x] rename foo2 -> foo throughout
  [x] a_u_v_s* to .h file
  [x] beef up main() with cmd arg handling
  [x] move heads/finals code into function
  [x] move relevant code into a_quads_new function (sorta)
  [x] replace a_rng implementation with srand/rand
  [x] properly do filtering in a_backtrace usage #2
  [x] generate grammar data with fixed size arrays
  [x] add assert( is_sorted ) where useful
  [x] avoid making two vsnprintf calls in a_string_appendf
  [x] put appendf calls into separate functions for reasons
  [x] compile only once per test series
  [x] move all vectors onto parser context (or registry?)?
  [x] rename comparison functions to indicate if they to asc or desc
  [x] do not generate redundant vector for forest
  [x] change API for output generation to start_array/end_array etc.
  [x] shadowed_inputs: use msb to indicate end of list, not 4 null bytes
  [x] heeve q onto p_s
  [x] avoid redundant vertex lists entries

  [/] better alx_..._bsearch/qsort parameter handling?

  [ ] test suite integration for randomised matches?

  [/] filter if2 in ordered_choices where if1 is a match
  [ ] keep track of min_distance for exclusion handling
      (Not sure this is still relevant)

alx_logf(cx, "debug", "%format%", ...)

realloc, free,
memcpy, memcmp, memset,
strchr, strlen,
bsearch, qsort, 
fprintf, ferror, feof, fopen, fclose, fwrite

NEW TODO:

  * Check that for skippable vertices in the grammar graph it is not
    possible to go from one vertex back to itself without crossing a
    non-skippable vertex in a way that would violate nesting. If it
    is not possible, then random tree generation does not need to 
    check for duplicates.

    (Not sure this is still current ^)

first time use of https://artybollocks.com/generator.html:

My work explores the relationship between Pre-raphaelite tenets and
skateboard ethics. With influences as diverse as Derrida and Roy
Lichtenstein, new synergies are synthesised from both orderly and
random structures.

Ever since I was a student I have been fascinated by the unrelenting
divergence of the mind. What starts out as yearning soon becomes
finessed into a tragedy of greed, leaving only a sense of nihilism
and the dawn of a new order.

As shifting derivatives become distorted through emergent and diverse
practice, the viewer is left with an insight into the edges of our
culture. 

*/


