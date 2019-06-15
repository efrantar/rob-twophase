#ifndef SYM_H_
#define SYM_H_

#include <stdint.h>
#include "coord.h"
#include "cubie.h"

#define N_SYMS 48

#ifdef FACES5
  #define N_SYMS_SUB 4 // only 4 symmetries applicable when using 5 faces
  #define N_FSLICE_SYM 255664
  #define N_CPERM_SYM 10368
#else
  #define N_SYMS_SUB 16 // #symmetries used for the reductions (sub-group DH4)
  #define N_FSLICE_SYM 64430
  #define N_CPERM_SYM 2768
#endif

#define SYMCOORD(coord, sym) (SymCoord(coord) * N_SYMS_SUB + sym)
#define SYM(scoord) (scoord % N_SYMS_SUB)
#define COORD(scoord) (scoord / N_SYMS_SUB)

// Store the class index and the symmetry in a single coordinate to avoid an extra table lookup
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

extern Coord (*conj_twist)[N_SYMS_SUB];
extern Coord (*conj_udedges)[N_SYMS_SUB];

extern SymCoord *fslice_sym;
extern SymCoord *cperm_sym;
extern CCoord *fslice_raw;
extern Coord *cperm_raw;
extern SelfSyms *fslice_selfs;
extern SelfSyms *cperm_selfs;

void initConjTwist();
void initConjUDEdges();

void initFlipSliceSym();
void initCPermSym();

#endif
