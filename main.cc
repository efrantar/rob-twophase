#include <iostream>
#include <stdlib.h>

#include "cubie.h"
#include "misc.h"
#include "moves.h"
#include "coord.h"

void testCoord(int n_coords, Coord (*get)(CubieCube &), void (*set)(CubieCube &, Coord)) {
  CubieCube cube;
  for (Coord c = 0; c < n_coords; c++) {
    set(cube, c);
    if (get(cube) != c) {
      std::cout << "error: " << c << "\n";
      return;
    }
  }
  std::cout << "ok\n";
}

void testCoords() {
  std::cout << "Testing coords ...\n";
  testCoord(N_TWIST_COORDS, getTwist, setTwist);
  testCoord(N_FLIP_COORDS, getFlip, setFlip);
  testCoord(N_SLICE_COORDS, getSlice, setSlice);
  testCoord(N_UEDGES_COORDS, getUEdges, setUEdges);
  testCoord(N_DEDGES_COORDS, getDEdges, setDEdges);
  testCoord(N_UDEDGES_COORDS_P2, getUDEdges, setUDEdges);
  testCoord(N_CORNERS_COORDS, getCorners, setCorners);
}

int main() {
  initMisc();
  testCoords();
  return 0;
}

