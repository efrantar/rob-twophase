#include "coord.h"

#include <algorithm>
#include <bitset>
#include <stdint.h>
#include "cubie.h"
#include "moves.h"

#define N_PERM4 24 // 4!
#define N_C12K4 495 // binomial(12, 4)

Coord (*twist_move)[N_MOVES];
Coord (*flip_move)[N_MOVES];
Coord (*sslice_move)[N_MOVES];
Coord (*edges4_move)[N_MOVES];
Coord (*udedges_move2)[N_MOVES2];
Coord (*cperm_move)[N_MOVES];

/*
 * To encode 4-element permutations, we represent them as 8-bit integers (2-bits encoding the element at every
 * position) and then index into an array which contains a unique number between 0 and 4! - 1. Decoding works exactly
 * the other way around.
 */
uint8_t enc_perm4[1 << (4 * 2)];
uint8_t dec_perm4[N_PERM4];

/*
 * The positions of 4 edges are encoded by a bitmask of length 12 with exactly 4 ones indicating the positions the
 * edges. These two arrays convert between such a bitmask and a unique number between 0 and binomial(12, 4) - 1.
 */
uint16_t enc_comb4[1 << N_EDGES];
uint16_t dec_comb4[N_C12K4];

void initCoord() {
  int perm[] = {0, 1, 2, 3};
  for (int i = 0; i < N_PERM4; i++) {
    int tmp = 0;
    for (int j = 3; j >= 0; j--) // the lowest index should be the least significant bit
      tmp = (tmp << 2) | perm[j];
    enc_perm4[tmp] = i;
    dec_perm4[i] = tmp;
    std::next_permutation(perm, perm + 4);
  }

  int j = 0;
  for (int i = 0; i < (1 << N_EDGES); i++) {
    // Not the fastest, but doing this ~4000 times is very much negligible compared to the move-table setup
    if (std::bitset<N_EDGES>(i).count() == 4) {
      enc_comb4[i] = j;
      dec_comb4[j] = i;
      j++;
    }
  }
}

// Computes the orientation coordinate
Coord getOri(const int oris[], int len, int n_oris) {
  Coord val = 0;
  for (int i = 0; i < len - 1; i++) // last value can be reconstructed by parity
    val = n_oris * val + oris[i];
  return val;
}

// Encodes the position and permutation of 4 consecutive edges given by the mask
int getEdges4(const int edges[], int mask, int min_edge, bool inv) {
  int comb = 0;
  int perm = 0;

  for (int i = N_EDGES - 1; i >= 0; i--) { // solved permutation should lead to 0 coord
    if ((mask & (1 << edges[i])) != 0) {
      comb |= 1 << i;
      perm = (perm << 2) | (edges[i] - min_edge);
    }
  }

  // For SSLICE we want the bitmask 0xf00 (i.e. phase 1 solved) to be mapped to 0, hence the parameter `inv`
  return N_PERM4 * (inv ? (N_C12K4 - 1) - enc_comb4[comb] : enc_comb4[comb]) + enc_perm4[perm];
}

// Computes the coordinate of an 8-element permutation
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

  // Combine position and permutation of elements 0 - 3 with the permutation of elements 4 - 7
  return N_PERM4 * (N_PERM4 * enc_comb4[comb] + enc_perm4[perm1]) + enc_perm4[perm2];
}

// Decodes an orientation coordinate
void setOri(Coord val, int oris[], int len, int n_oris) {
  int par = 0;
  for (int i = len - 2; i >= 0; i--) {
    oris[i] = val % n_oris;
    par += oris[i];
    val /= n_oris;
  }
  // Reconstruct last orientation by using the fact that the parity % number of orientations must always be 0
  oris[len - 1] = (n_oris - par % n_oris) % n_oris;
}

// Decoded the position and permutation of 4 edges
int setEdges4(Coord val, int edges[], int min_edge, bool inv) {
  // N_C12K4 - 1 - (N_C12K4 - 1 - val) = val
  int comb = dec_comb4[inv ? (N_C12K4 - 1) - val / N_PERM4 : val / N_PERM4];
  int perm = dec_perm4[val % N_PERM4];

  int cubie = 0;
  for (int i = 0; i < N_EDGES; i++) {
    if (cubie == min_edge)
      cubie += 4;
    if ((comb & (1 << i)) != 0) {
      edges[i] = (perm & 0x3) + min_edge;
      perm >>= 2;
    } else
      edges[i] = cubie++; // the other slots should also be filled so that the result is a valid edge permutation
  }
}

// Decodes a permutation of 8 elements
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
  // `inv` must be true so that the position part of SSLICE is 0 in phase 2
  return getEdges4(cube.ep, 0xf00, FR, true);
}

Coord getUEdges(const CubieCube &cube) {
  return getEdges4(cube.ep, 0x00f, UR, false);
}

Coord getDEdges(const CubieCube &cube) {
  return getEdges4(cube.ep, 0x0f0, DR, false);
}

Coord getUDEdges(const CubieCube &cube) {
  return getPerm8(cube.ep); // only the first 8 elements will be considered
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
  // Make sure that the result is always valid edge permutation
  for (int i = FR; i < N_EDGES; i++)
    cube.ep[i] = i;
  setPerm8(udedges, cube.ep);
}

void setCPerm(CubieCube &cube, Coord cperm) {
  setPerm8(cperm, cube.cp); // only the first 8 elements will be touched
}

// Generates the move-table for a coordinate
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

  cube1 = kSolvedCube; // properly initialize all 4 arrays so that multiplication will always work
  for (Coord c = 0; c < n_coords; c++) {
    setCoord(cube1, c);
    for (int m = 0; m < N_MOVES; m++) {
      mul(cube1, move_cubes[m], cube2);
      coord_move1[c][m] = getCoord(cube2);
    }
  }

  *coord_move = coord_move1;
}

void initCoordMove() {
  initMoveCoord(&twist_move, N_TWIST, getTwist, setTwist, mulCorners);
  initMoveCoord(&flip_move, N_FLIP, getFlip, setFlip, mulEdges);
  initMoveCoord(&sslice_move, N_SSLICE, getSSlice, setSSlice, mulEdges);
  // We use UEDGES to initialize the EDGES4 coord (DEDGES would be exactly the same)
  initMoveCoord(&edges4_move, N_EDGES4, getUEdges, setUEdges, mulEdges);
  initMoveCoord(&cperm_move, N_CPERM, getCPerm, setCPerm, mulCorners);

  /* The UDEDGES move-table should only contain phase 2 moves, hence a little code duplication */
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

Coord sliceMove(Coord slice, int move) {
  return SS_SLICE(sslice_move[SSLICE(slice)][move]);
}
