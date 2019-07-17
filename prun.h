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

#ifdef FACES5
  #define MAX_DEPTH_P2 12
#else
  #ifdef AXIAL
    #define MAX_DEPTH_P2 6
  #else
    #define MAX_DEPTH_P2 10
  #endif
#endif

#define FSTWIST(fssym, twist) (CCoord(fssym) * N_TWIST + twist)
#define CORNED(csym, udedges) (CCoord(csym) * N_UDEDGES2 + udedges)

#define CORNSLICE(cperm, sslice) (CCoord(cperm) * N_SSLICE2 + sslice)
#define CS_CORNERS(cornslice) (cornslice / N_SSLICE2)
#define CS_SSLICE(cornslice) (cornslice % N_SSLICE2)

// Get new dist from current dist and mod 3 pruning value
extern int (*next_dist)[3];

extern uint64_t *fstwist_prun3;
extern uint64_t *corned_prun; // full resolution; 16 entries per cell
extern uint8_t *cornslice_prun; // phase 2 precheck; comparatively small -> just 1 entry per cell

int getFSTwistDist(Coord flip, Coord sslice, Coord twist);
int getFSTwistPrun3(Coord flip, Coord sslice, Coord twist);
int getCornEdPrun(Coord cperm, Coord udedges);
int getCornSlicePrun(Coord cperm, Coord sslice);

void initFSTwistPrun3();
void initCornEdPrun();
void initCornSlicePrun();

#endif
