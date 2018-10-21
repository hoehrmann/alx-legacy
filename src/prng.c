#include "alx.h"

/////////////////////////////////////////////////////////////////////
// Pseudo-random number generator helper functions
/////////////////////////////////////////////////////////////////////

int
alx_rng_minmax(
  struct alx_rng * const rng,
  int const min,
  int const max
) {

  return min + rand() / (RAND_MAX / (max - min + 1) + 1);

}
