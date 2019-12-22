#include <iostream>
#include "move.h"

namespace move {

  using namespace cubie::corner;
  using namespace cubie::edge;

  #ifdef QT
    #ifdef AXIAL
        const int COUNT = 30;
        const int map[] = {
          0, -1, 1, 2, -1, 3,
          4, -1, 5, -1, -1, -1, 6, -1, 7,
          8, 24, 9, 10, 25, 11,
          12, -1, 13, -1, 26, -1, 14, -1, 15,
          16, 27, 17, 18, 28, 19,
          20, -1, 21, -1, 29, -1, 22, -1, 23
        };
      #else
        const int map[] = {
          0, -1, 1, 2, -1, 3,
          -1, -1, -1, -1, -1, -1, -1, -1, -1,
          4, 12, 5, 6, 13, 7,
          -1, -1, -1, -1, -1, -1, -1, -1, -1,
          8, 14, 9, 10, 15, 11,
          -1, -1, -1, -1, -1, -1, -1, -1, -1
        };
      #endif
  #else
    #ifdef AX
      const int map[] = {
        0, 1, 2, 3, 4, 5,
        6, 7, 8, 9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20,
        21, 22, 23, 24, 25, 26, 27, 28, 29,
        30, 31, 32, 33, 34, 35,
        36, 37, 38, 39, 40, 41, 42, 43, 44
      };
    #else
      const int map[] = {
        0, 1, 2, 3, 4, 5,
        -1, -1, -1, -1, -1, -1, -1, -1, -1,
        6, 7, 8, 9, 10, 11,
        -1, -1, -1, -1, -1, -1, -1, -1, -1,
        12, 13, 14, 15, 16, 17,
        -1, -1, -1, -1, -1, -1, -1, -1, -1
      };
    #endif
  #endif

  std::string names[COUNT];
  cubie::cube cubes[COUNT];
  int inv[COUNT];

  mask next[COUNT];
  mask qt_skip[COUNT];

  mask p1mask = (mask(1) << 45) - 1;
  mask p2mask = 0x040904097fff;

  const cubie::cube U = {
    {UBR, URF, UFL, ULB, DFR, DLF, DBL, DRB},
    {UB, UR, UF, UL, DR, DF, DL, DB, FR, FL, BL, BR},
    {}, {}
  };
  const cubie::cube D = {
    {URF, UFL, ULB, UBR, DLF, DBL, DRB, DFR},
    {UR, UF, UL, UB, DF, DL, DB, DR, FR, FL, BL, BR},
    {}, {}
  };
  const cubie::cube R = {
    {DFR, UFL, ULB, URF, DRB, DLF, DBL, UBR},
    {FR, UF, UL, UB, BR, DF, DL, DB, DR, FL, BL, UR},
    {2, 0, 0, 1, 1, 0, 0, 2}, {}
  };
  const cubie::cube L = {
    {URF, ULB, DBL, UBR, DFR, UFL, DLF, DRB},
    {UR, UF, BL, UB, DR, DF, FL, DB, FR, UL, DL, BR},
    {0, 1, 2, 0, 0, 2, 1, 0}, {}
  };
  const cubie::cube F = {
    {UFL, DLF, ULB, UBR, URF, DFR, DBL, DRB},
    {UR, FL, UL, UB, DR, FR, DL, DB, UF, DF, BL, BR},
    {1, 2, 0, 0, 2, 1, 0, 0},
    {0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0}
  };
  const cubie::cube B = {
    {URF, UFL, UBR, DRB, DFR, DLF, ULB, DBL},
    {UR, UF, UL, BR, DR, DF, DL, BL, FR, FL, UB, DB},
    {0, 0, 1, 2, 0, 0, 2, 1},
    {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1}
  };

  mask reindex(mask mm) {
    mask mm1 = 0;
    for (int m = 0; m < 45; m++) {
      if (map[m] != -1 && mm & (mask(1) << m))
        mm1 |= mask(1) << map[m];
    }
    return mm1;
  }

  bool autoinit() {
    std::string names1[45];
    cubie::cube cubes1[45];
    int inv1[45];
    mask next1[45];
    mask qt_skip1[45];

    std::string fnames[] = {"U", "D", "R", "L", "F", "B"};
    std::string pnames[] = {"", "2", "'"};
    cubie::cube fcubes[] = {U, D, R, L, F, B};

    for (int ax = 0; ax < 3; ax++) {
      int i1 = 15 * ax;
      int i2 = 15 * ax + 3;
      int i3 = 15 * ax + 6;

      for (int f : {2 * ax, 2 * ax + 1}) {
        for (int cnt = 0; cnt < 3; cnt++) {
          int m = (f & 1) ? i2 + cnt : i1 + cnt;
          names1[m] = fnames[f] + pnames[cnt];
          if (cnt == 0)
            cubes1[m] = fcubes[f];
          else
            cubie::mul(cubes1[m - 1], cubes1[f], cubes1[m]);
          inv1[m] = (f & 1) ? i2 + (2 - cnt) : i1 + (2 - cnt);
          if (f % 2 == 0) {
            next1[m] |= mask(0x3f) << i1;
            next1[m] |= mask(0x7) << i3 + 3 * cnt;
          } else {
            next1[m] |= mask(0x3) << i2;
            next1[m] |= mask(0x124) << i3 + 3 * cnt;
          }
        }
      }
      for (int cnt1 = 0; cnt1 < 3; cnt1++) {
        for (int cnt2 = 0; cnt2 < 3; cnt2++) {
          int m = i3 + 3 * cnt1 + cnt2;
          names1[m] = "(" + names1[i1 + cnt1] + " " + names1[i2 + cnt2] + ")";
          cubie::mul(cubes1[i1 + cnt1], cubes1[i2 + cnt2], cubes1[m]);
          inv1[m] = i3 + 3 * (2 - cnt1) + (2 - cnt2);
          next1[m] |= mask(0x7fff) << 15 * ax;
        }
      }
    }
    #ifdef QT
      for (int m : {0, 3, 6, 15, 18, 21, 30, 33, 36})
        next1[m] |= mask(1) << m;
    #endif
    for (int m = 0; m < 45; m++)
      next1[m] = ~next1[m];
    for (int ax = 0; ax < 3; ax++) {
      for (int f = 0; f < 2; f++) {
        int i = 15 * ax + 3 * f;
        qt_skip1[i] |= mask(1) << i;
        qt_skip1[i] |= mask(1) << 15 * ax + 6;
      }
    }

    for (int m = 0; m < 45; m++) {
      if (map[m] == -1)
        continue;
      int i = map[m];

      names[i] = names1[m];
      cubes[i] = cubes1[m];
      inv[i] = map[inv1[m]];
      next[i] = reindex(next1[m]);
      qt_skip[i] = reindex(qt_skip1[m]);

      std::cout << inv[i] << "\n";
    }

    #ifdef QT
      p1mask &= ~0x92e92e92e92;
      p2mask &= ~0x2e92;
    #endif
    #ifdef F5
      mask tmp = ~(mask(0xfff) << 33);
      p1mask &= tmp;
      p2mask &= tmp;
    #endif
    p1mask = reindex(p1mask);
    p2mask = reindex(p2mask);

    return true;
  }
  bool inited = autoinit();

}
