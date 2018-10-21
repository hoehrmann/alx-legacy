#include "alx.h"

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_sqldump_dump_file_contents(
  struct alx_parser_state const* const p,
  sqlite3 * const db
) {

  // TODO: remove, obsolete

  FILE *f = fopen(p->args.in_path, "rb");

  if (!f) {
    alx_logf( p, "fopen(%s) failed\n", p->args.in_path );
    return;
  }

  sqlite3_str * str = sqlite3_str_new( db );

  while (true) {

    char buf[4096];

    size_t read = fread(buf, 1, 4096, f);

    sqlite3_str_append(str, buf, alx_size_t_to_int(read));

    if (read < 4096) {
      break;
    }

  }

  sqlite3_stmt * sth;

  int rc = sqlite3_prepare_v2(
    db,
    "CREATE TABLE input_data AS SELECT ? AS utf8",
    -1,
    &sth,
    NULL
  );

  alx_sqldump_check_rc( p, db, sth, rc, SQLITE_OK );

  // bind whole file contents as single parameter
  rc = sqlite3_bind_text64(
    sth,
    1,
    sqlite3_str_value( str ),
    sqlite3_str_length( str ),
    SQLITE_STATIC,
    SQLITE_UTF8
  );

  alx_sqldump_check_rc( p, db, sth, rc, SQLITE_OK );

  alx_logf(
    p,
    "Dumping re-read %s to input_data (%zu bytes)\n",
    p->args.in_path,
    sqlite3_str_length( str )
  );

  rc = sqlite3_step( sth );

  alx_sqldump_check_rc( p, db, sth, rc, SQLITE_DONE );

  sqlite3_free( sqlite3_str_finish( str ) );

  sqlite3_finalize( sth );

}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_sqldump_check_rc_p(
  struct alx_log_context const ctx,
  struct alx_parser_state const* const p,
  sqlite3 * const db,
  sqlite3_stmt * const sth,
  int const got,
  int const expected
) {

  if (got != expected) {

    alx_logf_p(
      ctx,
      "bad sqlite3 rc %u (expected %u): %s\n",
      got,
      expected,
      sqlite3_errmsg(db)
    );

    if (sth) {
      char * const expanded = sqlite3_expanded_sql(sth);
      alx_logf_p(
        ctx,
        "expanded sql: %s\n",
        expanded
      );
      sqlite3_free(expanded);
    }

    // This should be sufficient to release all sqlite3 resources
    // provided there is no code that uses more than one stmt.

    sqlite3_finalize( sth );
    sqlite3_close( db );

    alx_ps_longjmp(p, 3);

  }

}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_sqldump_insert_vector(
  struct alx_parser_state const* const p,
  sqlite3 * const db,
  sqlite3_stmt * const sth,
  struct alx_v32 const* const vector,
  size_t const start_index,
  size_t const size,
  size_t const items_per_row
) {

  // NOTE: size is handled regardless of start_index

  assert( size % items_per_row == 0 );
  assert( start_index % items_per_row == 0 );

  uint64_t row_id = start_index / items_per_row;

  for (
    size_t ix = start_index;
    ix < size;
    ix += items_per_row, row_id += 1
  ) {

    int rc = sqlite3_bind_int64(
      sth,
      1,
      row_id
    );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

    for (size_t ax = 0; ax < items_per_row; ++ax) {

      uint32_t const value = alx_v32_at( vector, ix + ax );

      rc = sqlite3_bind_int64(
        sth,
        alx_size_t_to_int( ax + 2 ),
        alx_uint32_t_to_int64_t( value )
      );

      alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

    }

    rc = sqlite3_step( sth );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_DONE);

    rc = sqlite3_reset( sth );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

  }

}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_sqldump_dump_parser_properties(
  struct alx_parser_state const* const p,
  sqlite3 * db
) {

  alx_logf( p, "dumping parser_state properties\n" );

  sqlite3_stmt * sth;

  int rc = sqlite3_prepare_v2(
    db,
    "INSERT INTO parser_state("
    "  input_path,"
    "  start_index,"
    "  final_index"
    ") VALUES ("
    "  ?,"
    "  ?,"
    "  ?"
    ")",
    -1,
    &sth,
    NULL
  );

  alx_sqldump_check_rc( p, db, sth, rc, SQLITE_OK );

  // input_path
  rc = sqlite3_bind_text(
    sth,
    1,
    p->args.in_path,
    -1,
    SQLITE_STATIC
  );

  alx_sqldump_check_rc( p, db, sth, rc, SQLITE_OK );

  // start_index
  rc = sqlite3_bind_int64(
    sth,
    2,
    1
  );

  // final_index
  rc = sqlite3_bind_int64(
    sth,
    3,
    p->final_index
  );

  alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

  rc = sqlite3_step( sth );

  alx_sqldump_check_rc(p, db, sth, rc, SQLITE_DONE);

  rc = sqlite3_finalize( sth );

  alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_sqldump_dump_vertex_properties(
  struct alx_parser_state const* const p,
  sqlite3 * db
) {

  alx_logf( p, "dumping vertex_properties\n" );

  sqlite3_stmt * sth;

  int rc = sqlite3_prepare_v2(
    db,
    "INSERT INTO vertex_property("
    "  vertex,"
    "  type,"
    "  name,"
    "  p1,"
    "  p2,"
    "  partner,"
    "  self_loop,"
    "  contents_self_loop,"
    "  topo,"
    "  skippable,"
    "  is_push,"
    "  is_pop"
    ") VALUES ("
    "  ?,"            // vertex
    "  ?,"            // type
    "  ?,"            // name
    "  NULLIF(?, 0)," // p1
    "  NULLIF(?, 0)," // p2
    "  NULLIF(?, 0)," // partner
    "  ?,"            // self_loop
    "  ?,"            // contents_self_loop
    "  NULLIF(?, 0)," // topo
    "  ?,"            // skippable
    "  ?,"            // is_push
    "  ?"             // is_pop
    ")",
    -1,
    &sth,
    NULL
  );

  alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

  for (
    alx_vertex v = alx_grammar_min_vertex(alx_g(p));
    v <= alx_grammar_max_vertex(alx_g(p));
    ++v
  ) {

    //
    // bind variables

    // vertex
    rc = sqlite3_bind_int64(
      sth,
      1,
      v
    );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

    // type
    rc = sqlite3_bind_text(
      sth,
      2,
      alx_gv_type(alx_g(p), v),
      -1,
      SQLITE_STATIC
    );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

    // name
    rc = sqlite3_bind_text(
      sth,
      3,
      alx_gv_name(alx_g(p), v),
      -1,
      SQLITE_STATIC
    );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

    // p1
    rc = sqlite3_bind_int64(
      sth,
      4,
      alx_gv_p1(alx_g(p), v)
    );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

    // p2
    rc = sqlite3_bind_int64(
      sth,
      5,
      alx_gv_p2(alx_g(p), v)
    );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

    // partner
    rc = sqlite3_bind_int64(
      sth,
      6,
      alx_gv_partner(alx_g(p), v)
    );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

    // self_loop
    rc = sqlite3_bind_text(
      sth,
      7,
      alx_gv_self_loop(alx_g(p), v),
      -1,
      SQLITE_STATIC
    );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

    // contents_self_loop
    rc = sqlite3_bind_text(
      sth,
      8,
      alx_gv_contents_self_loop(alx_g(p), v),
      -1,
      SQLITE_STATIC
    );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

    // topo
    rc = sqlite3_bind_int64(
      sth,
      9,
      alx_gv_topo(alx_g(p), v)
    );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

    // skippable
    rc = sqlite3_bind_int64(
      sth,
      10,
      alx_gv_is_skippable(alx_g(p), v)
    );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

    // is_push
    rc = sqlite3_bind_int64(
      sth,
      11,
      alx_gv_is_push(alx_g(p), v)
    );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

    // is_pop
    rc = sqlite3_bind_int64(
      sth,
      12,
      alx_gv_is_pop(alx_g(p), v)
    );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

    //
    // Execute

    rc = sqlite3_step( sth );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_DONE);

    rc = sqlite3_reset( sth );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

  }

  rc = sqlite3_finalize( sth );

  alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

}

