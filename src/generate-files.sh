#!/bin/bash

HERE="$( realpath $( dirname "${BASH_SOURCE[0]}" ) )"

makeheaders -v -h "$HERE"/*.c > "$HERE"/forward.h

rm "$HERE"/sqlquery.h

function sql_file_path_to_ident {

  local sql_file="$1"
  local basename=$(basename "$sql_file" .sql)
  local varname=$(echo "$basename" | tr \- _)
  local identifier=$(printf "alx_g_%s" "$varname")

  echo -n "$identifier"

}

function sql_file_to_header_string {
  
  local sql_file="$1"

  printf "\nstatic char const %s[] = {\n" \
    $( sql_file_path_to_ident "$sql_file" ) ;
  xxd -i -c 11 < "$sql_file" ;
  printf ", 0x00\n" ;
  printf "};\n"

}

(

  printf "#ifndef ALX_SQLQUERY_H\n"
  printf "#define ALX_SQLQUERY_H\n"
  printf "#include \"alx.h\"\n"

  for sql_file in "$HERE"/sql/*.sql; do
    sql_file_to_header_string "$sql_file"
  done

  echo "
struct {
  char const* const name;
  char const* const sql;
} const alx_g_sqls[] = {
  ";

  for sql_file in "$HERE"/sql/*.sql; do
    ident=$(sql_file_path_to_ident "$sql_file")
    bare=${ident#"alx_g_"}
    printf "  { \"%s\", %s },\n" "$bare" "$ident"
  done

  echo "
  { NULL, NULL }
};
  "

  printf "#endif\n"

) >> "$HERE"/sqlquery.h

