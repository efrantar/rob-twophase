#include "sym.h"

#include <iostream>

#define EMPTY 0xffff

CubieCube sym_cubes[N_SYMS];
Sym inv_sym[N_SYMS];
int conj_move[N_MOVES][N_SYMS];

Coord (*conj_twist)[N_SYMS_DH4];
Coord (*conj_udedges)[N_SYMS_DH4];

SymCoord *flipslice_sym;
Sym *flipslice_sym_sym;
LargeCoord *flipslice_raw;
SymSet *flipslice_symset;

SymCoord *corners_sym;
Sym *corners_sym_sym;
Coord *corners_raw;
SymSet *corners_symset;

void initSyms() {
  CubieCube cube;
  CubieCube tmp;

  for (Sym i = 0; i < N_SYMS; i++) {
    copy(cube, sym_cubes[i]);

    mulCubes(cube, kLR2Cube, tmp);
    copy(tmp, cube);

    if (i % 2 == 1) {
      mulCubes(cube, kU4Cube, tmp);
      copy(tmp, cube);
    }
    if (i % 8 == 7) {
      mulCubes(cube, kF2Cube, tmp);
      copy(tmp, cube);
    }
    if (i % 16 == 15) {
      mulCubes(cube, kURF3Cube, tmp);
      copy(tmp, cube);
    }
  }

  for (Sym i = 0; i < N_SYMS; i++) {
    for (Sym j = 0; j < N_SYMS; j++) {
      mulCubes(sym_cubes[i], sym_cubes[j], cube);
      if (cube.cp[URF] == URF && cube.cp[UFL] == UFL && cube.cp[ULB] == ULB) {
        inv_sym[i] = j;
        break;
      }
    }
  }

  for (int m = 0; m < N_MOVES; m++) {
    for (Sym s = 0; s < N_SYMS; s++) {
      mulCubes(sym_cubes[s], move_cubes[m], tmp);
      mulCubes(tmp, sym_cubes[inv_sym[s]], cube);
      for (int conj = 0; conj < N_MOVES; conj++) {
        if (equal(cube, move_cubes[conj])) {
          conj_move[m][s] = conj;
          break;
        }
      }
    }
  }
}

void initConjCoord(
  Coord (**conj_coord)[N_SYMS_DH4],
  int n_coords,
  Coord (*getCoord)(CubieCube &),
  void (*setCoord)(CubieCube &, Coord),
  void (*mul)(const CubieCube &, const CubieCube &, CubieCube &)
) {
  Coord (*conj_coord1)[N_SYMS_DH4] = new Coord[n_coords][N_SYMS_DH4];

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;

  for (Coord c = 0; c < n_coords; c++) {
    setCoord(cube1, c);
    conj_coord1[c][0] = c;
    for (Sym s = 1; s < N_SYMS_DH4; s++) { 
      mul(sym_cubes[s], cube1, tmp);
      mul(tmp, sym_cubes[inv_sym[s]], cube2);
      conj_coord1[c][s] = getCoord(cube2);
    }
  }

  *conj_coord = conj_coord1;
}

void initConjTwist() {
  initConjCoord(&conj_twist, N_TWIST_COORDS, getTwist, setTwist, mulCorners);
}

void initConjUDEdges() {
  initConjCoord(&conj_udedges, N_UDEDGES_COORDS_P2, getUDEdges, setUDEdges, mulEdges);
}

void initFlipSliceSyms() {
  flipslice_sym = new SymCoord[N_FLIPSLICE_COORDS];
  flipslice_sym_sym = new Sym[N_FLIPSLICE_COORDS];
  flipslice_raw = new LargeCoord[N_FLIPSLICE_SYM_COORDS];
  flipslice_symset = new SymSet[N_FLIPSLICE_SYM_COORDS];  
  std::fill(flipslice_sym, flipslice_sym + N_FLIPSLICE_COORDS, EMPTY);

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;
  SymCoord cls = 0;

  for (Coord slice = 0; slice < N_SLICE_COORDS; slice++) {
    setSlice(cube1, slice);
    for (Coord flip = 0; flip < N_FLIP_COORDS; flip++) {
      setFlip(cube1, flip);
      LargeCoord flipslice1 = FLIPSLICE(flip, slice);

      if (flipslice_sym[flipslice1] != EMPTY)
        continue;

      flipslice_sym[flipslice1] = cls;
      flipslice_sym_sym[flipslice1] = 0;
      flipslice_raw[cls] = flipslice1;
      flipslice_symset[cls] = 1;

      for (Sym s = 1; s < N_SYMS_DH4; s++) {
        mulEdges(sym_cubes[inv_sym[s]], cube1, tmp);
        mulEdges(tmp, sym_cubes[s], cube2);
        LargeCoord flipslice2 = FLIPSLICE(getFlip(cube2), getSlice(cube2));
        if (flipslice_sym[flipslice2] == EMPTY) {
          flipslice_sym[flipslice2] = cls;
          flipslice_sym_sym[flipslice2] = s;
        } else if (flipslice2 == flipslice1)
          flipslice_symset[cls] |= 1 << s;
      }
      cls++;
    }
  }
}

void initCornersSyms() {
  corners_sym = new SymCoord[N_CORNERS_COORDS];
  corners_sym_sym = new Sym[N_CORNERS_COORDS];
  corners_raw = new Coord[N_CORNERS_SYM_COORDS];
  corners_symset = new SymSet[N_CORNERS_SYM_COORDS];
  std::fill(corners_sym, corners_sym + N_CORNERS_COORDS, EMPTY);

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;
  SymCoord cls = 0;

  for (Coord corners1 = 0; corners1 < N_CORNERS_COORDS; corners1++) {
    setCorners(cube1, corners1);
    if (corners_sym[corners1] != EMPTY)
      continue;

    corners_sym[corners1] = cls;
    corners_sym_sym[corners1] = 0;
    corners_raw[cls] = corners1;
    corners_symset[cls] = 1;

    for (Sym s = 1; s < N_SYMS_DH4; s++) {
      mulCorners(sym_cubes[inv_sym[s]], cube1, tmp);
      mulCorners(tmp, sym_cubes[s], cube2);
      Coord corners2 = getCorners(cube2);
      if (corners_sym[corners2] == EMPTY) {
        corners_sym[corners2] = cls;
        corners_sym_sym[corners2] = s;
      } else if (corners2 == corners1)
        corners_symset[cls] |= 1 << s;
    }
    cls++;
  }
}