void
alx_sqldump_dump_grammar_properties(
  struct alx_parser_state const* const p,
  sqlite3 * db
) {

  alx_logf( p, "dumping grammar_properties\n" );

  sqlite3_stmt * sth;

  int rc = sqlite3_prepare_v2(
    db,
    "INSERT INTO grammar("
    "  start_vertex,"
    "  final_vertex"
    ") VALUES ("
    "  ?,"
    "  ?"
    ")",
    -1,
    &sth,
    NULL
  );

  alx_sqldump_check_rc( p, db, sth, rc, SQLITE_OK );

  // start_vertex
  rc = sqlite3_bind_int64(
    sth,
    1,
    alx_grammar_start_vertex(alx_g(p))
  );

  alx_sqldump_check_rc( p, db, sth, rc, SQLITE_OK );

  // final_vertex
  rc = sqlite3_bind_int64(
    sth,
    2,
    alx_grammar_final_vertex(alx_g(p))
  );

  alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

  rc = sqlite3_step( sth );

  alx_sqldump_check_rc(p, db, sth, rc, SQLITE_DONE);

  rc = sqlite3_finalize( sth );

  alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_sqldump_dump_vertex_list_items(
  struct alx_parser_state const* const p,
  sqlite3 * db,
  sqlite3_stmt * const sth,
  alx_vertex const vertex,
  alx_vertex_list_index const first
) {

  alx_vertex const* vlptr = 
    alx_grammar_vlix_to_vlptr(
      alx_g(p),
      first
    );

  int rc = SQLITE_OK;

  for (; *vlptr; ++vlptr) {

    // vertex
    rc = sqlite3_bind_int64(
      sth,
      1,
      vertex
    );

    alx_sqldump_check_rc( p, db, sth, rc, SQLITE_OK );

    // other
    rc = sqlite3_bind_int64(
      sth,
      2,
      *vlptr
    );

    alx_sqldump_check_rc( p, db, sth, rc, SQLITE_OK );

    rc = sqlite3_step( sth );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_DONE);

    rc = sqlite3_reset( sth );

    alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

  }

#if 0
  alx_vertex const vertex_lists[14775];
  alx_vertex_list_index const vertex_old_predecessors[1487];
  alx_vertex_list_index const vertex_shadows[1487];

  alx_vertex_list_index const vertex_unskippable_shadows[1487];
  alx_vertex_list_index const vertex_unskippable_predecessors[1487];
#endif

}

