#ifndef SYM_H_
#define SYM_H_

#include <bitset>
#include <stdint.h>
#include "coord.h"
#include "cubie.h"

#define N_SYMS 48
#define N_SYMS_DH4 16

#define N_FSLICE_SYM 64430
#define N_CORNERS_SYM 2768
#define N_SSLICE_SYM 788

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
extern Coord (*conj_flip)[N_SYMS_DH4][N_SSLICE_SYM];

extern SymCoord *fslice_sym;
extern SymCoord *corners_sym;
extern SymCoord *sslice_sym;
extern CoordL *fslice_raw;
extern Coord *corners_raw;
extern Coord *sslice_raw;
extern SelfSyms *fslice_selfs;
extern SelfSyms *corners_selfs;
extern SelfSyms *sslice_selfs;

void initConjTwist();
void initConjUDEdges();
void initConjFlip();

void initFlipSliceSym();
void initCornersSym();
void initSSliceSym();

#endif
