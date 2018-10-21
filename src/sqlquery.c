#include "alx.h"

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_sqlquery_cmdline_args_to_table(
  struct alx_parser_state const* const p,
  sqlite3 * const db
) {

  sqlite3_stmt * sth;

  // FIXME: decide on whether free-form defines should be possible
  // and adapt the code accordingly. Right now it is a bit more 
  // complicated than necessary because the direction was not clear
  // when it was first written...

  int rc = sqlite3_exec(
    db,
    "DROP TABLE IF EXISTS command_line_defines",
    NULL,
    NULL,
    NULL
  );

  alx_sqldump_check_rc( p, db, NULL, rc, SQLITE_OK );

  rc = sqlite3_prepare_v2(
    db,
    "\
      CREATE TABLE command_line_defines AS \
      SELECT \
        CAST( :name AS TEXT ) AS name, \
        CAST( :tail_length AS INT ) AS tail_length, \
        CAST( :max_edges AS INT ) AS max_edges \
    ",
    -1,
    &sth,
    NULL
  );

  alx_sqldump_check_rc( p, db, sth, rc, SQLITE_OK );

  sqlite3_str * str = sqlite3_str_new( db );

  // Parse command line parameters
  for (int ax = 1; ax < p->args.argc; ++ax) {

    char const* const s = p->args.argv[ax];
    char const* const value = strchr(s, '=');

    if ( !alx_starts_with(s, "-D") || value == NULL ) {

      // NOTE: this assumes we do not have arguments that span
      // multiple argv entries.
      continue;

    }

    size_t const nlen = value - s;

    sqlite3_str_reset( str );

    sqlite3_str_appendchar( str, 1, ':' );

    sqlite3_str_append(
      str,
      s + 2,
      alx_ptrdiff_t_to_int( value - (s + 2) )
    );

    int index = sqlite3_bind_parameter_index(
      sth,
      sqlite3_str_value(str)
    );

    if (index) {

      rc = sqlite3_bind_text(
        sth,
        index,
        value + 1,
        -1,
        SQLITE_STATIC
      );

      alx_sqldump_check_rc( p, db, sth, rc, SQLITE_OK );

      alx_logf(
        p,
        "Bound %s as %s = %s for bind parameter #%d\n",
        s,
        sqlite3_str_value(str),
        value + 1,
        index
      );

    } else {

      alx_logf(
        p,
        "Unknown -D key %s\n",
        sqlite3_str_value(str)
      );

    }

  }

  sqlite3_free( sqlite3_str_finish( str ) );

  rc = sqlite3_step( sth );

  alx_sqldump_check_rc(p, db, sth, rc, SQLITE_DONE);

  rc = sqlite3_finalize( sth );

  alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_sqlquery_dotbounds(
  char const* const fst_start,
  char const* * const fst_after,
  char const* * const snd_start,
  char const* * const snd_after
) {

  size_t const length = strlen( fst_start );

  char const* const dot = strchr(fst_start, '.');

  if (dot == NULL) {
    *fst_after = fst_start + length;
    *snd_start = NULL;
    *snd_after = NULL;
    return;
  }

  *fst_after = dot;
  *snd_start = dot + 1;

  char const* const dot2 = strchr(*snd_start, '.');

  if (dot2 == NULL) {
    *snd_after = fst_start + length;
  } else {
    *snd_after = dot2;
  }
  
}

/////////////////////////////////////////////////////////////////////
// Create _data VIEWs for --query pipeline
/////////////////////////////////////////////////////////////////////

char const*
alx_sqlquery_deploy_data_views(
  struct alx_parser_state const* const p,
  sqlite3 * const db
) {

  // NOTE(bh): "The sqlite3_str_new(D) interface always returns
  // a pointer to a valid sqlite3_str object". No need to handle
  // errors at this point.

  sqlite3_str * str = sqlite3_str_new( db );

  char const* path = p->args.query;

  while (true) {

    char const* fst_final;
    char const* snd_start;
    char const* snd_final;

    alx_sqlquery_dotbounds(path, &fst_final, &snd_start, &snd_final);

    if (snd_start == NULL) {
      break;
    }

    sqlite3_str_appendf(
      str,
      "\
      DROP VIEW IF EXISTS \"view_%.*w_data\"; \
      CREATE VIEW \"view_%.*w_data\" AS \
      SELECT * FROM \"view_%.*w\"; \
      ",
      snd_final - snd_start,
      snd_start,
      snd_final - snd_start,
      snd_start,
      fst_final - path,
      path
    );

    path = snd_start;

  }

#if 0
  alx_logf(
    p,
    "Query data SQL: %s\n",
    sqlite3_str_value(str)
  );
#endif

  int rc = sqlite3_exec(
    db,
    sqlite3_str_value(str),
    NULL,
    NULL,
    NULL
  );

  sqlite3_free( sqlite3_str_finish( str ) );

  alx_sqldump_check_rc( p, db, NULL, rc, SQLITE_OK );

  return path;

}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////

void
alx_sqlquery_plain(
  struct alx_parser_state const* const p,
  sqlite3 * const db,
  sqlite3_stmt * const sth  
) {

  int rc;

  ///////////////////////////////////////////////////////////////////
  // Retrieve and print results
  ///////////////////////////////////////////////////////////////////
  while (true) {

    rc = sqlite3_step( sth );

    if ( rc != SQLITE_ROW ) {
      break;
    }

    fprintf( stdout, "%s\n", sqlite3_column_text(sth, 0) );

  }

  alx_sqldump_check_rc(p, db, sth, rc, SQLITE_DONE);

  rc = sqlite3_finalize( sth );

  alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

}

void
alx_sqlquery_something(
  struct alx_parser_state const* const p,
  char const* const db_path
) {

  sqlite3 * db;

  alx_logf( p, "sqlite3_open on %s to perform query\n", db_path );

  // TODO: open read-only? Could be done by the caller through URI
  int rc = sqlite3_open_v2(
    db_path,
    &db,
    SQLITE_OPEN_READWRITE,
    NULL
  );
  
  if (rc != SQLITE_OK) {  
    alx_logf( p, "error opening db %s\n", db_path );
    return;
  }

  alx_sqlquery_cmdline_args_to_table( p, db );

  char const* const path = alx_sqlquery_deploy_data_views( p, db );

  sqlite3_str * str = sqlite3_str_new( db );

  sqlite3_str_appendf(
    str,
    "SELECT line FROM \"view_%w\"",
    path
  );

  sqlite3_stmt * sth;

#if 0
  alx_logf( p, "Query: %s\n", alx_g_tail_as_dot );
#endif

  rc = sqlite3_prepare_v2(
    db,
    sqlite3_str_value(str),
    -1,
    &sth,
    NULL
  );

  sqlite3_free( sqlite3_str_finish( str ) );

  alx_sqldump_check_rc(p, db, sth, rc, SQLITE_OK);

  // TODO: replace this with creation of a single argument table.

  ///////////////////////////////////////////////////////////////////
  // Bind NULL as default value for all parameters; rebind later.
  ///////////////////////////////////////////////////////////////////
  for (
    int ix = 1;
    ix <= sqlite3_bind_parameter_count(sth);
    ++ix
  ) {

    alx_logf(
      p,
      "bind parameter %u has name %s\n",
      ix,
      sqlite3_bind_parameter_name(sth, ix)
    );

    rc = sqlite3_bind_null(
      sth,
      ix
    );

    alx_sqldump_check_rc( p, db, sth, rc, SQLITE_OK );

  }

  alx_logf( p, "Selecting from %s\n", path );
  alx_sqlquery_plain( p, db, sth );
  alx_logf( p, "Done processing query\n", path );

}

