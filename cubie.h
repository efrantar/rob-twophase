/**
 * The main representation underlying the theory of this solver is the so called `CubieCube`. There are Rubik's Cube
 * is represented by the permutation of its 12 edges cubies (physical cubie pieces) and its 8 corner cubies as well
 * as the corresponding orientations. Note however that the solver does not directly operate on `CubieCube`s but
 * rather only on much faster lookup-tables that were generated based on this representation.
 *
 * Permutations are handled in a way that every cubie position on the solved cube is assigned a number, then it is
 * stored in which position every cubie resides, i.e. cubie `x` is in position `Y` and so on. Since a cubie in some
 * slot (position) is not always oriented in the same way, we also need to track its orientation. An edge can be
 * "flipped" twice and a corner can be "twisted" 3 times, hence there are a total of 2 edge- and 3 corner-orientations.
 * These are carefully defined such that they preserve very particular properties (most importantly that all of them
 * are 0 in a phase 1 solution, see als "solve.h"). Please refer to Kociemba's description for a precise definition.
 *
 * For this representation to be useful, we need to be able to (efficiently) manipulate them. Therefore this file
 * defines several functions for carrying out cube-moves. Notice how we call those `mulX()` since their internal
 * mechanics follow the mathematical "multiplication" of permutations. Furthermore, we define those functions of the
 * form `f(a, b, c)` resulting in `c = a * b`, since those are called many many times during table generation and we
 * explicitly want to manage all resources involved including where the result is to be stored. We provide separate
 * functions for manipulating corners and edges since some coordinates (see coord.cc) only depend on either cubie type
 * and considering the other would just be a waste.
 *
 * Finally, this file also contains some more convenience functions like validating, randomizing or printing a
 * `CubieCube` object.
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
