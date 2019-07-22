#include "coord.h"

#include <algorithm>
#include <bitset>
#include <stdint.h>
#include "cubie.h"
#include "moves.h"

#define N_PERM4 24
#define N_C12K4 495

Coord (*twist_move)[N_MOVES];
Coord (*flip_move)[N_MOVES];
Coord (*sslice_move)[N_MOVES];
Coord (*uedges_move)[N_MOVES];
Coord (*dedges_move)[N_MOVES];
Coord (*udedges_move2)[N_MOVES2];
Coord (*cperm_move)[N_MOVES];

// Don't waste space here as we want these tables to fit into the cache
uint8_t enc_perm4[1 << (4 * 2)];
uint8_t dec_perm4[N_PERM4];
uint16_t enc_comb4[1 << N_EDGES];
uint16_t dec_comb4[N_C12K4];

bool init() {
  int perm[] = {0, 1, 2, 3};
  for (int i = 0; i < N_PERM4; i++) {
    int tmp = 0;
    for (int j = 3; j >= 0; j--) // lowest index -> least significant bits
      tmp = (tmp << 2) | perm[j];
    enc_perm4[tmp] = i;
    dec_perm4[i] = tmp;
    std::next_permutation(perm, perm + 4);
  }

  int j = 0;
  for (int i = 0; i < (1 << N_EDGES); i++) {
    if (std::bitset<N_EDGES>(i).count() == 4) {
      enc_comb4[i] = j;
      dec_comb4[j] = i;
      j++;
    }
  }
}
static bool inited = init();

Coord getOri(const int *oris, int len, int n_oris) {
  Coord val = 0;
  for (int i = 0; i < len - 1; i++) // last value can be reconstructed by parity
    val = n_oris * val + oris[i];
  return val;
}

// `inv` is needed so that the comb mask 0xf00 is mapped to 0 for SLICE coords
int getEdges4(const int edges[], int mask, int min_edge, bool inv) {
  int comb = 0;
  int perm = 0;

  for (int i = N_EDGES - 1; i >= 0; i--) { // solved permutation should lead to 0 coord
    if ((mask & (1 << edges[i])) != 0) {
      comb |= (1 << i);
      perm = (perm << 2) | (edges[i] - min_edge);
    }
  }

  return N_PERM4 * (inv ? N_C12K4 - enc_comb4[comb] - 1 : enc_comb4[comb]) + enc_perm4[perm];
}

Coord getPerm8(const int perm[]) {
  int comb = 0;
  int perm1 = 0;
  int perm2 = 0;

  for (int i = 7; i >= 0; i--) {
    if (perm[i] < 4) {
      perm1 = (perm1 << 2) | perm[i];
      comb |= 1 << i;
    }
    else
      perm2 = (perm2 << 2) | (perm[i] - 4);
  }

  return N_PERM4 * (N_PERM4 * enc_comb4[comb] + enc_perm4[perm1]) + enc_perm4[perm2];
}

void setOri(Coord val, int *oris, int len, int n_oris) {
  int parity = 0;
  for (int i = len - 2; i >= 0; i--) {
    oris[i] = val % n_oris;
    parity += oris[i];
    val /= n_oris;
  }
  oris[len - 1] = (n_oris - parity % n_oris) % n_oris;
}

int setEdges4(Coord val, int edges[], int min_edge, bool inv) {
  // N_C12K4 - (N_C12K4 - val - 1) - 1 = val
  int comb = dec_comb4[inv ? N_C12K4 - val / N_PERM4 - 1 : val / N_PERM4];
  int perm = dec_perm4[val % N_PERM4];

  int cubie = 0;
  for (int i = 0; i < N_EDGES; i++) {
    if (cubie == min_edge)
      cubie += 4;
    if ((comb & (1 << i)) != 0) {
      edges[i] = (perm & 0x3) + min_edge;
      perm >>= 2;
    } else
      edges[i] = cubie++;
  }
}

void setPerm8(Coord val, int perm[]) {
  int comb = dec_comb4[(val / N_PERM4) / N_PERM4];
  int perm1 = dec_perm4[(val / N_PERM4) % N_PERM4];
  int perm2 = dec_perm4[val % N_PERM4];

  for (int i = 0; i < 8; i++) {
    if ((comb & (1 << i)) != 0) {
      perm[i] = perm1 & 0x3;
      perm1 >>= 2;
    } else {
      perm[i] = (perm2 & 0x3) + 4;
      perm2 >>= 2;
    }
  }
}

