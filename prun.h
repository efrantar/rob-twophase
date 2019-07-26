/**
 * Pruning tables
 */

#ifndef PRUN_H_
#define PRUN_H_

#include <stdint.h>
#include "coord.h"
#include "sym.h"

#define AXIS_BITMASK ((1 << N_PER_AXIS) - 1)
#define N_INFO 2

#define N_FSTWIST (N_FSLICE_SYM * N_TWIST)
#define N_CORNED (N_CPERM_SYM * N_UDEDGES2)
#define N_CORNSLICE (N_CPERM * N_SSLICE2)

#define FSTWIST(fssym, twist) (CCoord(fssym) * N_TWIST + twist)
#define CORNED(csym, udedges) (CCoord(csym) * N_UDEDGES2 + udedges)
#define CORNSLICE(cperm, sslice) (CCoord(cperm) * N_SSLICE2 + sslice)

#define DIST(prun) (prun & 0xff)
#define OFF(key) (N_PER_AXIS * (key >> 1))
#define INFO(key) (key & 1)

typedef uint32_t Prun;
typedef uint8_t MMChunk;
extern uint8_t mm_key[N_SYMS_SUB][N_AXES];
extern MMChunk mm_map[2][N_INFO][1 << (N_PER_AXIS + 1)];
extern Prun *fstwist_prun;

extern uint8_t *corned_prun; // full resolution; 16 entries per cell
extern uint8_t *cornslice_prun; // phase 2 precheck; comparatively small -> just 1 entry per cell

Prun getFSTwistPrun(Coord flip, Coord sslice, Coord twist);
MoveMask getFSTwistMoves(Coord flip, Coord sslice, Coord twist, int togo);
int getCornEdPrun(Coord cperm, Coord udedges);
int getCornSlicePrun(Coord cperm, Coord sslice);

void initFSTwistPrun();
void initCornEdPrun();
void initCornSlicePrun();

void initPrun();

#endif
