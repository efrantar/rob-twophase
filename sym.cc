#include "sym.h"

#include <iostream>

CubieCube sym_cubes[N_SYMS];
Sym inv_sym[N_SYMS];
int conj_move[N_MOVES][N_SYMS];

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

