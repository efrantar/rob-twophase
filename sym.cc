#include "sym.h"

CubieCube sym_cubes[N_SYMS];
Sym inv_sym[N_SYMS];
int conj_move[N_MOVES][N_SYMS];

void initSyms() {
  CubieCube cube;
  int i = 0;

  for (int urf3 = 0; urf3 < 3; urf3++) {
    for (int f2 = 0; f2 < 2; f2++) {
      for (int u4 = 0; u4 < 4; u4++) {
        for (int lr2 = 0; lr2 < 2; lr2++) {
          sym_cubes[i] = copy(cube);
          i++;
          mulCubes(cube, kLR2Cube, cube);
        }
        mulCubes(cube, kU4Cube, cube);
      }
      mulCubes(cube, kF2Cube, cube);
    }
    mulCubes(cube, kURF3Cube, cube);
  }


}

