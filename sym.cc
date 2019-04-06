#include "sym.h"

#include <iostream>

CubieCube sym_cubes[N_SYMS];
Sym inv_sym[N_SYMS];
int conj_move[N_MOVES][N_SYMS];

Coord (*conj_twist)[N_SYMS_DH4];
Coord (*conj_udedges)[N_SYMS_DH4];

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

