/**
 * Pruning table generation and lookup.
 */

#ifndef __PRUN__
#define __PRUN__

#include <cstdint>
#include "coord.h"
#include "sym.h"

namespace prun {

  const int N_FS1TWIST = sym::N_FSLICE1 * coord::N_TWIST;
  const int N_CORNUD2 = sym::N_CORNERS * coord::N_UDEDGES2;
  const int N_CSLICE2 = coord::N_CORNERS * coord::N_SLICE2;

  #ifdef AX
    using prun1 = uint64_t;
  #else
    using prun1 = uint32_t;
  #endif

  extern prun1  *phase1;
  extern uint8_t *phase2;
  extern uint8_t *precheck;

  int get_phase1(int flip, int slice, int twist, int togo, move::mask& next);
  int get_phase2(int corners, int udedges);
  int get_precheck(int corners, int slice);

  bool init(bool file = true);

}

#endif
