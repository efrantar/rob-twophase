/*
 * Cubie level; performing cube moves + some utility methods
 */

#ifndef CUBIE_H_
#define CUBIE_H_

#include <iostream>
#include <string>

#define N_CORNERS 8
#define N_EDGES 12

/* Cubie definitions; note that any changes here might require modifications all throughout the solver */

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

// We never store more than a few `CubieCube`s -> size does not matter -> just use fastest machine type -> int
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

// Some of the following operations are performed millions of times -> explicitly passing result-object most efficient
void mulCorners(const CubieCube &cube1, const CubieCube &cube2, CubieCube &res);
void mulEdges(const CubieCube &cube1, const CubieCube &cube2, CubieCube &res);
void mul(const CubieCube &cube1, const CubieCube &cube2, CubieCube &res);
void inv(const CubieCube &cube, CubieCube &res);

int check(const CubieCube &cube);
void randomize(CubieCube &cube);

bool operator==(const CubieCube &cube1, const CubieCube &cube2);
bool operator!=(const CubieCube &cube1, const CubieCube &cube2);
std::ostream& operator<<(std::ostream &os, const CubieCube &cube);

#endif
