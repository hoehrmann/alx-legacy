#include "alx.h"

/////////////////////////////////////////////////////////////////////
// Helper function for unused variables
/////////////////////////////////////////////////////////////////////

void
alx_unused(void const* const value) {
  // This can be used to silence "unused" compiler warnings.
}

/////////////////////////////////////////////////////////////////////
// Trap for debugging
/////////////////////////////////////////////////////////////////////

void
alx_dbg_break(void) {
#if 1
  abort();
#elif defined(__GNUC__)
  // Apparently some CLang versions do not have __builtin__trap()
  __builtin__trap();
#else
  __debugbreak();
#endif
}

/////////////////////////////////////////////////////////////////////
// String functions
/////////////////////////////////////////////////////////////////////

bool
alx_starts_with(
  char const* const string,
  char const* const prefix
) {

  size_t prefix_length = strlen(prefix);

  return strncmp(string, prefix, prefix_length) == 0;

}
