#include "sym.h"

#include <iostream>

#define EMPTY 0xffff

CubieCube sym_cubes[N_SYMS];
Sym inv_sym[N_SYMS];
int conj_move[N_MOVES][N_SYMS];

Coord (*conj_twist)[N_SYMS_DH4];
Coord (*conj_udedges)[N_SYMS_DH4];

Class *flipslice_cls;
Sym *flipslice_sym;
LargeCoord *flipslice_rep;

Class *corners_cls;
Sym *corners_sym;
Coord *corners_rep;

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
    for (Sym s = 0; s < N_SYMS_DH4; s++) { 
      mul(cube1, sym_cubes[s], tmp);
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
  flipslice_cls = new Class[N_FLIPSLICE_COORDS];
  flipslice_sym = new Sym[N_FLIPSLICE_COORDS];
  flipslice_rep = new LargeCoord[N_FLIPSLICE_CLASSES];
  std::fill(flipslice_cls, flipslice_cls + N_FLIPSLICE_COORDS, EMPTY);

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;
  Class cls = 0;

  for (Coord slice = 0; slice < N_SLICE_COORDS; slice++) {
    setSlice(cube1, slice);
    for (Coord flip = 0; flip < N_FLIP_COORDS; flip++) {
      setFlip(cube1, flip);
      LargeCoord flipslice1 = FLIPSLICE(flip, slice);

      if (flipslice_cls[flipslice1] != EMPTY)
        continue;

      flipslice_cls[flipslice1] = cls;
      flipslice_sym[flipslice1] = 0;
      flipslice_rep[cls] = flipslice1;
     
      for (Sym s = 0; s < N_SYMS_DH4; s++) {
        mulEdges(sym_cubes[inv_sym[s]], cube1, tmp);
        mulEdges(tmp, sym_cubes[s], cube2);
        LargeCoord flipslice2 = FLIPSLICE(getFlip(cube2), getSlice(cube2));
        if (flipslice_cls[flipslice2] == EMPTY) {
          flipslice_cls[flipslice2] = cls;
          flipslice_sym[flipslice2] = s;
        }
      }
      cls++;
    }
  }
}

void initCornersSyms() {
  corners_cls = new Class[N_CORNERS_COORDS];
  corners_sym = new Sym[N_CORNERS_COORDS];
  corners_rep = new Coord[N_CORNERS_CLASSES];
  std::fill(corners_cls, corners_cls + N_CORNERS_COORDS, EMPTY);

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;
  Class cls = 0;

  for (Coord corners1 = 0; corners1 < N_CORNERS_COORDS; corners1++) {
    setCorners(cube1, corners1);
    if (corners_cls[corners1] != EMPTY)
      continue;

    corners_cls[corners1] = cls;
    corners_sym[corners1] = 0;
    corners_rep[cls] = corners1;

    for (Sym s = 0; s < N_SYMS_DH4; s++) {
      mulCorners(sym_cubes[inv_sym[s]], cube1, tmp);
      mulCorners(tmp, sym_cubes[s], cube2);
      Coord corners2 = getCorners(cube2);
      if (corners_cls[corners2] == EMPTY) {
        corners_cls[corners2] = cls;
        corners_sym[corners2] = s;
      }
    }
    cls++;
  }
}

