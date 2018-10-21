#include "alx.h"

/////////////////////////////////////////////////////////////////////
// Various CREATE VIEW and related statements.
/////////////////////////////////////////////////////////////////////

void
alx_sqldump_deploy_views(
  struct alx_parser_state const* const p,
  sqlite3 * db
) {

  // TODO: it might be better to create the views just prior to 
  // running queries, or perhaps re-create them before running any
  // query? Would then be easier to experiment with adding new
  // queries...

  for (size_t ix = 0; alx_g_sqls[ix].name; ++ix) {

    if (!alx_starts_with(alx_g_sqls[ix].name, "view_")) {
      continue;
    }

    alx_logf(
      p,
      "Deploying VIEW %s\n",
      alx_g_sqls[ix].name
    );

    int rc = sqlite3_exec( db, alx_g_sqls[ix].sql, NULL, NULL, NULL );

    alx_sqldump_check_rc(p, db, NULL, rc, SQLITE_OK);

  }

}

/////////////////////////////////////////////////////////////////////
// All the CREATE TABLE and related statements.
/////////////////////////////////////////////////////////////////////

void
alx_sqldump_deploy_schema(
  struct alx_parser_state const* const p,
  sqlite3 * db
) {

  int rc = sqlite3_exec( db, alx_g_schema, NULL, NULL, NULL );

  if (rc != SQLITE_OK) {  
    alx_logf( p, "failed to deploy schema (rc: %d)\n", rc );
    return;
  }

}
