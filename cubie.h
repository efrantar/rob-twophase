/*
 * Cubie level; performing cube moves and some utility methods
 */

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

// Coordinate definitions strongly depend on this ordering
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

// CubieCubes are never stored in mass -> size does not matter -> just use fastest machine type int
typedef struct {
  int cp[N_CORNERS];
  int ep[N_EDGES];
  int co[N_CORNERS];
  int eo[N_EDGES];
} CubieCube;

const CubieCube kSolvedCube = {
  {URF, UFL, ULB, UBR, DFR, DLF, DBL, DRB},
  {UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR},
  {}, {}
};

// Table generation performs millions of muls -> this seemingly inconvenient interface is the most efficient
void mulEdges(const CubieCube &cube1, const CubieCube &cube2, CubieCube &cube3);
void mulCorners(const CubieCube &cube1, const CubieCube &cube2, CubieCube &cube3);
void mul(const CubieCube &cube1, const CubieCube &cube2, CubieCube &cube3);
CubieCube invCube(const CubieCube &cube);

int check(const CubieCube &cube);
CubieCube randomCube();

void copy(const CubieCube &from, CubieCube &to); // mostly used to initialize a new CubieCube as solved
bool operator==(const CubieCube &cube1, const CubieCube &cube2);
bool operator!=(const CubieCube &cube1, const CubieCube &cube2);
std::ostream& operator<<(std::ostream &os, const CubieCube &cube);

#endif
