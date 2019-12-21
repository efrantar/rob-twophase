#ifndef __CUBIE__
#define __CUBIE__

#include <string>

namespace cubie {

  namespace corner {
    const int COUNT = 8;

    const int URF = 0;
    const int UFL = 1;
    const int ULB = 2;
    const int UBR = 3;
    const int DFR = 4;
    const int DLF = 5;
    const int DBL = 6;
    const int DRB = 7;
  }
  using namespace corner;

  namespace edge {
    const int COUNT = 12;

    const int UR = 0;
    const int UF = 1;
    const int UL = 2;
    const int UB = 3;
    const int DR = 4;
    const int DF = 5;
    const int DL = 6;
    const int DB = 7;
    const int FR = 8;
    const int FL = 9;
    const int BL = 10;
    const int BR = 11;
  }
  using namespace edge;

  struct cube {
    int cperm[corner::COUNT];
    int eperm[edge::COUNT];
    int cori[corner::COUNT];
    int eori[edge::COUNT];
  };

  const cube SOLVED_CUBE = {
    {URF, UFL, ULB, UBR, DFR, DLF, DBL, DRB},
    {UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR},
    {}, {}
  };

  namespace corner {
    void mul(const cube& c1, const cube& c2, cube& into);
  }
  namespace edge {
    void mul(const cube& c1, const cube& c2, cube& into);
  }

  void mul(const cube& c1, const cube& c2, cube& into);
  void inv(const cube& c, cube& into);
  void shuffle(cube& c);
  int check(const cube& c);

  bool operator==(const cube& c1, const cube& c2);
  bool operator!=(const cube& c1, const cube& c2);

}

#endif
