#ifndef CUBIE_H_
#define CUBIE_H_

#include <iostream>
#include <stdint.h>
#include <string>

#define N_CORNERS 8
#define N_EDGES 12

#define URF 0
#define UFL 1
#define ULB 2
#define UBR 3
#define DFR 4
#define DLF 5
#define DBL 6
#define DRB 7

#define UR 0
#define UF 1
#define UL 2
#define UB 3
#define DR 4
#define DF 5
#define DL 6
#define DB 7
#define FR 8
#define FL 9
#define BL 10
#define BR 11

const std::string kCornerNames[] = {
  "URF", "UFL", "ULB", "UBR", "DFR", "DLF", "DBL", "DRB"
};
const std::string kEdgeNames[] = {
  "UR", "UF", "UL", "UB", "DR", "DF", "DL", "DB", "FR", "FL", "BL", "BR"
};

typedef struct {
  int cp[N_CORNERS] = {URF, UFL, ULB, UBR, DFR, DLF, DBL, DRB};
  int ep[N_EDGES] = {UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR};
  int co[N_CORNERS] = {};
  int eo[N_EDGES] = {};
} CubieCube;

void mulEdges(const CubieCube &cube_a, const CubieCube &cube_b, CubieCube &cube_c);
void mulCorners(const CubieCube &cube_a, const CubieCube &cube_b, CubieCube &cube_c);
void mulCubes(const CubieCube &cube_a, const CubieCube &cube_b, CubieCube &cube_c);
CubieCube mulCubes(const CubieCube &cube_a, const CubieCube &cube_b);

bool isSolvable(const CubieCube &cube);

void copy(const CubieCube &cube_from, CubieCube &cube_to);
bool equal(const CubieCube &cube1, const CubieCube &cube2);
void print(const CubieCube &cube);

#endif

