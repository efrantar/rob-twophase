#include <iostream>
#include <stdlib.h>

#include "cubie.h"
#include "misc.h"
#include "moves.h"
#include "coord.h"

void initMoves() {
  initMoveCubes();

  initTwistMove();
  initFlipMove();
  initSliceMove();
  initUEdgesMove();
  initDEdgesMove();
  initUDEdgesMove();
  initCornersMove();
}

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

void testCoordMove(Coord coord_move[][N_MOVES], int n_coords) {
  for (Coord c = 0; c < n_coords; c++) {
    for (Move m = 0; m < N_MOVES; m++) {
      if (coord_move[coord_move[c][m]][kInvMove[m]] != c) {
        std::cout << "error: " << c << "\n";
        return;
      }
    }
  }
  std::cout << "ok\n";
}

void testCoordMoveP2(Coord coord_move[][N_MOVES_P2], int n_coords) {
  for (Coord c = 0; c < n_coords; c++) { 
    for (Move m = 0; m < N_MOVES_P2; m++) {
      Move inv = 0;
      while (kPhase2Moves[inv] != kInvMove[kPhase2Moves[m]])
        inv++;
      if (coord_move[coord_move[c][m]][inv] != c) {
        std::cout << "error: " << c << "\n";
        return;
      }
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

void testCoordMoves() {
  std::cout << "Testing coord moves ...\n";
  testCoordMove(twist_move, N_TWIST_COORDS);
  testCoordMove(flip_move, N_FLIP_COORDS);
  testCoordMove(slice_move, N_SLICE_COORDS);
  testCoordMove(uedges_move, N_UEDGES_COORDS);
  testCoordMove(dedges_move, N_DEDGES_COORDS);
  testCoordMoveP2(udedges_move, N_UEDGES_COORDS_P2);
  testCoordMove(corners_move, N_CORNERS_COORDS);
}

int main() {
  initMisc();
  initMoves();

  testCoords();
  testCoordMoves();

  return 0;
}

