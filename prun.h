#ifndef __PRUN__
#define __PRUN__

#include <cstdint>
#include "coord.h"
#include "sym.h"

namespace prun {

  const int N_CORNUD2 = sym::N_CORNERS * coord::N_UDEDGES2;
  const int N_CSLICE2 = coord::N_CORNERS * coord::N_SLICE2;

  extern uint8_t *phase2;
  extern uint8_t *precheck;

  int get_phase2(int corners, int udedges);
  int get_precheck(int corners, int slice2);

  void init();

}

#endif
