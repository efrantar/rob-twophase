#ifndef MOVES_H_
#define MOVES_H_

#include <string>
#include "cubie.h"

#ifdef AXIAL
  #define N_MOVES 45
  #define N_MOVES2 21
  #define N_AXES 3
#else
  #define N_MOVES 18
  #define N_MOVES2 10
  #define N_AXES 6
#endif

#define MOVEBIT(m) (MoveMask(1) << (m))

typedef uint64_t MoveMask;

extern std::string move_names[N_MOVES];
extern int inv_move[N_MOVES];
extern int split[N_MOVES]; // TODO: this is a bit ugly
extern MoveMask movemasks[N_MOVES + 1];
extern MoveMask all_movemask;
extern MoveMask extra_movemask;

extern int moves2[N_MOVES2];

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

extern CubieCube move_cubes[N_MOVES];

#endif
