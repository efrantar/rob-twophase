/**
 * All kinds of moveset definitions and setup
 */

#ifndef __MOVE__
#define __MOVE__

#include <cstdint>
#include <string>
#include <vector>

#include "cubie.h"

namespace move {

  using mask = uint64_t;

  #ifdef QT
    #ifdef AX
      const int COUNT = 30;
      const int COUNT1 = 24;
      const int split[] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        8, 10, 12, 16, 18, 20
      }; // split extra half-turns into quarter-turn (for cleaner phase 2 implementation)
    #else
      const int COUNT = 16;
      const int COUNT1 = 12;
      const int split[] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        4, 6, 8, 10
      };
    #endif
  #else
    #ifdef AX
      const int COUNT = 45;
      const int COUNT1 = 45;
    #else
      const int COUNT = 18;
      const int COUNT1 = 18;
    #endif
  #endif

  extern std::string names[COUNT];
  extern cubie::cube cubes[COUNT];
  extern int inv[COUNT];

  extern mask next[COUNT]; // successor moves that should be explored
  extern mask qt_skip[COUNT]; // to avoid ever trying M^3 = M' in QT mode

  extern mask p1mask; // phase 1 moves
  extern mask p2mask; // phase 2 moves

  inline mask bit(int m) {
    return mask(1) << m;
  }
  inline bool in(int m, mask mm) {
    return mm & bit(m);
  }

  // Convert solution to AXHT; especially useful when solving in AXQT
  std::string compress(const std::vector<int>& mseq);

  /* Compute solution lengths in different metrics */
  int len_ht(const std::vector<int>& mseq);
  int len_axht(const std::vector<int>& mseq);
  int len_qt(const std::vector<int>& mseq);
  int len_axqt(const std::vector<int>& mseq);

  void init();

}

#endif
