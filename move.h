#ifndef __MOVE__
#define __MOVE__

#include <cstdint>
#include <string>

#include "cubie.h"

namespace move {

  using mask = uint64_t;

  #ifdef QT
    #ifdef AX
      const int COUNT = 30;
    #else
      const int COUNT = 16;
    #endif
  #else
    #ifdef AX
      const int COUNT = 45;
    #else
      const int COUNT = 18;
    #endif
  #endif

  extern std::string names[COUNT];
  extern cubie::cube cubes[COUNT];
  extern int inv[COUNT];

  extern mask next[COUNT];
  extern mask qt_skip[COUNT];

  extern mask p1mask;
  extern mask p2mask;

}

#endif
