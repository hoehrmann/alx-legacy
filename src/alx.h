#ifndef ALX_ALX_H

  #define ALX_ALX_H

  #ifdef __cplusplus
    extern "C" {
  #endif

  /////////////////////////////////////////////////////////////////////
  // Compile-time options
  /////////////////////////////////////////////////////////////////////
  
  #define ALX_TRACE 0
  #define ALX_DEBUG 0
  #define ALX_SQLITE 1

  #if defined(_MSC_VER)

    #define _CRT_SECURE_NO_WARNINGS 1

    // pre-C11 functions may be unsafe
    #pragma warning(disable:4996)

    // nonstandard extension used: bit field types other than int
    #pragma warning(disable:4214)

    // nonstandard extension used: initialisation w/automatic variable
    #pragma warning(disable:4221)

    // nonstandard extension used: non-constant aggregate initializer
    #pragma warning(disable:4204)

    // unreferenced formal parameter
    #pragma warning(disable:4100)

    // unreferenced local variabkle
    #pragma warning(disable:4101)

    // function selected for automatic inline expansion
    #pragma warning(disable:4711)

    // function not inlined
    #pragma warning(disable:4710)

    // 'x' is not defined as a preprocessor macro, replacing ...
    #pragma warning(disable:4668)

    // 'x' bytes padding added after data member 'y'
    #pragma warning(disable:4820)

    // structure was padded due to alignment specifier
    #pragma warning(disable:4324)

    // has C-linkage specified, but returns UDT
    #pragma warning(disable:4190)

  #elif defined(__GNUC__)

    #pragma GCC diagnostic ignored "-Wunused-variable"
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #pragma GCC diagnostic ignored "-Wunused-function"
    #pragma GCC diagnostic ignored "-Woverlength-strings"
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"

    #if !defined(__clang__)
    #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    #endif

    #if defined(__clang__)
    #pragma GCC diagnostic ignored "-Wgnu-empty-initializer"
    #pragma GCC diagnostic ignored \
      "-Wtautological-constant-out-of-range-compare"
    #endif

  #endif

  /////////////////////////////////////////////////////////////////////
  // Standard includes
  /////////////////////////////////////////////////////////////////////

  #include <assert.h>
  #include <inttypes.h>
  #include <limits.h>
  #include <memory.h>
  #include <setjmp.h>
  #include <stdarg.h>
  #include <stdbool.h>
  #include <stddef.h>
  #include <stdint.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <time.h>

  /////////////////////////////////////////////////////////////////////
  // Third party includes
  /////////////////////////////////////////////////////////////////////

  #include "ext/sqlite3.h"

  /////////////////////////////////////////////////////////////////////
  // Inline suggestions
  /////////////////////////////////////////////////////////////////////

  #define ALX_INLINE 

  /////////////////////////////////////////////////////////////////////
  // Project includes
  /////////////////////////////////////////////////////////////////////

  #include "types.h"
  #include "forward.h"

  /////////////////////////////////////////////////////////////////////
  // Override inline suggestions (to make makeheaders happy)
  /////////////////////////////////////////////////////////////////////

  #undef ALX_INLINE
  #if defined(__GNUC__)
  #define ALX_INLINE __attribute__((always_inline))
  #else
  #define ALX_INLINE inline
  #endif

  /////////////////////////////////////////////////////////////////////
  // Forward declarations not handled by makeheaders
  /////////////////////////////////////////////////////////////////////

  void alx_logf_p( struct alx_log_context const ctx, ... );
  struct alx_set * alx_set_create( struct alx_parser_state const* const p );
  void alx_set_insert( struct alx_set * const set, struct alx_graph_quad const* const quad );
  bool alx_set_has( struct alx_set * const set, struct alx_graph_quad const* const quad );
  bool alx_set_has_exteriors( struct alx_set * const set, struct alx_graph_quad const* const quad );
  void alx_set_clear( struct alx_set * const set );
  void alx_set_destroy( struct alx_set * const set );

  /////////////////////////////////////////////////////////////////////
  // Logging macros
  /////////////////////////////////////////////////////////////////////

  #define alx_logf(p, ...) alx_logf_p( \
    alx_log_get_context(p, __FILE__, __func__, __LINE__), __VA_ARGS__)

  #define alx_log_quad(p, ...) alx_log_quad_p( \
    alx_log_get_context(p, __FILE__, __func__, __LINE__), __VA_ARGS__)

  #define alx_log_graph_node(p, ...) alx_log_graph_node_p( \
    alx_log_get_context(p, __FILE__, __func__, __LINE__), __VA_ARGS__)

  #define alx_log_pair(p, ...) alx_log_pair_p( \
    alx_log_get_context(p, __FILE__, __func__, __LINE__), __VA_ARGS__)

  #define alx_log_memory(p) alx_log_memory_p( \
    alx_log_get_context(p, __FILE__, __func__, __LINE__), p)

  #define alx_sqldump_check_rc(p, db, sth, got, expected) \
    alx_sqldump_check_rc_p( \
      alx_log_get_context(p, __FILE__, __func__, __LINE__), \
        p, db, sth, got, expected)

  #define alx_debugf fprintf
  //#define alx_debugf if (0) fprintf

  #ifdef __cplusplus
    }
  #endif

#endif
