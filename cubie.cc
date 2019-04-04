#include "cubie.h"

// TODO: handle reflection
void mulCorners(const CubieCube &cube_a, const CubieCube &cube_b, CubieCube &cube_c) {
  for (int i = 0; i < N_CORNERS; i++) {
    cube_c.cp[i] = cube_a.cp[cube_b.cp[i]];
    cube_c.co[i] = (cube_a.co[cube_b.cp[i]] + cube_b.co[i]) % 3;
  }
}

void mulEdges(const CubieCube &cube_a, const CubieCube &cube_b, CubieCube &cube_c) {
  for (int i = 0; i < N_EDGES; i++) {
    cube_c.ep[i] = cube_a.ep[cube_b.ep[i]];
    cube_c.eo[i] = (cube_a.eo[cube_b.ep[i]] + cube_b.eo[i]) & 1;
  }
}

void mulCubes(const CubieCube &cube_a, const CubieCube &cube_b, CubieCube &cube_c) {
  mulEdges(cube_a, cube_b, cube_c);
  mulCorners(cube_a, cube_b, cube_c);
}

bool isSolvable(const CubieCube &cube) {
  return false; // TODO: implement
}

