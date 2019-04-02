#include "coord.h"

#include "cubie.h"
#include "moves.h"

coord twist_move[N_TWIST_COORDS][N_MOVES];

coord getTwist(const CubieCube &cube) {
    return 0;
}

void setTwist(CubieCube &cube, coord val) {
}

void initTwistMove() {
  CubieCube cube1;
  CubieCube cube2;

  for (coord c = 0; c < N_TWIST_COORDS; c++) {
    setTwist(cube1, c);
    for (move m = 0; m < N_MOVES; m++) {
        mulCorners(cube1, move_cubes[m], cube2);
        twist_move[c][m] = getTwist(cube2);
    }
  }
}

