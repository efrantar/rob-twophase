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
};
const CubieCube kF2Cube = {
};
const CubieCube kU4Cube = {
};
const CubieCube kLR2Cube = {
};

CubieCube sym_cubes[N_SYMS];
Sym inv_sym[N_SYMS];

Move conj_move[N_MOVE][N_SYMS];

Coord *conj_twist[N_SYMS_DH4];
Coord *conj_udedges[N_SYMS_DH4];

Class flipslice_class[N_FLIPSLICE_COORDS];
Sym flipslice_sym[N_FLIPSLICE_COORDS];
Coord flipslice_rep[N_FLIPSLICE_CLASSES];

Class corners_class[N_CORNERS_COORDS];
Sym corners_sym[N_CORNERS_COORDS];
Coord corners_rep[N_CORNERS_CLASSES];

#endif

