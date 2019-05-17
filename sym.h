#ifndef SYM_H_
#define SYM_H_

#include <bitset>
#include <stdint.h>
#include "coord.h"
#include "cubie.h"

#define N_SYMS 48
#define N_SYMS_DH4 16

#define N_FSLICE_SYM 64430
#define N_FSSLICE_SYM -1 // TODO:
#define N_CORNERS_SYM 2768

#define SYMCOORD(coord, sym) (SymCoord(coord) * N_SYMS_DH4 + sym)
#define SYM(scoord) (scoord % N_SYMS_DH4)
#define COORD(scoord) (scoord / N_SYMS_DH4)

typedef uint32_t SymCoord;
typedef uint16_t SelfSyms;

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
  {UB, UR, UF, UL, DB, DR, DF, DL, BR, FR, FL, BL},
  {}, {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1}
};
const CubieCube kLR2Cube = {
  {UFL, URF, UBR, ULB, DLF, DFR, DRB, DBL},
  {UL, UF, UR, UB, DL, DF, DR, DB, FL, FR, BR, BL},
  {3, 3, 3, 3, 3, 3, 3, 3}, {}
};

extern CubieCube sym_cubes[N_SYMS];
extern int inv_sym[N_SYMS];
extern int conj_move[N_MOVES][N_SYMS];

extern Coord (*conj_twist)[N_SYMS_DH4];
extern Coord (*conj_udedges)[N_SYMS_DH4];

extern SymCoord *fslice_sym;
extern CoordL *fslice_raw;
extern SelfSyms *fslice_selfs;

extern SymCoord *fsslice_sym;
extern CoordL *fsslice_raw;
extern SelfSyms *fsslice_selfs;

extern SymCoord *corners_sym;
extern CoordL *corners_raw;
extern SelfSyms *corners_selfs;

void checkSyms(const CubieCube &cube, bool &rot, bool &anti);

void initConjTwist();
void initConjUDEdges();

void initFlipSliceSym();
void initFlipSSliceSym();
void initCornersSym();

#endif
