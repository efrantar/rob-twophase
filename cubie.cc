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

CubieCube copy(const CubieCube &cube) {
  CubieCube cube1;
  std::copy(cube.cp, cube.cp + N_CORNERS, cube1.cp);
  std::copy(cube.ep, cube.ep + N_EDGES, cube1.ep);
  std::copy(cube.co, cube.co + N_CORNERS, cube1.co);
  std::copy(cube.eo, cube.eo + N_EDGES, cube1.eo);
  return cube1;
}

