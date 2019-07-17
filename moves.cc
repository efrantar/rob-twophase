#include "moves.h"
#include "cubie.h"

std::string kMoveNames[MAX_MOVES] = {
  "U", "U2", "U'",
  "R", "R2", "R'",
  "F", "F2", "F'",
  "D", "D2", "D'",
  "L", "L2", "L'",
  "B", "B2", "B'"
};

int kPhase2Moves[MAX_MOVES] = {
  U1, U2, U3, R2, F2, D1, D2, D3, L2, B2
};

int kInvMove[MAX_MOVES] = {
  U3, U2, U1,
  R3, R2, R1,
  F3, F2, F1,
  D3, D2, D1,
  L3, L2, L1,
  B3, B2, B1
};

CubieCube move_cubes[MAX_MOVES];

static bool init() {
  int n_phase1 = 18;
  int n_phase2 = 10;
  #ifdef FACES5
    n_phase1 = 15;
    n_phase2 = 9;
  #endif

  move_cubes[U1] = kUCube;
  move_cubes[R1] = kRCube;
  move_cubes[F1] = kFCube;
  move_cubes[D1] = kDCube;
  move_cubes[L1] = kLCube;
  move_cubes[B1] = kBCube;

  for (int i = 0; i < n_phase1; i += 3) {
    mul(move_cubes[i], move_cubes[i], move_cubes[i + 1]);
    mul(move_cubes[i + 1], move_cubes[i], move_cubes[i + 2]);
  }

  int i = n_phase1;
  int j = n_phase2;
  for (int ax1 = U1; ax1 <= F1; ax1 += 3) {
    #ifdef FACES5
        if (ax1 == F1)
          continue;
    #endif
    int ax2 = 3 * (ax1 + 3);
    for (int pow1 = 0; pow1 < 3; pow1++) {
      for (int pow2 = 0; pow2 < 3; pow2++) {
        kMoveNames[i] = kMoveNames[ax1] + " " + kMoveNames[ax2];
        if (ax1 == U1 || (pow1 != 1 && pow2 != 1))
          kPhase2Moves[j++] = i;
        kInvMove[i] = n_phase1 + (ax1 / 3) * 9 + 3 * (2 - pow1) + (2 - pow2);
        mul(move_cubes[ax1 + pow1], move_cubes[ax2 + pow2], move_cubes[i]);
        i++;
      }
    }
  }

  return true;
}
static bool inited = init();
