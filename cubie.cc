#include "cubie.h"

#include "coord.h"
#include "misc.h"

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

bool parity(const int perm[], int len) {
  int par = 0;
  for (int i = 0; i < len; i++) {
    for (int j = 0; j < i; j++) {
      if (perm[j] > perm[i])
        par++;
    }
  }
  return par & 1;
}

int checkCube(const CubieCube &cube) {
  bool corners[N_CORNERS] = {};
  int co_sum = 0;
  
  for (int i = 0; i < N_CORNERS; i++) {
    if (cube.cp[i] < 0 || cube.cp[i] >= N_CORNERS)
      return 1;
    corners[cube.cp[i]] = true;
    if (cube.co[i] < 0 || cube.co[i] >= 3)
      return 2;
    co_sum += cube.co[i];
  }
  if (co_sum % 3 != 0)
    return 3;
  for (int i = 0; i < N_CORNERS; i++) {
    if (!corners[i])
      return 4;
  }

  bool edges[N_EDGES] = {};
  int eo_sum = 0;

  for (int i = 0; i < N_EDGES; i++) {
    if (cube.ep[i] < 0 || cube.ep[i] >= N_EDGES)
      return 5;
    edges[cube.ep[i]] = true;
    if (cube.eo[i] < 0 || cube.eo[i] >= 2)
      return 6;
    eo_sum += cube.eo[i];
  }
  if (eo_sum & 1 != 0)
    return 7;
  for (int i = 0; i < N_EDGES; i++) {
    if (!edges[i])
      return 8;
  }
  
  if (parity(cube.cp, N_CORNERS) != parity(cube.ep, N_EDGES))
    return 9;
  return 0;
}

CubieCube randomCube() {
  CubieCube cube;

  setTwist(cube, rand(N_TWIST_COORDS));
  setFlip(cube, rand(N_FLIP_COORDS));
  do {
    setCorners(cube, rand(N_CORNERS_COORDS));
    setEdges(cube, randLong(N_EDGES_COORDS));
  } while (parity(cube.cp, N_CORNERS) != parity(cube.ep, N_EDGES));

  return cube;
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

