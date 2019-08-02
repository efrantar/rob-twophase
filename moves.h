#ifndef MOVES_H_
#define MOVES_H_

#include <stdint.h>
#include <string>
#include "cubie.h"

#ifdef QUARTER
  #ifdef AXIAL
    #define N_MOVES 24
    #define N_DOUBLE2 6
  #else
    #define N_MOVES 12
    #define N_DOUBLE2 4
  #endif
#else
  #ifdef AXIAL
    #define N_MOVES 45
  #else
    #define N_MOVES 18
  #endif
  #define N_DOUBLE2 0
#endif

#define MOVEBIT(m) (MoveMask(1) << m)

typedef uint64_t MoveMask;

extern CubieCube move_cubes[N_MOVES + N_DOUBLE2];
extern std::string move_names[N_MOVES + N_DOUBLE2];
extern int inv_move[N_MOVES + N_DOUBLE2];

extern MoveMask next_moves[N_MOVES + N_DOUBLE2];
extern MoveMask phase1_moves;
extern MoveMask phase2_moves;

const CubieCube kUCube = {
  {UBR, URF, UFL, ULB, DFR, DLF, DBL, DRB},
  {UB, UR, UF, UL, DR, DF, DL, DB, FR, FL, BL, BR},
  {}, {}
};
const CubieCube kDCube = {
  {URF, UFL, ULB, UBR, DLF, DBL, DRB, DFR},
  {UR, UF, UL, UB, DF, DL, DB, DR, FR, FL, BL, BR},
  {}, {}
};
const CubieCube kRCube = {
  {DFR, UFL, ULB, URF, DRB, DLF, DBL, UBR},
  {FR, UF, UL, UB, BR, DF, DL, DB, DR, FL, BL, UR},
  {2, 0, 0, 1, 1, 0, 0, 2}, {}
};
const CubieCube kLCube = {
  {URF, ULB, DBL, UBR, DFR, UFL, DLF, DRB},
  {UR, UF, BL, UB, DR, DF, FL, DB, FR, UL, DL, BR},
  {0, 1, 2, 0, 0, 2, 1, 0}, {}
};
const CubieCube kFCube = {
  {UFL, DLF, ULB, UBR, URF, DFR, DBL, DRB},
  {UR, FL, UL, UB, DR, FR, DL, DB, UF, DF, BL, BR},
  {1, 2, 0, 0, 2, 1, 0, 0},
  {0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0}
};
const CubieCube kBCube = {
  {URF, UFL, UBR, DRB, DFR, DLF, ULB, DBL},
  {UR, UF, UL, BR, DR, DF, DL, BL, FR, FL, UB, DB},
  {0, 0, 1, 2, 0, 0, 2, 1},
  {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1}
};

#endif
