#include "moves.h"
#include "cubie.h"

CubieCube move_cubes[N_MOVES];

static bool init() {
  move_cubes[U1] = kUCube;
  move_cubes[R1] = kRCube;
  move_cubes[F1] = kFCube;
  move_cubes[D1] = kDCube;
  move_cubes[L1] = kLCube;
  #ifndef FACES5
    move_cubes[B1] = kBCube;
  #endif

  for (int i = 0; i < N_MOVES; i += 3) {
    mul(move_cubes[i], move_cubes[i], move_cubes[i + 1]);
    mul(move_cubes[i + 1], move_cubes[i], move_cubes[i + 2]);
  }

  return true;
}
static bool inited = init();
