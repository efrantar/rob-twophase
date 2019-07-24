#include "moves.h"
#include "cubie.h"

#include <string>

std::string move_names[N_MOVES];
int inv_move[N_MOVES];
int split[N_MOVES];
MoveMask movemasks[N_MOVES + 1];
MoveMask all_movemask = MOVEBIT(N_MOVES) - 1;
MoveMask extra_movemask = 0;

int moves2[N_MOVES2];
CubieCube move_cubes[N_MOVES];

static bool init() {
  #ifdef AXIAL
    int off[] = {12, 27, 42, 0, 15, 30};
  #else
    int off[] = {0, 3, 6, 9, 12, 15};
  #endif

  CubieCube basic_cubes[] = {kUCube, kRCube, kFCube, kDCube, kLCube, kBCube};
  std::string basic_names[] = {"U", "R", "F", "D", "L", "B"};
  int n_moves2 = 0;
  std::fill(movemasks, movemasks + N_MOVES + 1, ~MoveMask(0));

  for (int ax = 0; ax < 6; ax++) {
    int i = off[ax];

    move_cubes[i] = basic_cubes[ax];
    move_names[i] = basic_names[ax];
    inv_move[i] = off[ax] + 2;
    if (ax % 3 == 0)
      moves2[n_moves2++] = i;

    movemasks[off[ax]] &= ~((MoveMask(0x3) << (off[ax])) | (MoveMask(0x3) << (off[(ax + 3) % 3])));
    #ifdef AXIAL
      movemasks[i] &= ~(MoveMask(0x1ff) << (off[ax % 3 + 3] + 3));
    #endif
    if (ax % 3 != 0) {
      extra_movemask |= MOVEBIT(off[ax] + 1);
      split[off[ax] + 1] = off[ax];
    }

    i++;
    for (int pow = 1; pow < 3; pow++) {
      mul(basic_cubes[ax], move_cubes[off[ax] + pow - 1], move_cubes[i]);
      move_names[i] = basic_names[ax] + (pow == 2 ? "'" : "2");
      inv_move[i] = off[ax] + (2 - pow);
      movemasks[i] = movemasks[off[ax]];

      if (ax % 3 == 0 || pow == 1)
        moves2[n_moves2++] = i;

      i++;
    }

    #ifdef QUARTER
      all_movemask &= ~MOVEBIT(off[ax] + 1);
    #endif
  }
  #ifdef FACES5
    all_movemask &= ~(MoveMask(0x7) << off[5]);
    extra_movemask ^= MOVEBIT(off[5] + 1);
  #endif

  #ifdef AXIAL
    for (int ax1 = 0; ax1 < 3; ax1++) {
      int ax2 = ax1 + 3;
      int i = off[ax2] + 3;
      movemasks[i] &= ~(MoveMask(0x7fff) << off[ax2]);

      for (int pow1 = 0; pow1 < 3; pow1++) {
        int i1 = off[ax1] + pow1;
        for (int pow2 = 0; pow2 < 3; pow2++) {
          int i2 = off[ax2] + pow2;

          mul(move_cubes[i1], move_cubes[i2], move_cubes[i]);
          move_names[i] = move_names[i1] + " " + move_names[i2];
          inv_move[i] = off[ax2] + 3 + 3 * (2 - pow1) + (2 - pow2);

          movemasks[i] = movemasks[off[ax2] + 3];
          #ifdef QUARTER
            if (pow1 == 1 || pow2 == 1)
              all_movemask &= ~MOVEBIT(i);
            else {
              movemasks[i1] |= MOVEBIT(i);
              movemasks[i2] |= MOVEBIT(i);
            }
          #endif
          if (ax1 != 0 && pow1 == 1 && pow2 == 1) {
            extra_movemask |= MOVEBIT(i);
            split[i] = i - 4;
          }

          if (ax1 == 0 || (pow1 == 1 && pow2 == 1))
            moves2[n_moves2++] = i;

          i++;
        }
      }
    }
    #ifdef FACES5
      all_movemask &= ~(MoveMask(0x1ff) << (off[5] + 3));
      extra_movemask ^= MOVEBIT(off[5] + 3 + 5);
    #endif
  #endif

  for (int m = 0; m < N_MOVES; m++) {
    #ifdef QUARTER
      movemasks[m] |= MOVEBIT(m);
    #endif
    movemasks[m] &= all_movemask;
  }
  movemasks[N_MOVES] = all_movemask;

  return true;
}
static bool inited = init();
