/**
 * Cubie level:
 * A cube is represented by the permutation and orientation of its 12 edge cubies (physical cube pieces) as well as
 * the permutation and orientation of its 8 corner cubies. This representation is primarily utilized to compute all
 * kinds of lookup-tables.
 */

#ifndef CUBIE_H_
#define CUBIE_H_

#include <iostream>
#include <string>

#define N_CORNERS 8 // number of corner cubies
#define N_EDGES 12 // number of edge cubies

/* Corner cubie order */
#define URF 0
#define UFL 1
#define ULB 2
#define UBR 3
#define DFR 4
#define DLF 5
#define DBL 6
#define DRB 7

/* Edge cubie order */
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

/* Map corner/edge cubie IDs to their respective cubie names */
const std::string kCornerNames[] = {
  "URF", "UFL", "ULB", "UBR", "DFR", "DLF", "DBL", "DRB"
};
const std::string kEdgeNames[] = {
  "UR", "UF", "UL", "UB", "DR", "DF", "DL", "DB", "FR", "FL", "BL", "BR"
};

// Main data-structure; we simply use int as we never need to store more than a few CubieCubes
typedef struct {
  int cp[N_CORNERS]; // corner permutation
  int ep[N_EDGES]; // edge permutation
  int co[N_CORNERS]; // corner orientation (values of 0 - 2; >= 3 indicates mirrored state)
  int eo[N_EDGES]; // edge orientation (values of 0 - 1)
} CubieCube;

// The CubieCube representation of a solved cube
const CubieCube kSolvedCube = {
  {URF, UFL, ULB, UBR, DFR, DLF, DBL, DRB},
  {UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR},
  {}, {}
};

/* Main cube operations; some are performed millions of times, hence result has to be passed explicitly */

// Multiplies only the edges of two cubes
void mulCorners(const CubieCube &cube1, const CubieCube &cube2, CubieCube &res);
// Multiplies only the corners of two cubes
void mulEdges(const CubieCube &cube1, const CubieCube &cube2, CubieCube &res);
// Multiplies two cubes
void mul(const CubieCube &cube1, const CubieCube &cube2, CubieCube &res);
// Inverts a cube
void inv(const CubieCube &cube, CubieCube &res);

// Validates a cubCubieCube
int check(const CubieCube &cube);
// Generates a uniformly random cube
void shuffle(CubieCube &cube);

/* Define a few convenient standard operators */
bool operator==(const CubieCube &cube1, const CubieCube &cube2);
bool operator!=(const CubieCube &cube1, const CubieCube &cube2);
std::ostream& operator<<(std::ostream &os, const CubieCube &cube);

#endif
