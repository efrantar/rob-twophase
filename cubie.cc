#include "cubie.h"

void mulCorners(const CubieCube &cube_a, const CubieCube &cube_b, CubieCube &cube_c) {
  for (int i = 0; i < N_CORNERS; i++) {
    cube_c.cp[i] = cube_a.cp[cube_b.cp[i]];
 
    int ori_a = cube_a.co[cube_b.cp[i]];
    int ori_b = cube_b.co[i];
    int ori_c;

    if (ori_a < 3 && ori_b < 3)
      ori_c = (ori_a + ori_b) % 3;
    else {
      if (ori_a >= 3) {
        ori_c = ori_a - ori_b;
        if (ori_b >= 3) {
          if (ori_c < 0)
            ori_c += 3;
        } else
          ori_c = ori_c % 3 + 3;
      } else
        ori_c = (ori_a + ori_b - 3) % 3 + 3;
    }

    cube_c.co[i] = ori_c;
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

void copy(const CubieCube &cube_from, CubieCube &cube_to) {
  std::copy(cube_from.cp, cube_from.cp + N_CORNERS, cube_to.cp);
  std::copy(cube_from.ep, cube_from.ep + N_EDGES, cube_to.ep);
  std::copy(cube_from.co, cube_from.co + N_CORNERS, cube_to.co);
  std::copy(cube_from.eo, cube_from.eo + N_EDGES, cube_to.eo);
}

bool equal(const CubieCube &cube1, const CubieCube &cube2) {
  return
    std::equal(cube1.cp, cube1.cp + N_CORNERS, cube2.cp) &&
    std::equal(cube1.ep, cube1.ep + N_EDGES, cube2.ep) &&
    std::equal(cube1.co, cube1.co + N_CORNERS, cube2.co) &&
    std::equal(cube1.eo, cube1.eo + N_EDGES, cube2.eo)
  ;
}

void print(const CubieCube &cube) {
  for (int i = 0; i < N_CORNERS; i++)
    std::cout << kCornerNames[cube.cp[i]] << "(" << cube.co[i] << ") ";
  std::cout << "\n";
  for (int i = 0; i < N_EDGES; i++)
    std::cout << kEdgeNames[cube.ep[i]] << "(" << cube.eo[i] << ") ";
  std::cout << "\n";
}

