#include "coord.h"

#include <algorithm>
#include <bitset>
#include <cstdint>

#include "cubie.h"
#include "moves.h"

#define N_C12K4 495 // binom(12, 4)
#define N_C8K4 70 // binom(8, 4)
#define N_PERM4 24 // 4!

uint16_t (*move_flip)[N_MOVES];
uint16_t (*move_twist)[N_MOVES];

/*
 * Set of tables used for performing edge and corner coordinate moves. The `*_move` tables store how a move affects
 * the given 4-cubie-combination as well as in the last 5 bits how it affects their permutation. `mul_perms` multiplies
 * two permutations given by their index.
 */
uint16_t edges4_move[N_C12K4][N_MOVES];
uint16_t cperm4_move[N_C8K4][N_MOVES];
uint8_t mul_perms[N_PERM4][N_PERM4];

/*
 * Set of tables used for en- and decoding edge and corner coordinates. `enc_perm` compresses a 4-element permutation
 * encoded as an 8-bit integer with 2 bits used for every element to a number from 0 - 23, `dec_perm` does the inverse.
 * `enc_comb` maps a 4-element combination encoded in form of a bitmask with exactly 4 bits on (indicating where the
 * elements are located) to a number between 0 - 494, `dec_comb` is again for decompression.
 */
uint8_t enc_perm[1 << (4 * 2)];
uint8_t dec_perm[N_PERM4];
uint16_t enc_comb[1 << N_EDGES];
uint16_t dec_comb[N_C12K4];

Edges4::Edges4(int edges4) {
  set(edges4);
}

int Edges4::val() const {
  return N_PERM4 * comb + perm;
}

void Edges4::set(int edges4) {
  comb = edges4 / N_PERM4;
  perm = edges4 % N_PERM4;
}

CPerm::CPerm(int cperm) {
  set(cperm);
}

void CPerm::set(int cperm) {
  comb1 = (cperm / N_PERM4) / N_PERM4;
  perm1 = (cperm / N_PERM4) % N_PERM4;
  // The bitmask of the position of corners DFR to DRB is not redundantly encoded in the coordinate value and must
  // hence be computed explicitly
  comb2 = enc_comb[0xff & ~dec_comb[comb1]];
  perm2 = cperm % N_PERM4;
}

int CPerm::val() const {
  return N_PERM4 * (N_PERM4 * comb1 + perm1) + perm2;
}

// Converts a 4-element permutation into an 8-bit number using 2 bits to represent every element
int binarizePerm4(int perm4[]) {
  int bin = 0;
  for (int i = 3; i >= 0; i--)
    bin = (bin << 2) | perm4[i];
  return bin;
}

// Inverse of `binarizePerm4()`
void unbinarizePerm4(int perm4[], int bin) {
  for (int i = 0; i < 4; i++) {
    perm4[i] = 0x3 & bin;
    bin >>= 2;
  }
}

// Initializes the `enc_*`, `dec_*` and `mul_perms` arrays
void initCoord() {
  int perm[] = {0, 1, 2, 3};
  for (int i = 0; i < N_PERM4; i++) {
    int tmp = binarizePerm4(perm);
    enc_perm[tmp] = i;
    dec_perm[i] = tmp;
    std::next_permutation(perm, perm + 4);
  }

  int j = 0;
  for (int i = 0; i < (1 << N_EDGES); i++) {
    if (std::bitset<N_EDGES>(i).count() == 4) {
      enc_comb[i] = j;
      dec_comb[j] = i;
      j++;
    }
  }

  int perm1[4];
  int perm2[4];
  int perm3[4];
  for (int i = 0; i < N_PERM4; i++) {
    unbinarizePerm4(perm1, dec_perm[i]);
    for (int j = 0; j < N_PERM4; j++) {
      unbinarizePerm4(perm2, dec_perm[j]);
      for (int k = 0; k < 4; k++)
        perm3[k] = perm1[perm2[k]];
      mul_perms[i][j] = enc_perm[binarizePerm4(perm3)];
    }
  }
}

// Computes an orientation coordinate
int getOri(const int oris[], int len, int n_oris) {
  int val = 0;
  for (int i = 0; i < len - 1; i++) // skip the last orientation as it can be reconstructed by parity
    val = n_oris * val + oris[i];
  return val;
}

