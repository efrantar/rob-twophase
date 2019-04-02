#include "moves.h"
#include "cubie.h"

CubieCube move_cubes[N_MOVES];

void initMoveCubes() {
  move_cubes[U1] = kUCube;
  move_cubes[R1] = kRCube;
  move_cubes[F1] = kFCube;
  move_cubes[D1] = kDCube;
  move_cubes[L1] = kLCube;
  move_cubes[B1] = kBCube;

  for (int i = 0; i < N_MOVES; i += 3) {
    mulCorners(move_cubes[i], move_cubes[i], move_cubes[i + 1]);
    mulCorners(move_cubes[i + 1], move_cubes[i], move_cubes[i + 2]);
  }
}

