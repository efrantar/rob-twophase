#include "cubie.h"

#include <algorithm>
#include <random>
#include "coord.h"

/*
 * Due to mirrored corner orientations, properly carrying out corner multiplication (and inversion) involves several
 * different cases. We avoid a sequence of if-statements by considerably faster (and also simpler) table-lookups.
 * Edge orientation is easy enough to handle directly.
 */

// Multiplies two corner orientations
int mul_coris[][6] = {
  {0, 1, 2, 3, 4, 5},
  {1, 2, 0, 4, 5, 3},
  {2, 0, 1, 5, 3, 4},
  {3, 5, 4, 0, 2, 1},
  {4, 3, 5, 1, 0, 2},
  {5, 4, 3, 2, 1, 0}
};

// Inverts a corner orientation
int inv_cori[] = {
  0, 2, 1, 3, 4, 5
};

/* Random generator setup */
std::random_device device;
std::mt19937 gen(device());

void mulCorners(const CubieCube &cube1, const CubieCube &cube2, CubieCube &res) {
  for (int i = 0; i < N_CORNERS; i++) {
    res.cp[i] = cube1.cp[cube2.cp[i]];
    res.co[i] = mul_coris[cube1.co[cube2.cp[i]]][cube2.co[i]];
  }
}

void mulEdges(const CubieCube &cube1, const CubieCube &cube2, CubieCube &res) {
  for (int i = 0; i < N_EDGES; i++) {
    res.ep[i] = cube1.ep[cube2.ep[i]];
    res.eo[i] = (cube1.eo[cube2.ep[i]] + cube2.eo[i]) & 1;
  }
}

void mul(const CubieCube &cube1, const CubieCube &cube2, CubieCube &res) {
  mulEdges(cube1, cube2, res);
  mulCorners(cube1, cube2, res);
}

void inv(const CubieCube &cube, CubieCube &res) {
  for (int corner = 0; corner < N_CORNERS; corner++)
    res.cp[cube.cp[corner]] = corner; // inv[a[i]] = i
  for (int edge = 0; edge < N_EDGES; edge++)
    res.ep[cube.ep[edge]] = edge;
  for (int i = 0; i < N_CORNERS; i++)
    res.co[i] = inv_cori[cube.co[res.cp[i]]];
  for (int i = 0; i < N_EDGES; i++)
    res.eo[i] = cube.eo[res.ep[i]];
}

// Computes the parity of a permutation, i.e. #inversion % 2
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

// A mirrored cube is not considered valid; it should only exist during intermediate results of a symmetry transform
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
    return 9; // corner and edge permutation parity mismatch
  return 0;
}

void shuffle(CubieCube &cube) {
  for (int i = 0; i < N_CORNERS; i++)
    cube.cp[i] = i;
  for (int i = 0; i < N_EDGES; i++)
    cube.ep[i] = i;

  // We shuffle explicitly as we do not have a coordinate that encodes all edges
  std::shuffle(cube.cp, cube.cp + N_CORNERS, gen);
  std::shuffle(cube.ep, cube.ep + N_EDGES, gen);
  if (parity(cube.cp, N_CORNERS) != parity(cube.ep, N_EDGES))
    // Any single swap of two elements flips the permutation parity
    std::swap(cube.cp[N_CORNERS - 2], cube.cp[N_CORNERS - 1]);

  setTwist(cube, std::uniform_int_distribution<Coord>(0, N_TWIST)(gen));
  setFlip(cube, std::uniform_int_distribution<Coord>(0, N_FLIP)(gen));
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
