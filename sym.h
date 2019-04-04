#ifndef SYM_H_
#define SYM_H_

#include <stdint.h>
#include "coord.h"
#include "cubie.h"

#define N_SYMS 48
#define N_SYMS_DH4 16

#define N_FLIPSLICE_CLASSES 
#define N_CORNERS_CLASSES 

typedef uint8_t Sym;
typedef uint16_t Class;

const CubieCube kURF3Cube = {
  {URF, DFR, DLF, UFL, UBR, DRB, DBL, ULB},
  {UF, FR, DF, FL, UB, BR, DB, BL, UR, DR, DL, UL},
  {1, 2, 1, 2, 2, 1, 2, 1},
  {1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1}
};
const CubieCube kF2Cube = {
  {DLF, DFR, DRB, DBL, UFL, URF, UBR, ULB},
  {DL, DF, DR, DB, UL, UF, UR, UB, FL, FR, BR, BL},
  {}, {}
};
const CubieCube kU4Cube = {
  {UBR, URF, UFL, ULB, DRB, DFR, DLF, DBL},
  {UB, UR, UF, UL, DB, DR, DF, DL, BR, FR, FL, BL}
  {}, {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1}
};
const CubieCube kLR2Cube = {
  {UFL, URF, UBR, ULB, DLF, DFR, DRB, DBL},
  {UL, UF, UR, UB, DL, DF, DR, DB, FL, FR, BR, BL}
  {3, 3, 3, 3, 3, 3, 3, 3}, {}
};

extern CubieCube sym_cubes[N_SYMS];
extern Sym inv_sym[N_SYMS];
extern int conj_move[N_MOVES][N_SYMS];

extern Coord *conj_twist[N_SYMS_DH4];
extern Coord *conj_udedges[N_SYMS_DH4];

extern Class flipslice_class[N_FLIPSLICE_COORDS];
extern Sym flipslice_sym[N_FLIPSLICE_COORDS];
extern Coord flipslice_rep[N_FLIPSLICE_CLASSES];

extern Class corners_class[N_CORNERS_COORDS];
extern Sym corners_sym[N_CORNERS_COORDS];
extern Coord corners_rep[N_CORNERS_CLASSES];

void initSyms();

void initConjTwist();
void initConjUDEdges();

void initFlipSliceSyms();
void initCornerSyms();

#endif

