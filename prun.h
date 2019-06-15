/**
 * Pruning tables
 */

#ifndef PRUN_H_
#define PRUN_H_

#include <stdint.h>
#include "coord.h"
#include "sym.h"

#define N_FSTWIST (N_FSLICE_SYM * N_TWIST)
#define N_CORNED (N_CPERM_SYM * N_UDEDGES2)
#define N_CORNSLICE (N_CPERM * N_SSLICE2)

#define FSTWIST(fssym, twist) (CCoord(fssym) * N_TWIST + twist)
#define CORNED(csym, udedges) (CCoord(csym) * N_UDEDGES2 + udedges)

#define CORNSLICE(cperm, sslice) (CCoord(cperm) * N_SSLICE2 + sslice)
#define CS_CORNERS(cornslice) (cornslice / N_SSLICE2)
#define CS_SSLICE(cornslice) (cornslice % N_SSLICE2)

// Get new dist from current dist and mod 3 pruning value
extern int (*next_dist)[3];

extern uint64_t *fstwist_prun3;
extern uint64_t *corned_prun3;
extern uint8_t *cornslice_prun; // phase 2 precheck; not mod 3

int getPrun3(uint64_t *prun3, CCoord c);
int getFSTwistDist(Coord flip, Coord sslice, Coord twist);
int getCornEdDist(Coord cperm, Coord udedges);

void initFSTwistPrun3();
void initCornUDPrun3();
void initCornSlicePrun();

#endif
