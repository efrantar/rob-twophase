#include "coord.h"

#include "cubie.h"
#include "misc.h"
#include "moves.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <stdint.h>

Coord (*twist_move)[N_MOVES];
Coord (*flip_move)[N_MOVES];
Coord (*slicesorted_move)[N_MOVES];
Coord (*uedges_move)[N_MOVES];
Coord (*dedges_move)[N_MOVES];
Coord (*udedges_move)[N_MOVES_P2];
Coord (*corners_move)[N_MOVES];

Coord (*merge_udedges)[24];

Coord getOriCoord(const int oris[], int len, int n_oris) {
  Coord val = 0;
  for (int i = 0; i < len - 1; i++)
    val = n_oris * val + oris[i];
  return val;
}

Coord getPermCoord(int cubies[], int len, int max_cubie) {
  Coord val = 0;

  for (int i = len - 1; i > 0; i--) {
    int n_rots = 0;
    while (cubies[i] != max_cubie) {
      int first = cubies[0];
      for (int j = 0; j < i; j++)
        cubies[j] = cubies[j + 1];
      cubies[i] = first;
      n_rots++;
    }
    val = (val + n_rots) * i;
    max_cubie--;
  }

  return val;
}

Coord getPosPermCoord(
  const int cubies[], int len, int min_cubie, int max_cubie, bool from_left
) {
  int len1 = max_cubie - min_cubie + 1;
  int cubies1[len1];

  Coord val = 0;
  if (from_left) {
    int j = 0;
    for (int i = 0; i < len; i++) {
      if (min_cubie <= cubies[i] && cubies[i] <= max_cubie) {
        val += cnk[i][j + 1];
        cubies1[j] = cubies[i];
        j++;
      }
    }
  } else {
    int j = len1 - 1;
    for (int i = 0; i < len; i++) {
      if (min_cubie <= cubies[i] && cubies[i] <= max_cubie) {
        val += cnk[len - 1 - i][j + 1];
        cubies1[len1 - 1 - j] = cubies[i];
        j--;
      }
    }
  }

  return fac[len1] * val + getPermCoord(cubies1, len1, max_cubie);
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

void setPermCoord(uint64_t val, int cubies[], int len, int max_cubie) {
  for (int i = len - 1; i >= 0; i--) {
    cubies[i] = max_cubie;
    max_cubie--;
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

void setPosPermCoord(
  Coord val, int cubies[], int len, int min_cubie, int max_cubie, bool from_left
) {
  int len1 = max_cubie - min_cubie + 1;
  int cubies1[len1];
  setPermCoord(val % fac[len1], cubies1, len1, max_cubie);
  val /= fac[len1];

  int j = len1 - 1;
  if (from_left) {
    for (int i = len - 1; i >= 0; i--) {
      int tmp = cnk[i][j + 1];
      if (val - tmp >= 0) {
        cubies[i] = cubies1[j];
        val -= tmp;
        j--;
      } else
        cubies[i] = -1;
    }
  } else {
    for (int i = 0; i < len; i++) {
      int tmp = cnk[len - 1 - i][j + 1];
      if (val - tmp >= 0) {
        cubies[i] = cubies1[len1 - 1 - j];
        val -= tmp;
        j--;
      } else
        cubies[i] = -1;
    }
  }
}

void initMoveCoord(
  Coord (**coord_move)[N_MOVES], 
  int n_coords,
  Coord (*getCoord)(CubieCube &),
  void (*setCoord)(CubieCube &, Coord),
  void (*mul)(const CubieCube &, const CubieCube &, CubieCube &)
) {
  Coord (*coord_move1)[N_MOVES] = new Coord[n_coords][N_MOVES];

  CubieCube cube1;
  CubieCube cube2;

  for (Coord c = 0; c < n_coords; c++) {
    setCoord(cube1, c);
    for (int m = 0; m < N_MOVES; m++) {
      mul(cube1, move_cubes[m], cube2);
      coord_move1[c][m] = getCoord(cube2);
    }
  }

  *coord_move = coord_move1;
}

Coord getTwist(CubieCube &cube) {
  return getOriCoord(cube.co, N_CORNERS, 3);
}

Coord getFlip(CubieCube &cube) {
  return getOriCoord(cube.eo, N_EDGES, 2);
}

Coord getSliceSorted(CubieCube &cube) {
  return getPosPermCoord(cube.ep, N_EDGES, FR, BR, false);
}

Coord getUEdges(CubieCube &cube) {
  return getPosPermCoord(cube.ep, N_EDGES, UR, UB, true);
}

Coord getDEdges(CubieCube &cube) {
  return getPosPermCoord(cube.ep, N_EDGES, DR, DB, true);
}

Coord getUDEdges(CubieCube &cube) {
  return getPermCoord(cube.ep, N_EDGES - 4, DB);
}

Coord getCorners(CubieCube &cube) {
  return getPermCoord(cube.cp, N_CORNERS, N_CORNERS - 1);
}

Coord getSlice(CubieCube &cube) {
  Coord val = 0;

  int j = 3;
  for (int i = 0; i < N_EDGES; i++) {
    if (FR <= cube.ep[i] && cube.ep[i] <= BR) {
      val += cnk[N_EDGES - 1 - i][j + 1];
      j--;
    }
  }

  return val;
}

void setTwist(CubieCube &cube, Coord twist) {
  setOriCoord(twist, cube.co, N_CORNERS, 3);
}

void setFlip(CubieCube &cube, Coord flip) {
  setOriCoord(flip, cube.eo, N_EDGES, 2);
}

void setSliceSorted(CubieCube &cube, Coord slicesorted) {
  setPosPermCoord(slicesorted, cube.ep, N_EDGES, FR, BR, false);
}

void setUEdges(CubieCube &cube, Coord uedges) {
  setPosPermCoord(uedges, cube.ep, N_EDGES, UR, UB, true);
}

void setDEdges(CubieCube &cube, Coord dedges) {
  setPosPermCoord(dedges, cube.ep, N_EDGES, DR, DB, true);
}

void setUDEdges(CubieCube &cube, Coord udedges) {
  setPermCoord(udedges, cube.ep, N_EDGES - 4, DB);
}

void setCorners(CubieCube &cube, Coord corners) {
  setPermCoord(corners, cube.cp, N_CORNERS, N_CORNERS - 1);
}

void setSlice(CubieCube &cube, Coord slice) {
  int j = 3;
  for (int i = 0; i < N_EDGES; i++) {
    int tmp = cnk[N_EDGES - 1 - i][j + 1];
    if (slice - tmp >= 0) {
      cube.ep[i] = FR + j;
      slice -= tmp;
      j--;
    } else
      cube.ep[i] = i - (3 - j);
  }
}

void setEdges(CubieCube &cube, uint64_t edges) {
  setPermCoord(edges, cube.ep, N_EDGES, N_EDGES - 1);
}

void initTwistMove() {
  initMoveCoord(
    &twist_move, N_TWIST_COORDS, getTwist, setTwist, mulCorners
  );
}

void initFlipMove() {
  initMoveCoord(
    &flip_move, N_FLIP_COORDS, getFlip, setFlip, mulEdges
  );
}

void initSliceSortedMove() {
  initMoveCoord(
    &slicesorted_move, N_SLICESORTED_COORDS, getSliceSorted, setSliceSorted, mulEdges
  );
}

void initUEdgesMove() {
  initMoveCoord(
    &uedges_move, N_UEDGES_COORDS, getUEdges, setUEdges, mulEdges
  );
}

void initDEdgesMove() {
  initMoveCoord(
    &dedges_move, N_DEDGES_COORDS, getDEdges, setDEdges, mulEdges
  );
}

void initUDEdgesMove() { 
  udedges_move = new Coord[N_UDEDGES_COORDS_P2][N_MOVES_P2];

  CubieCube cube1;
  CubieCube cube2;

  for (Coord c = 0; c < N_UDEDGES_COORDS_P2; c++) {
    setUDEdges(cube1, c);
    for (int m = 0; m < N_MOVES_P2; m++) {
      mulEdges(cube1, move_cubes[kPhase2Moves[m]], cube2);
      udedges_move[c][m] = getUDEdges(cube2);
    }
  }
}

void initCornersMove() {
  initMoveCoord(
    &corners_move, N_CORNERS_COORDS, getCorners, setCorners, mulCorners
  );
}

void initMergeUDEdges() {
  merge_udedges = new Coord[N_UEDGES_COORDS_P2][24];

  CubieCube cube;
  for (Coord c = 0; c < N_UDEDGES_COORDS_P2; c++) {
    setUDEdges(cube, c);
    
    int dedges[4];
    int j = 0;
    for (int i = 0; i < 8; i++) {
      if (DR <= cube.ep[i] && cube.ep[i] <= DB) {
        dedges[j] = cube.ep[i];
        j++;
      }
    }

    merge_udedges[getUEdges(cube)][getPermCoord(dedges, 4, DB)] = c;
  }
}

Coord sliceMove(Coord slice, int move) {
  return SS_SLICE(slicesorted_move[SLICESORTED(slice)][move]);
}

