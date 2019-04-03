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
Coord (*udedges_move)[N_MOVES_P2];
Coord (*corners_move)[N_MOVES];

Coord (*merge_uedges_dedges)[N_DEDGES_COORDS];

Coord getOriCoord(const int oris[], int len, int n_oris) {
  Coord val = 0;
  for (int i = 0; i < len - 1; i++)
    val = 3 * val + oris[i];
  return val;
}

Coord getPermCoord(int cubies[], int len, int min_cubie) {
  Coord val = 0;

  for (int i = len - 1; i > 0; i--) {
    int n_rots = 0;
    while (cubies[i] != min_cubie) {
      int first = cubies[0];
      for (int j = 0; j < i; j++)
        cubies[j] = cubies[j + 1];
      cubies[i] = first;
      n_rots++;
    }
    val = (val + n_rots) * i;
    min_cubie++;
  }

  return val;
}

Coord getPosPermCoord(
  const int cubies[], int n_cubies, int min_cubie, int max_cubie, bool from_left
) {
  return 0;
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

void setPermCoord(Coord val, int cubies[], int len, int min_cubie) {
  for (int i = 0; i < len; i++) {
    cubies[i] = min_cubie;
    min_cubie++;
  }

  for (int i = 1; i < len; i++) {
    int n_rots = val % (i + 1);
    while (n_rots-- > 0) {
      int last = cubies[i];
      for (int j = i; j > 0; j--)
        cubies[j] = cubies[j - 1];
      cubies[0] = last;
    }
    val /= i + 1;
  }
}

void initMoveCoord(
  Coord (*coord_move)[N_MOVES], 
  int n_coords,
  Coord (*getCoord)(CubieCube &),
  void (*setCoord)(CubieCube &, Coord),
  void (*mul)(const CubieCube &, const CubieCube &, CubieCube &)
) {
  coord_move = new Coord[n_coords][N_MOVES];

  CubieCube cube1;
  CubieCube cube2;

  for (Coord c = 0; c < n_coords; c++) {
    setCoord(cube1, c);
    for (Move m = 0; m < N_MOVES; m++) {
      mul(cube1, move_cubes[m], cube2);
      coord_move[c][m] = getCoord(cube2);
    }
  }
}

Coord getTwist(CubieCube &cube) {
  return getOriCoord(cube.co, N_CORNERS, 3);
}

Coord getFlip(CubieCube &cube) {
  return getOriCoord(cube.eo, N_EDGES, 2);
}

Coord getUDEdges(CubieCube &cube) {
  return getPermCoord(cube.ep, N_EDGES - 4, 0);
}

Coord getCorners(CubieCube &cube) {
  return getPermCoord(cube.cp, N_CORNERS, 0);
}

void setTwist(CubieCube &cube, Coord twist) {
  setOriCoord(twist, cube.co, N_CORNERS, 3);
}

void setFlip(CubieCube &cube, Coord flip) {
  setOriCoord(flip, cube.eo, N_EDGES, 2);
}

void setUDEdges(CubieCube &cube, Coord udedges) {
  setPermCoord(udedges, cube.ep, N_EDGES - 4, 0);
}

void setCorners(CubieCube &cube, Coord corners) {
  setPermCoord(corners, cube.cp, N_CORNERS, 0);
}

void setEdges(CubieCube &cube, Coord edges) {
  setPermCoord(edges, cube.ep, N_EDGES, 0);
}

void initTwistMove() {
  initMoveCoord(
    twist_move, N_TWIST_COORDS, getTwist, setTwist, mulCorners
  );
}

void initFlipMove() {
  initMoveCoord(
    flip_move, N_FLIP_COORDS, getFlip, setFlip, mulEdges
  );
}

void initUDEdgesMove() { 
  udedges_move = new Coord[N_UDEDGES_COORDS_P2][N_MOVES_P2];

  CubieCube cube1;
  CubieCube cube2;

  for (Coord c = 0; c < N_UDEDGES_COORDS_P2; c++) {
    setUDEdges(cube1, c);
    for (Move m = 0; m < N_MOVES_P2; m++) {
      mulEdges(cube1, move_cubes[kPhase2Moves[m]], cube2);
      udedges_move[c][m] = getUDEdges(cube2);
    }
  }
}

void initCornersMove() {
  initMoveCoord(
    corners_move, N_CORNERS_COORDS, getCorners, setCorners, mulCorners
  );
}

