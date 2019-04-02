#include "coord.h"

#include "cubie.h"
#include "moves.h"

#include <iostream>
#include <vector>

Coord (*twist_move)[N_MOVES];
Coord (*flip_move)[N_MOVES];
Coord (*slice_move)[N_MOVES];
Coord (*uedges_move)[N_MOVES];
Coord (*dedges_move)[N_MOVES];
Coord (*udedges_move)[N_MOVES];
Coord (*corners_move)[N_MOVES];

Coord (*merge_uedges_dedges)[N_DEDGES_COORDS];

const std::vector<Move> kMoves = {
  U1, U2, U3, 
  R1, R2, R3, 
  F1, F2, F3, 
  D1, D2, D3, 
  L1, L2, L3, 
  B1, B2, B3
};

Coord getOriCoord(const int oris[], int len, int n_oris) {
  Coord val = 0;
  for (int i = 0; i < len - 1; i++)
    val = 3 * val + oris[i];
  return val;
}

void setOriCoord(Coord val, int oris[], int len, int n_oris) {
  int parity = 0;
  for (int i = len - 2; i >= 0; i--) {
    oris[i] = val % n_oris;
    parity += oris[i];
    val /= n_oris;
  }
  oris[len - 1] = (n_oris - parity % n_oris) % n_oris;
}

void initMoveCoord(
  Coord (*coord_move)[N_MOVES], 
  int n_coords, 
  std::vector<Move> moves,
  Coord (*getCoord)(const CubieCube &),
  void (*setCoord)(CubieCube &, Coord),
  void (*mul)(const CubieCube &, const CubieCube &, CubieCube &)
) {
  coord_move = new Coord[n_coords][N_MOVES];

  CubieCube cube1;
  CubieCube cube2;

  for (Coord c = 0; c < n_coords; c++) {
    setCoord(cube1, c);
    for (Move m : moves) {
      mul(cube1, move_cubes[m], cube2);
      coord_move[c][m] = getCoord(cube2);
    }
  }
}

Coord getTwist(const CubieCube &cube) {
  return getOriCoord(cube.co, N_CORNERS, 3);
}

Coord getFlip(const CubieCube &cube) {
  return getOriCoord(cube.eo, N_EDGES, 2);
}

void setTwist(CubieCube &cube, Coord twist) {
  setOriCoord(twist, cube.co, N_CORNERS, 3);
}

void setFlip(CubieCube &cube, Coord flip) {
  setOriCoord(flip, cube.eo, N_EDGES, 2);
}

void initTwistMove() {
  initMoveCoord(
    twist_move, N_TWIST_COORDS, kMoves, getTwist, setTwist, mulCorners
  );
}

void initFlipMove() {
  initMoveCoord(
    flip_move, N_FLIP_COORDS, kMoves, getFlip, setFlip, mulEdges
  );
}

