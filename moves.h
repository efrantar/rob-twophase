/*
 * Move definitions
 */

#ifndef MOVES_H_
#define MOVES_H_

#include <string>
#include "cubie.h"

#ifdef FACES5
  #ifdef AXIAL
    #define N_MOVES 33
    #define N_MOVES2 18
  #else
    #define N_MOVES 15
    #define N_MOVES2 9
  #endif
#else
  #ifdef AXIAL
    #define N_MOVES 45
    #define N_MOVES2 29
  #else
    #define N_MOVES 18
    #define N_MOVES2 10
  #endif
#endif

#define MAX_MOVES 45

// Order s.t. / 3 gives axis and % 3 power; X1 clockwise, X3 counter-clockwise
#define U1 0
#define U2 1
#define U3 2
#define R1 3
#define R2 4
#define R3 5
#define F1 6
#define F2 7
#define F3 8
#define D1 9
#define D2 10
#define D3 11
#define L1 12
#define L2 13
#define L3 14
#define B1 15
#define B2 16
#define B3 17

extern std::string kMoveNames[MAX_MOVES];
extern int kPhase2Moves[MAX_MOVES];
extern int kInvMove[MAX_MOVES];

const CubieCube kUCube = { 
  {UBR, URF, UFL, ULB, DFR, DLF, DBL, DRB},
  {UB, UR, UF, UL, DR, DF, DL, DB, FR, FL, BL, BR},
  {}, {}
};
const CubieCube kRCube = {
  {DFR, UFL, ULB, URF, DRB, DLF, DBL, UBR},
  {FR, UF, UL, UB, BR, DF, DL, DB, DR, FL, BL, UR},
  {2, 0, 0, 1, 1, 0, 0, 2}, {}
};
const CubieCube kFCube = {
  {UFL, DLF, ULB, UBR, URF, DFR, DBL, DRB},
  {UR, FL, UL, UB, DR, FR, DL, DB, UF, DF, BL, BR},
  {1, 2, 0, 0, 2, 1, 0, 0},
  {0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0}
};
const CubieCube kDCube = {
  {URF, UFL, ULB, UBR, DLF, DBL, DRB, DFR},
  {UR, UF, UL, UB, DF, DL, DB, DR, FR, FL, BL, BR},
  {}, {}
};
const CubieCube kLCube = {
  {URF, ULB, DBL, UBR, DFR, UFL, DLF, DRB},
  {UR, UF, BL, UB, DR, DF, FL, DB, FR, UL, DL, BR},
  {0, 1, 2, 0, 0, 2, 1, 0}, {}
};
const CubieCube kBCube = {
  {URF, UFL, UBR, DRB, DFR, DLF, ULB, DBL},
  {UR, UF, UL, BR, DR, DF, DL, BL, FR, FL, UB, DB},
  {0, 0, 1, 2, 0, 0, 2, 1},
  {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1}
};

extern CubieCube move_cubes[MAX_MOVES];

#endif
