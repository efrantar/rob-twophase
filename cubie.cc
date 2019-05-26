#include "cubie.h"

#include <iostream>
#include "coord.h"
#include "misc.h"

void mulCorners(const CubieCube &cube1, const CubieCube &cube2, CubieCube &cube3) {
  for (int i = 0; i < N_CORNERS; i++) {
    cube3.cp[i] = cube1.cp[cube2.cp[i]];
 
    int ori1 = cube1.co[cube2.cp[i]];
    int ori2 = cube2.co[i];
    int ori3;

    if (ori1 < 3 && ori2 < 3) // handle this case first as move-tables do not use symmetries
      ori3 = (ori1 + ori2) % 3;
    else { // handle mirroring symmetry
      if (ori1 >= 3) {
        ori3 = ori1 - ori2;
        if (ori2 >= 3) { // mirr + mirr = not
          if (ori3 < 0)
            ori3 += 3;
        } else
          ori3 = ori3 % 3 + 3; // mir + not = mirr
      } else
        ori3 = (ori1 + ori2 - 3) % 3 + 3; // not + mir = mirr
    }

    cube3.co[i] = ori3;
  }
}

void mulEdges(const CubieCube &cube1, const CubieCube &cube2, CubieCube &cube3) {
  for (int i = 0; i < N_EDGES; i++) {
    cube3.ep[i] = cube1.ep[cube2.ep[i]];
    cube3.eo[i] = (cube1.eo[cube2.ep[i]] + cube2.eo[i]) & 1;
  }
}

void mul(const CubieCube &cube1, const CubieCube &cube2, CubieCube &cube3) {
  mulEdges(cube1, cube2, cube3);
  mulCorners(cube1, cube2, cube3);
}

// #inversions % 2
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

int check(const CubieCube &cube) {
  bool corners[N_CORNERS] = {};
  int co_sum = 0;
  
  for (int i = 0; i < N_CORNERS; i++) {
    if (cube.cp[i] < 0 || cube.cp[i] >= N_CORNERS)
      return 1; // invalid corner cubie
    corners[cube.cp[i]] = true;
    if (cube.co[i] < 0 || cube.co[i] >= 3)
      return 2; // invalid corner orientation
    co_sum += cube.co[i];
  }
  if (co_sum % 3 != 0)
    return 3; // invalid twist parity
  for (bool corner : corners) {
    if (!corner)
      return 4; // missing corner
  }

  bool edges[N_EDGES] = {};
  int eo_sum = 0;

  for (int i = 0; i < N_EDGES; i++) {
    if (cube.ep[i] < 0 || cube.ep[i] >= N_EDGES)
      return 5; // invalid edge cubie
    edges[cube.ep[i]] = true;
    if (cube.eo[i] < 0 || cube.eo[i] >= 2)
      return 6; // invalid edge orientation
    eo_sum += cube.eo[i];
  }
  if (eo_sum & 1 != 0)
    return 7; // invalid flip parity
  for (bool edge : edges) {
    if (!edge)
      return 8; // missing edge
  }
  
  if (parity(cube.cp, N_CORNERS) != parity(cube.ep, N_EDGES))
    return 9; // corner and edge parity mismatch
  return 0;
}

CubieCube invCube(const CubieCube &cube) {
  CubieCube inv;

  for (int corner = 0; corner < N_CORNERS; corner++)
    inv.cp[cube.cp[corner]] = corner; // inv[a[i]] = i
  for (int edge = 0; edge < N_EDGES; edge++)
    inv.ep[cube.ep[edge]] = edge;

  for (int i = 0; i < N_CORNERS; i++) {
    int ori = cube.co[inv.cp[i]];
    if (ori >= 3)
      inv.co[i] = ori; // mirr * mirr calculated as ori1 - ori2 = 0
    else
      inv.co[i] = mod(-ori, 3); // (ori + -ori) % 3 = 0
  }
  for (int i = 0; i < N_EDGES; i++)
    inv.eo[i] = cube.eo[inv.ep[i]];

  return inv;
}

CubieCube randomCube() {
  CubieCube cube;

  setTwist(cube, rand(N_TWIST));
  setFlip(cube, rand(N_FLIP));
  do {
    setCorners(cube, rand(N_CORNERS_C));
    setEdges(cube, rand64(N_EDGES_C)); // EDGES_C is a CoordLL
  } while (parity(cube.cp, N_CORNERS) != parity(cube.ep, N_EDGES));

  return cube;
}

void copy(const CubieCube &from, CubieCube &to) {
  std::copy(from.cp, from.cp + N_CORNERS, to.cp);
  std::copy(from.ep, from.ep + N_EDGES, to.ep);
  std::copy(from.co, from.co + N_CORNERS, to.co);
  std::copy(from.eo, from.eo + N_EDGES, to.eo);
}

bool operator==(const CubieCube &cube1, const CubieCube &cube2) {
  return
    std::equal(cube1.cp, cube1.cp + N_CORNERS, cube2.cp) &&
    std::equal(cube1.ep, cube1.ep + N_EDGES, cube2.ep) &&
    std::equal(cube1.co, cube1.co + N_CORNERS, cube2.co) &&
    std::equal(cube1.eo, cube1.eo + N_EDGES, cube2.eo)
  ;
}

bool operator!=(const CubieCube &cube1, const CubieCube &cube2) {
  return !(cube1 == cube2);
}

std::ostream& operator<<(std::ostream &os, const CubieCube &cube) {
  for (int i = 0; i < N_CORNERS; i++)
    os << kCornerNames[cube.cp[i]] << "(" << cube.co[i] << ") ";
  os << std::endl;
  for (int i = 0; i < N_EDGES; i++)
    os << kEdgeNames[cube.ep[i]] << "(" << cube.eo[i] << ") ";
  os << std::endl;

  return os;
}