// Decodes an orientation coordinate
void setOri(int val, int oris[], int len, int n_oris) {
  int par = 0;
  for (int i = len - 2; i >= 0; i--) {
    oris[i] = val % n_oris;
    par += oris[i];
    val /= n_oris;
  }
  // Reconstruct last element by using the fact that the orientation parity (for a solvable cube) must always be 0
  oris[len - 1] = (n_oris - par % n_oris) % n_oris;
}

// Computes the combination and permutation coordinate for the set of 4 cubies indicated by the bitmask `mask` with
// `min_cubie` being the one of minimal value
void getCombPerm(int &comb, int &perm, const int cubies[], int len, int mask, int min_cubie) {
  comb = 0;
  perm = 0;

  for (int i = len - 1; i >= 0; i--) {
    if ((mask & (1 << cubies[i])) != 0) {
      comb |= 1 << i;
      perm = (perm << 2) | (cubies[i] - min_cubie);
    }
  }

  comb = enc_comb[comb];
  perm = enc_perm[perm];
}

// Decodes a combination and permutation coordinate for the 4 consecutive cubies starting with `min_cubie`. `fill`
// indicates whether or not the unspecified locations should be filled up to form a full valid cubie permutation.
void setCombPerm(int comb, int perm, int cubies[], int len, int min_cubie, bool fill) {
  comb = dec_comb[comb];
  perm = dec_perm[perm];

  int cubie = 0;
  for (int i = 0; i < len; i++) {
    if (cubie == min_cubie)
      cubie += 4;
    if ((comb & (1 << i)) != 0) {
      cubies[i] = (perm & 0x3) + min_cubie;
      perm >>= 2;
    } else if (fill)
      cubies[i] = cubie++;
  }
}

int getTwist(const CubieCube &cube) {
  return getOri(cube.co, N_CORNERS, 3);
}

void setTwist(CubieCube &cube, int twist) {
  setOri(twist, cube.co, N_CORNERS, 3);
}

int getFlip(const CubieCube &cube) {
  return getOri(cube.eo, N_EDGES, 2);
}

void setFlip(CubieCube &cube, int flip) {
  setOri(flip, cube.eo, N_EDGES, 2);
}

int getSSlice(const CubieCube &cube) {
  Edges4 sslice;
  getCombPerm(sslice.comb, sslice.perm, cube.ep, N_EDGES, 0xf00, FR);
  // As SLICE must be 0 in phase 2, hence the combination coordinate must be inverted
  sslice.comb = (N_C12K4 - 1) - sslice.comb;
  return sslice.val();
}

void setSlice(CubieCube &cube, int slice) {
  setCombPerm((N_C12K4 - 1) - slice, 0, cube.ep, N_EDGES, FR, true);
}

int getUEdges(const CubieCube &cube) {
  Edges4 uedges;
  getCombPerm(uedges.comb, uedges.perm, cube.ep, N_EDGES, 0x00f, UR);
  return uedges.val();
}

int getDEdges(const CubieCube &cube) {
  Edges4 dedges;
  getCombPerm(dedges.comb, dedges.perm, cube.ep, N_EDGES, 0x0f0, DR);
  return dedges.val();
}

int getUDEdges2(const CubieCube &cube) {
  return N_PERM4 * getUEdges(cube) + getDEdges(cube) % N_PERM4;
}

void setUDEdges2(CubieCube &cube, int udedges) {
  Edges4 uedges;
  Edges4 dedges;
  splitUDEdges2(udedges, uedges, dedges);
  setCombPerm(uedges.comb, uedges.perm, cube.ep, N_EDGES, UR, false);
  // Note that `fill == true` would override UEDGES set in the previous line
  setCombPerm(dedges.comb, dedges.perm, cube.ep, N_EDGES, DR, false);
  // The result should always be a fully valid edge permutation, hence we need to also set the slice-edges
  for (int i = FR; i < N_EDGES; i++)
    cube.ep[i] = i;
}

int getCPerm(const CubieCube &cube) {
  CPerm cperm;
  getCombPerm(cperm.comb1, cperm.perm1, cube.cp, N_CORNERS, 0x0f, URF);
  getCombPerm(cperm.comb2, cperm.perm2, cube.cp, N_CORNERS, 0xf0, DFR);
  return cperm.val();
}

