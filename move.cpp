#include "move.h"

#include <algorithm>
#include <iostream>

namespace move {

  using namespace cubie::corner;
  using namespace cubie::edge;

  /* Select moves and order according to used metric */
  #ifdef QT
    #ifdef AX
        const int map[] = {
          // U, U2, U', D, D2, D'
          0, -1, 1, 2, -1, 3,
          // (U D), (U D2), (U D'), (U2 D), (U2 D2), (U2 D'), (U' D), (U' D2), (U' D')
          4, -1, 5, -1, -1, -1, 6, -1, 7,
          // R, R2, R', L, L2, L'
          8, 24, 9, 10, 25, 11,
          // (R L), (R L2), (R L'), (R2 L), (R2 L2), (R2 L'), (R' L), (R' L2), (R' L')
          12, -1, 13, -1, 26, -1, 14, -1, 15,
          // F, F2, F', B, B2, B'
          16, 27, 17, 18, 28, 19,
          // (F B), (F B2), (F B'), (F2 B), (F2 B2), (F2 B'), (F' B), (F' B2), (F' B')
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

  mask p1mask = bit(45) - 1;
  mask p2mask = 0x10482097fff; // 000010000 010010 000010000 010010 111111111 111111;

  // For full set of 45 moves no matter the solving mode
  std::string names1[45];
  int merge[45][45];
  int unmap[COUNT];

  // Translate bitmask from full moveset to configured one
  mask reindex(mask mm) {
    mask mm1 = 0;
    for (int m = 0; m < 45; m++) {
      if (map[m] != -1 && in(m, mm)) // drop unmapped moves
        mm1 |= bit(map[m]);
    }
    return mm1;
  }

  // Build full moveset first, then remap to configured one
  void init() {
    for (int m = 0; m < 45; m++) {
      if (map[m] != -1)
        unmap[map[m]] = m;
    }

    cubie::cube cubes1[45];
    int inv1[45];
    mask next1[45];
    mask qt_skip1[45];

    std::string fnames[] = {"U", "D", "R", "L", "F", "B"};
    std::string pnames[] = {"", "2", "'"};
    cubie::cube fcubes[] = {
      { // U
        {UBR, URF, UFL, ULB, DFR, DLF, DBL, DRB},
        {UB, UR, UF, UL, DR, DF, DL, DB, FR, FL, BL, BR},
        {}, {}
      },
      { // D
        {URF, UFL, ULB, UBR, DLF, DBL, DRB, DFR},
        {UR, UF, UL, UB, DF, DL, DB, DR, FR, FL, BL, BR},
        {}, {}
      },
      { // R
        {DFR, UFL, ULB, URF, DRB, DLF, DBL, UBR},
        {FR, UF, UL, UB, BR, DF, DL, DB, DR, FL, BL, UR},
        {2, 0, 0, 1, 1, 0, 0, 2}, {}
      },
      { // L
        {URF, ULB, DBL, UBR, DFR, UFL, DLF, DRB},
        {UR, UF, BL, UB, DR, DF, FL, DB, FR, UL, DL, BR},
        {0, 1, 2, 0, 0, 2, 1, 0}, {}
      },
      { // F
        {UFL, DLF, ULB, UBR, URF, DFR, DBL, DRB},
        {UR, FL, UL, UB, DR, FR, DL, DB, UF, DF, BL, BR},
        {1, 2, 0, 0, 2, 1, 0, 0},
        {0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0}
      },
      { // B
        {URF, UFL, UBR, DRB, DFR, DLF, ULB, DBL},
        {UR, UF, UL, BR, DR, DF, DL, BL, FR, FL, UB, DB},
        {0, 0, 1, 2, 0, 0, 2, 1},
        {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1}
      }
    };

    for (int ax = 0; ax < 3; ax++) {
      int i1 = 15 * ax; // index to start first face moves
      int i2 = 15 * ax + 3; // index to start of second face moves
      int i3 = 15 * ax + 6; // index to start of axial moves

      int f1 = 2 * ax; // first face
      int f2 = 2 * ax + 1; // second face

      for (int cnt = 0; cnt < 3; cnt++) {
        int m = i1 + cnt;
        names1[m] = fnames[f1] + pnames[cnt];
        if (cnt == 0)
          cubes1[m] = fcubes[f1];
        else
          cubie::mul(cubes1[m - 1], fcubes[f1], cubes1[m]);
        inv1[m] = i1 + (2 - cnt);
        next1[m] |= mask(0x7) << i1; // block any moves on same face
        #ifdef AX
          next1[m] |= mask(0x38) << i1; // block also moves on opposite face
        #endif
        #ifdef QT
          if (cnt == 0) { // block all axial moves but the ones with M
            next1[m] |= mask(0x1ff ^ (0x7 << 3 * cnt)) << i3;
            continue;
          }
        #endif
        next1[m] |= mask(0x1ff) << i3; // block all axial moves
      }
      for (int cnt = 0; cnt < 3; cnt++) {
        int m = i2 + cnt;
        names1[m] = fnames[f2] + pnames[cnt];
        if (cnt == 0)
          cubes1[m] = fcubes[f2];
        else
          cubie::mul(cubes1[m - 1], fcubes[f2], cubes1[m]);
        inv1[m] = i2 + (2 - cnt);
        next1[m] |= mask(0x3f) << i1; // block all simple moves on both faces
        #ifdef QT
          if (cnt == 0) { // block all axial moves but the ones with M
            next1[m] |= mask(0x1ff ^ (0x49 << cnt)) << i3; // 0x49 == 0b1001001
            continue;
          }
        #endif
        next1[m] |= mask(0x1ff) << i3;
      }
      for (int cnt1 = 0; cnt1 < 3; cnt1++) {
        for (int cnt2 = 0; cnt2 < 3; cnt2++) {
          int m = i3 + 3 * cnt1 + cnt2;
          names1[m] = "(" + names1[i1 + cnt1] + " " + names1[i2 + cnt2] + ")";
          cubie::mul(cubes1[i1 + cnt1], cubes1[i2 + cnt2], cubes1[m]);
          inv1[m] = i3 + 3 * (2 - cnt1) + (2 - cnt2);
          next1[m] |= mask(0x7fff) << 15 * ax; // block all simple and axial moves
        }
      }

      qt_skip1[i1] |= bit(i1);
      qt_skip1[i1] |= bit(i3);
      qt_skip1[i2] |= bit(i2);
      qt_skip1[i2] |= bit(i3);
      qt_skip1[i3] |= bit(i3);
    }
    // Half-slice moves commute
    next1[25] |= bit(10);
    next1[40] |= bit(10) | bit(25);
    #ifdef QT
    // Allow repetitions of purely clockwise moves
      for (int m : {0, 3, 6, 15, 18, 21, 30, 33, 36})
        next1[m] ^= bit(m);
    #endif
    // Was built by blocking moves, but should actually indicate permitted ones
    for (int m = 0; m < 45; m++)
      next1[m] = ~next1[m];

    for (int m = 0; m < 45; m++) {
      if (map[m] == -1)
        continue;
      int i = map[m];

      names[i] = names1[m];
      cubes[i] = cubes1[m];
      inv[i] = map[inv1[m]];
      next[i] = reindex(next1[m]);
      qt_skip[i] = reindex(qt_skip1[m]);
    }

    #ifdef QT
      // Unmapped moves are skipped automatically during reindexing
      p1mask &= ~0x10482090000; // 000010000 010010 000010000 010010 000000000 000000
    #endif
    #ifdef F5
      mask tmp = ~(mask(0xfff) << 33);
      p1mask &= tmp;
      p2mask &= tmp;
    #endif
    p1mask = reindex(p1mask);
    p2mask = reindex(p2mask);

    cubie::cube c;
    for (int m1 = 0; m1 < 45; m1++) {
      for (int m2 = 0; m2 < 45; m2++) {
        merge[m1][m2] = -1;
        cubie::mul(cubes1[m1], cubes1[m2], c);
        for (int i = 0; i < 45; i++) {
          if (c == cubes1[i]) {
            merge[m1][m2] = i;
            break;
          }
        }
      }
    }
  }

  void compress1(const std::vector<int>& mseq, std::vector<int>& into) {
    into.clear();
    for (int m : mseq) {
      m = unmap[m];
      if (into.size() == 0 || merge[into.back()][m] == -1)
        into.push_back(m);
      else {
        int tmp = into.back();
        into.pop_back();
        into.push_back(merge[tmp][m]);
      }
    }
  }

  std::string compress(const std::vector<int>& mseq) {
    std::vector<int> comp;
    compress1(mseq, comp);

    // Faster string building probably not worth it in a function like this
    std::string s;
    for (int i = 0; i < comp.size(); i++) {
      s += names1[comp[i]];
      if (i != comp.size() - 1)
        s += " ";
    }
    return s;
  }

  int len(const std::vector<int>& mseq, int cost[]) {
    std::vector<int> comp;
    compress1(mseq, comp);

    int res = 0;
    for (int m : comp)
      res += cost[m];
    return res;
  }

  int len_ht(const std::vector<int>& mseq) {
    int cost[] = {
      1, 1, 1, 1, 1, 1,
      2, 2, 2, 2, 2, 2, 2, 2, 2,
      1, 1, 1, 1, 1, 1,
      2, 2, 2, 2, 2, 2, 2, 2, 2,
      1, 1, 1, 1, 1, 1,
      2, 2, 2, 2, 2, 2, 2, 2, 2
    };
    return len(mseq, cost);
  }

  int len_axht(const std::vector<int>& mseq) {
    int cost[] = {
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1
    };
    return len(mseq, cost);
  }

  int len_qt(const std::vector<int>& mseq) {
    int cost[] = {
      1, 2, 1, 1, 2, 1,
      2, 3, 2, 3, 4, 3, 2, 3, 2,
      1, 2, 1, 1, 2, 1,
      2, 3, 2, 3, 4, 3, 2, 3, 2,
      1, 2, 1, 1, 2, 1,
      2, 3, 2, 3, 4, 3, 2, 3, 2
    };
    return len(mseq, cost);
  }

  int len_axqt(const std::vector<int>& mseq) {
    int cost[] = {
      1, 2, 1, 1, 2, 1,
      1, 2, 1, 2, 2, 2, 1, 2, 1,
      1, 2, 1, 1, 2, 1,
      1, 2, 1, 2, 2, 2, 1, 2, 1,
      1, 2, 1, 1, 2, 1,
      1, 2, 1, 2, 2, 2, 1, 2, 1
    };
    return len(mseq, cost);
  }

}
