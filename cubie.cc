#include "cubie.h"

// TODO: handle reflection
void mulCorners(const CubieCube &cube_a, const CubieCube &cube_b, CubieCube &cube_c) {
  for (corner c = 0; c < N_CORNERS; c++) {
    cube_c.cp[c] = cube_a.cp[cube_b.cp[c]];
    cube_c.co[c] = (cube_a.co[cube_b.ep[c]] + cube_b.ep[c]) % 3;
  }
}

void mulEdges(const CubieCube &cube_a, const CubieCube &cube_b, CubieCube &cube_c) {
  for (edge e = 0; e < N_EDGES; e++) {
    cube_c.ep[e] = cube_a.ep[cube_b.ep[e]];
    cube_c.eo[e] = (cube_a.eo[cube_b.ep[e]] + cube_b.eo[e]) & 1;
  }
}

bool isSolvable(const CubieCube &cube) {
  return false; // TODO: implement
}