void setCPerm(CubieCube &cube, int cperm) {
  CPerm cperm1(cperm);
  setCombPerm(cperm1.comb1, cperm1.perm1, cube.cp, N_CORNERS, URF, false);
  setCombPerm(cperm1.comb2, cperm1.perm2, cube.cp, N_CORNERS, DFR, false);
}

int getFSlice(const CubieCube &cube) {
  return N_FLIP * (getSSlice(cube) / N_PERM4) + getFlip(cube);
}

void moveSSlice(const Edges4 &sslice, int move, Edges4 &sslice1) {
  // We need to undo the inversion of the combination part to use the normal move-tables
  int tmp = edges4_move[(N_C12K4 - 1) - sslice.comb][move];
  sslice1.comb = (N_C12K4 - 1) - (tmp >> 5);
  sslice1.perm = mul_perms[sslice.perm][tmp & 0x1f];
}

void moveEdges4(const Edges4 &edges4, int move, Edges4 &moved) {
  int tmp = edges4_move[edges4.comb][move];
  moved.comb = tmp >> 5;
  moved.perm = mul_perms[edges4.perm][tmp & 0x1f];
}

void moveCPerm(const CPerm &cperm, int move, CPerm &cperm1) {
  int tmp = cperm4_move[cperm.comb1][move];
  cperm1.comb1 = tmp >> 5;
  cperm1.perm1 = mul_perms[cperm.perm1][tmp & 0x1f];
  tmp = cperm4_move[cperm.comb2][move];
  cperm1.comb2 = tmp >> 5;
  cperm1.perm2 = mul_perms[cperm.perm2][tmp & 0x1f];
}

int mergeUDEdges2(const Edges4 &uedges, const Edges4 &dedges) {
  return N_PERM4 * uedges.val() + dedges.perm;
}

void splitUDEdges2(int udedges, Edges4 &uedges, Edges4 &dedges) {
  uedges.set(udedges / N_PERM4);
  dedges.comb = enc_comb[0xff & ~dec_comb[uedges.comb]];
  dedges.perm = udedges % N_PERM4;
}

void initMoveCoord(
  uint16_t (**move_coord)[N_MOVES],
  int n_coords,
  int (*getCoord)(const CubieCube &),
  void (*setCoord)(CubieCube &, int),
  void (*mul)(const CubieCube &, const CubieCube &, CubieCube &)
) {
  auto move_coord1 = new uint16_t[n_coords][N_MOVES];

  CubieCube cube1 = kSolvedCube; // otherwise `mul()`s can easily crash
  CubieCube cube2;
  for (int c = 0; c < n_coords; c++) {
    setCoord(cube1, c);
    for (int m = 0; m < N_MOVES; m++) {
      mul(cube1, move_cubes[m], cube2);
      move_coord1[c][m] = getCoord(cube2);
    }
  }

  *move_coord = move_coord1;
}

// Initializes all tables used for performing coordinate moves
void initCoordTables() {
  initMoveCoord(&move_twist, N_TWIST, getTwist, setTwist, mulCorners);
  initMoveCoord(&move_flip, N_FLIP, getFlip, setFlip, mulEdges);

  CubieCube cube1 = kSolvedCube;
  CubieCube cube2;

  for (int comb = 0; comb < N_C12K4; comb++) {
    setCombPerm(comb, 0, cube1.ep, N_EDGES, UR, true);
    for (int m = 0; m < N_MOVES; m++) {
      mulEdges(cube1, move_cubes[m], cube2);
      int comb1;
      int perm1;
      getCombPerm(comb1, perm1, cube2.ep, N_EDGES, 0x00f, UR);
      edges4_move[comb][m] = (comb1 << 5) | perm1;
    }
  }
  for (int comb = 0; comb < N_C8K4; comb++) {
    setCombPerm(comb, 0, cube1.cp, N_CORNERS, URF, true);
    for (int m = 0; m < N_MOVES; m++) {
      mulCorners(cube1, move_cubes[m], cube2);
      int comb1;
      int perm1;
      getCombPerm(comb1, perm1, cube2.cp, N_CORNERS, 0x0f, URF);
      cperm4_move[comb][m] = (comb1 << 5) | perm1;
    }
  }
}
