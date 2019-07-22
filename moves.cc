#include "moves.h"
#include "cubie.h"

#ifdef FACES5
  #define N_SIMPLE 15
#else
  #define N_SIMPLE 18
#endif

int moves2[N_MOVES2];

std::string move_names[N_MOVES] = {
  "U", "U2", "U'",
  "R", "R2", "R'",
  "F", "F2", "F'",
  "D", "D2", "D'",
  "L", "L2", "L'",
  #ifndef FACES5
    "B", "B2", "B'",
  #endif
};

int inv_move[N_MOVES];
MoveSet skip_moves[N_MOVES];
int qtm[N_MOVES];
int axis[N_MOVES];

CubieCube move_cubes[N_MOVES];

static bool init() {
  move_cubes[0] = kUCube;
  move_cubes[3] = kRCube;
  move_cubes[6] = kFCube;
  move_cubes[9] = kDCube;
  move_cubes[12] = kLCube;
  #ifndef FACES5
      move_cubes[15] = kBCube;
  #endif

  int m = 0;
  int j = 0;
  std::fill(qtm, qtm + N_MOVES, 0);

  for (int ax = 0; ax < N_SIMPLE; ax += 3) {
    for (int i = ax; i < ax + 3; i++)
      skip_moves[m] |= 1 << i;
    for (int i = N_SIMPLE + (ax % 3) * 9; i < N_SIMPLE + (ax % 3 + 1) * 9; i++)
      skip_moves[m] |= 1 << i;
    if (ax > 3) {
      for (int i = ax - 3 * 3; i < ax - 3 * 3 + 3; i++)
        skip_moves[m] |= 1 << i;
    }

    for (int pow = 0; pow < 3; pow++) {
      inv_move[m] = ax + (2 - pow);
      if (pow > 0)
        mul(move_cubes[ax], move_cubes[ax + (pow - 1)], move_cubes[m]);
      if (ax / 3 % 3 == 0 || pow == 1)
        moves2[j++] = m;
      if (pow == 1)
        qtm[m] = 2;
      axis[m] = ax;
      m++;
    }
  }
  for (int i = 0; i < N_SIMPLE; i += 3) {
    for (int j = 1; j < 3; j++)
      skip_moves[i + j] = skip_moves[i];
  }

  #ifdef AXIAL
    for (int ax1 = 0; ax1 <= N_SIMPLE / 2 - 3; ax1 += 3) {
      int ax2 = ax1 + 3 * 3;

      for (int i = ax1; i < ax1 + 3; i++)
        skip_moves[m] |= 1 << i;
      for (int i = ax2; i < ax2 + 3; i++)
        skip_moves[m] |= 1 << i;
      for (int i = N_SIMPLE + (ax1 / 3) * 9; i < N_SIMPLE + ((ax1 / 3) + 1) * 9; i++)
        skip_moves[m] |= 1 << i;

      for (int pow1 = 0; pow1 < 3; pow1++) {
        for (int pow2 = 0; pow2 < 3; pow2++) {
          move_names[m] = move_names[ax1 + pow1] + " " + move_names[ax2 + pow2];
          inv_move[m] = N_SIMPLE + (ax1 / 3) * 9 + 3 * (2 - pow1) + (2 - pow2);
          mul(move_cubes[ax1 + pow1], move_cubes[ax2 + pow2], move_cubes[m]);
          if (ax1 == 0 || (pow1 == 1 && pow2 == 1))
            moves2[j++] = m;
          if (pow1 == 1 || pow2 == 1)
            qtm[m] = 2;
          m++;
        }
      }
    }
    for (int i = N_SIMPLE; i < N_MOVES; i += 9) {
      for (int j = 1; j < 9; j++)
        skip_moves[i + j] = skip_moves[i];
    }
  #endif

  #ifdef QTM
    for (int m = 0; m < N_MOVES; m++)
      skip_moves[m] ^= 1 << m;
  #endif
  for (int i = 1; i < N_MOVES2; i++) {
    if (moves2[i] - moves2[i - 1] == 1 || qtm[moves2[i]] == 0)
      continue;
    qtm[moves2[i]] = 1;
  }
  for (int ax = N_SIMPLE; ax <  N_MOVES; ax += 9) {
    for (int i = 0; i < 9; i++)
      axis[ax + i] = ax;
  }

  return true;
}
static bool inited = init();
