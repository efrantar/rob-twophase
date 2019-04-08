#ifndef SYM_H_
#define SYM_H_

#include <stdint.h>
#include "coord.h"
#include "cubie.h"

#define N_SYMS 48
#define N_SYMS_DH4 16

#define N_FLIPSLICE_SYM_COORDS 63790 // TODO: 64430 
#define N_CORNERS_SYM_COORDS 2768

#define FLIPSLICE(flip, slice) ((LargeCoord) slice * N_FLIP_COORDS + flip)
#define FS_FLIP(flipslice) (flipslice % N_FLIP_COORDS)
#define FS_SLICE(flipslice) (flipslice / N_FLIP_COORDS)

typedef uint8_t Sym;
typedef uint16_t SymCoord;
typedef int LargeCoord;
typedef uint16_t SymSet;

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
extern Sym inv_sym[N_SYMS];
extern int conj_move[N_MOVES][N_SYMS];

extern Coord (*conj_twist)[N_SYMS_DH4];
extern Coord (*conj_udedges)[N_SYMS_DH4];

extern SymCoord *flipslice_sym;
extern Sym *flipslice_sym_sym;
extern LargeCoord *flipslice_raw;
extern SymSet *flipslice_symset;

extern SymCoord *corners_sym;
extern Sym *corners_sym_sym;
extern Coord *corners_raw;
extern Coord *corners_symset;

void initSyms();

void initConjTwist();
void initConjUDEdges();

void initFlipSliceSyms();
void initCornersSyms();

#endif