void
alx_sqldump_dump_old_edge(
  struct alx_parser_state const* const p,
  sqlite3 * db
) {

  alx_logf( p, "dumping old_edge\n" );

  sqlite3_stmt * sth;

  int rc = sqlite3_prepare_v2(
    db,
    "INSERT INTO old_edge("
    "  dst_vertex,"
    "  src_vertex"
    ") VALUES ("
    "  ?,"
    "  ?"
    ")",
    -1,
    &sth,
    NULL
  );

  // TODO: how to handle backtrace_state here?

  alx_sqldump_check_rc( p, db, sth, rc, SQLITE_OK );

  for (alx_vertex v = 1; v <= alx_grammar_max_vertex(alx_g(p)); ++v) {

    alx_vertex_list_index const idx = p->backtrace_state.predecessors[ v ];

    alx_sqldump_dump_vertex_list_items(p, db, sth, v, idx);

  }

  rc = sqlite3_finalize( sth );

  alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

}

void
alx_sqldump_dump_vertex_shadows(
  struct alx_parser_state const* const p,
  sqlite3 * db
) {

  alx_logf( p, "dumping vertex_shadows\n" );

  sqlite3_stmt * sth;

  int rc = sqlite3_prepare_v2(
    db,
    "INSERT INTO vertex_shadows("
    "  vertex,"
    "  shadows"
    ") VALUES ("
    "  ?,"
    "  ?"
    ")",
    -1,
    &sth,
    NULL
  );

  alx_sqldump_check_rc( p, db, sth, rc, SQLITE_OK );

  for (alx_vertex v = 1; v <= alx_grammar_max_vertex(alx_g(p)); ++v) {

    alx_vertex_list_index const idx = p->backtrace_state.shadows[ v ];

    alx_sqldump_dump_vertex_list_items(p, db, sth, v, idx);

  }

  rc = sqlite3_finalize( sth );

  alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_sqldump_dump_grammar(
  struct alx_parser_state const* const p,
  sqlite3 * db
) {

  alx_logf( p, "dumping grammar\n" );

  int rc = sqlite3_exec( db, "BEGIN TRANSACTION", 0, 0, NULL );

  alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

  alx_sqldump_dump_vertex_properties(p, db);
  alx_sqldump_dump_grammar_properties(p, db);
  alx_sqldump_dump_parser_properties(p, db);
  alx_sqldump_dump_old_edge(p, db);
  alx_sqldump_dump_vertex_shadows(p, db);

  rc = sqlite3_exec( db, "COMMIT TRANSACTION", 0, 0, NULL );

  alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_sqldump_dump_shadowed_inputs(
  struct alx_parser_state const* const p,
  sqlite3 * db
) {

  alx_logf(
    p,
    "dumping shadowed_inputs (vector size = %zu)\n",
    alx_v32_size(&p->shadowed_inputs)
  );

  int rc = sqlite3_exec( db, "BEGIN TRANSACTION", 0, 0, NULL );

  alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

  sqlite3_stmt * sth;

  rc = sqlite3_prepare_v2(
    db,
    "INSERT INTO shadowed_inputs("
    "  shadowed_inputs_id,"
    "  offset,"
    "  vertex"
    ") VALUES ("
    "  ?,"
    "  ?,"
    "  ?"
    ")",
    -1,
    &sth,
    NULL
  );

  alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

  size_t current_index = p->final_index;

  for (
    alx_vertex_list_index const* ptr =
      alx_v32_last_ptr( &p->shadowed_inputs );
    ptr >= p->shadowed_inputs.d;
    /**/
  ) {

    alx_vertex const* input_vlptr = 
      alx_grammar_vlix_to_vlptr(
        alx_g(p),
        alx_g(p)->nfa_states[
          alx_uint32_lsb31( *ptr ) ].vertices
      );

    for (; *input_vlptr; ++input_vlptr) {

      // index
      rc = sqlite3_bind_int64(
        sth,
        1,
        alx_ptrdiff_t_to_size_t(ptr - p->shadowed_inputs.d)
      );

      alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

      // offset
      rc = sqlite3_bind_int64(
        sth,
        2,
        current_index
      );

      alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

      // offset
      rc = sqlite3_bind_int64(
        sth,
        3,
        *input_vlptr
      );

      if (false) {
        alx_logf(
          p,
          "inserting %lu, %u, %u\n",
          alx_ptrdiff_t_to_size_t(ptr - p->shadowed_inputs.d),
          current_index,
          *input_vlptr
        );
      }

      rc = sqlite3_step( sth );

      alx_sqldump_check_rc(p, db, sth, rc, SQLITE_DONE);

      rc = sqlite3_reset( sth );

      alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

    }

    ptr -= 1;

    if (!alx_uint32_msb1(*ptr)) {
      current_index -= 1;
    }

  }

  rc = sqlite3_exec( db, "COMMIT TRANSACTION", 0, 0, NULL );

  alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

  rc = sqlite3_finalize( sth );

  alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_sqldump_dump_vectors(
  struct alx_parser_state const* const p,
  sqlite3 * db
) {

  for ( size_t ix = 0; ix < p->vector_count; ++ix ) {

    // Not all vectors should end up in the database
    if (!p->vector_list[ix]->table_name) {

      continue;

    }

    size_t placeholder_count = 0;

    for (size_t sx = 0; sx < strlen(p->vector_list[ix]->insert_template); ++sx) {
      placeholder_count += p->vector_list[ix]->insert_template[sx] == '?';
    }

    // vector index is always passed for first placeholder
    size_t const items_per_row = placeholder_count - 1;
    
    char sql[256];

    // NOTE: Using INSERT OR IGNORE here to handle duplicates, but
    // it also affects violations of other constraints like NOT NULL.
    // That can lead to confusing behavior. Might be best to simply 
    // put the duplicates into the result?
    int written = snprintf(
      sql,
      256,
      "INSERT OR IGNORE INTO %s VALUES(%s)",
      p->vector_list[ix]->table_name,
      p->vector_list[ix]->insert_template
    );

    // TODO: error handling?
    alx_unused(&written);

    int rc = sqlite3_exec( db, "BEGIN TRANSACTION", 0, 0, NULL );

    alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

    sqlite3_stmt * sth;

    rc = sqlite3_prepare_v2(
      db,
      sql,
      -1,
      &sth,
      NULL
    );

    alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

    alx_logf(
      p,
      "dumping to table %s (vector size = %zu)\n",
      p->vector_list[ix]->table_name,
      alx_v32_size( p->vector_list[ix] )
    );

    alx_sqldump_insert_vector(
      p,
      db,
      sth,
      p->vector_list[ix],
      0,
      alx_v32_size( p->vector_list[ix] ),
      items_per_row
    );

    rc = sqlite3_finalize( sth );

    alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

    rc = sqlite3_exec( db, "COMMIT TRANSACTION", 0, 0, NULL );

    alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

  }

}

void
alx_sqldump_postdata(
  struct alx_parser_state const* const p,
  sqlite3 * db
) {

  alx_logf( p, "Creating indices and adding computed columns\n" );

  int rc = sqlite3_exec(
    db,
    alx_g_indices_and_computed_columns,
    NULL,
    NULL,
    NULL
  );

  alx_sqldump_check_rc( p, db, NULL, rc, SQLITE_OK );  

}

void
alx_sqldump_dump_parser_state(
  struct alx_parser_state const* const p,
  char const* const db_path
) {

  sqlite3 * db;

  if ( remove( db_path ) != 0 ) {
    // Ignore errors
  }

  int rc = sqlite3_open_v2(
    db_path,
    &db,
    SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE,
    NULL
  );
  
  if (rc != SQLITE_OK) {  
    alx_logf( p, "error opening db %s\n", db_path );
    return;
  }

  alx_sqldump_deploy_schema(p, db);

  alx_sqldump_deploy_views(p, db);

  alx_sqldump_dump_grammar(p, db);

  alx_sqldump_dump_shadowed_inputs(p, db);

  alx_sqldump_dump_vectors(p, db);

  rc = sqlite3_exec( db, "ANALYZE", 0, 0, NULL );

  alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

  alx_sqldump_dump_file_contents( p, db );

  alx_sqldump_postdata(p, db);

  rc = sqlite3_close( db );

  alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

}

