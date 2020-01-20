/**
 * Symmetry definition and reduction/conjugation tables.
 */

#ifndef __SYM__
#define __SYM__

#include "coord.h"
#include "cubie.h"
#include "move.h"

namespace sym {

  const int COUNT = 48;

  #ifdef F5
    const int COUNT_SUB = 4; // number of symmetries used for reduction
    const int N_FSLICE1 = 255664;
    const int N_CORNERS = 10368;
    const int ROT = 36; // 90 degree rotation around FB-axis
  #else
    const int COUNT_SUB = 16;
    const int N_FSLICE1 = 64430;
    const int N_CORNERS = 2768;
    const int ROT = 16; // 120 degree rotation around axis through URF and DLB corner
  #endif

  extern cubie::cube cubes[COUNT];
  extern int inv[COUNT];
  extern int effect[COUNT][3];

  extern int conj_move[move::COUNT][COUNT];
  extern uint16_t conj_twist[coord::N_TWIST][COUNT_SUB];
  extern uint16_t conj_udedges2[coord::N_UDEDGES2][COUNT_SUB];

  extern uint32_t fslice1_sym[coord::N_FSLICE1];
  extern uint32_t corners_sym[coord::N_CORNERS];
  extern uint32_t fslice1_raw[N_FSLICE1];
  extern uint16_t corners_raw[N_CORNERS];
  extern uint16_t fslice1_selfs[N_FSLICE1];
  extern uint16_t corners_selfs[N_CORNERS];

  inline bool eff_inv(int eff) { return eff & 1; }
  inline bool eff_flip(int eff) { return eff & 2; }
  inline int eff_shift(int eff) { return eff >> 2; }
  inline int coord_c(int coord) { return coord / COUNT_SUB; }
  inline int coord_s(int coord) { return coord % COUNT_SUB; }

  void init();

}

#endif
