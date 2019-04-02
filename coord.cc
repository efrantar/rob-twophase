#include "coord.h"

#include "cubie.h"
#include "moves.h"

#include <iostream>

coord (*twist_move)[N_MOVES];
coord (*flip_move)[N_MOVES];
coord (*slice_move)[N_MOVES];
coord (*uedges_move)[N_MOVES];
coord (*dedges_move)[N_MOVES];
coord (*udedges_move)[N_MOVES];
coord (*corners_move)[N_MOVES];

coord (*merge_uedges_dedges)[N_DEDGES_COORDS];

coord getTwist(const CubieCube &cube) {
    coord twist = 0;
    for (int i = 0; i < N_CORNERS - 1; i++)
        twist = 3 * twist + cube.co[i];
    return twist;
}

void setTwist(CubieCube &cube, coord twist) {
    int parity = 0;
    for (int i = N_CORNERS - 2; i >= 0; i--) {
        cube.co[i] = twist % 3;
        parity += cube.co[i];
        twist /= 3;
    }
    cube.co[N_CORNERS - 1] = (3 - parity % 3) % 3;
}

void initTwistMove() {
  twist_move = new coord[N_TWIST_COORDS][N_MOVES];

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