Coord getTwist(const CubieCube &cube) {
  return getOri(cube.co, N_CORNERS, 3);
}

Coord getFlip(const CubieCube &cube) {
  return getOri(cube.eo, N_EDGES, 2);
}

Coord getSSlice(const CubieCube &cube) {
  return getEdges4(cube.ep, 0xf00, FR, true);
}

Coord getUEdges(const CubieCube &cube) {
  return getEdges4(cube.ep, 0x00f, UR, false);
}

Coord getDEdges(const CubieCube &cube) {
  return getEdges4(cube.ep, 0x0f0, DR, false);
}

Coord getUDEdges(const CubieCube &cube) {
  return getPerm8(cube.ep);
}

Coord getCPerm(const CubieCube &cube) {
  return getPerm8(cube.cp);
}

void setTwist(CubieCube &cube, Coord twist) {
  setOri(twist, cube.co, N_CORNERS, 3);
}

void setFlip(CubieCube &cube, Coord flip) {
  setOri(flip, cube.eo, N_EDGES, 2);
}

void setSSlice(CubieCube &cube, Coord sslice) {
  setEdges4(sslice, cube.ep, FR, true);
}

void setUEdges(CubieCube &cube, Coord uedges) {
  setEdges4(uedges, cube.ep, UR, false);
}

void setDEdges(CubieCube &cube, Coord dedges) {
  setEdges4(dedges, cube.ep, DR, false);
}

void setUDEdges(CubieCube &cube, Coord udedges) {
  // Make sure result is a valid cube
  for (int i = FR; i < N_EDGES; i++)
    cube.ep[i] = i;
  setPerm8(udedges, cube.ep);
}

void setCPerm(CubieCube &cube, Coord cperm) {
  setPerm8(cperm, cube.cp);
}

void initMoveCoord(
  Coord (**coord_move)[N_MOVES], 
  int n_coords,
  Coord (*getCoord)(const CubieCube &),
  void (*setCoord)(CubieCube &, Coord),
  void (*mul)(const CubieCube &, const CubieCube &, CubieCube &)
) {
  auto coord_move1 = new Coord[n_coords][N_MOVES];

  CubieCube cube1;
  CubieCube cube2;

  cube1 = kSolvedCube;
  for (Coord c = 0; c < n_coords; c++) {
    setCoord(cube1, c);
    for (int m = 0; m < N_MOVES; m++) {
      mul(cube1, move_cubes[m], cube2);
      coord_move1[c][m] = getCoord(cube2);
    }
  }

  *coord_move = coord_move1;
}

void initTwistMove() {
  initMoveCoord(
    &twist_move, N_TWIST, getTwist, setTwist, mulCorners
  );
}

void initFlipMove() {
  initMoveCoord(
    &flip_move, N_FLIP, getFlip, setFlip, mulEdges
  );
}

void initSSliceMove() {
  initMoveCoord(
    &sslice_move, N_SSLICE, getSSlice, setSSlice, mulEdges
  );
}

void initUEdgesMove() {
  initMoveCoord(
    &uedges_move, N_UEDGES, getUEdges, setUEdges, mulEdges
  );
}

void initDEdgesMove() {
  initMoveCoord(
    &dedges_move, N_DEDGES, getDEdges, setDEdges, mulEdges
  );
}

// Only need phase 2 moves here -> code repetition easiest solution (`initCoordMove()` not so easy to adapt)
void initUDEdgesMove2() {
  udedges_move2 = new Coord[N_UDEDGES2][N_MOVES2];

  CubieCube cube1;
  CubieCube cube2;

  cube1 = kSolvedCube;
  for (Coord c = 0; c < N_UDEDGES2; c++) {
    setUDEdges(cube1, c);
    for (int m = 0; m < N_MOVES2; m++) {
      mulEdges(cube1, move_cubes[moves2[m]], cube2);
      udedges_move2[c][m] = getUDEdges(cube2);
    }
  }
}

void initCPermMove() {
  initMoveCoord(
    &cperm_move, N_CPERM, getCPerm, setCPerm, mulCorners
  );
}

Coord sliceMove(Coord slice, int move) {
  return SS_SLICE(sslice_move[SSLICE(slice)][move]);
}
