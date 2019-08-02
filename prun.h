/**
 * Pruning tables
 */

#ifndef PRUN_H_
#define PRUN_H_

#include <stdint.h>
#include "coord.h"
#include "sym.h"

#define AXIS_BITMASK ((1 << N_PER_AXIS) - 1)
#define N_INFO 4
#ifdef QUARTER
  #define N_PER_MOVE 2
#else
  #define N_PER_MOVE 1
#endif

#define N_FSTWIST (N_FSLICE_SYM * N_TWIST)
#define N_CORNED (N_CPERM_SYM * N_UDEDGES2)
#define N_CORNSLICE (N_CPERM * N_SSLICE2)

#define FSTWIST(fssym, twist) (fssym * N_TWIST + twist)
#define CORNED(csym, udedges) (csym * N_UDEDGES2 + udedges)
#define CORNSLICE(cperm, sslice) (cperm * N_SSLICE2 + sslice)

#define DIST(prun) (prun & 0xff)
#define OFF(key) (N_PER_AXIS * (key >> 2))
#define INFO(key) (key & 0x3)

#ifdef AXIAL
  typedef uint64_t Prun;
  typedef uint16_t MMChunk;
#else
  typedef uint32_t Prun;
  typedef uint8_t MMChunk;
#endif

extern uint8_t mm_key[N_SYMS_SUB][N_AXES];
extern MMChunk mm_map[2][N_INFO][1 << (N_PER_AXIS + 1)];
extern Prun *fstwist_prun;

extern uint8_t *corned_prun; // full resolution; 16 entries per cell
extern uint8_t *cornslice_prun; // phase 2 precheck; comparatively small -> just 1 entry per cell

int getFSTwistPrun(int flip, Edges4 sslice, int twist, int togo, MoveMask &movemask);
int getCornEdPrun(CPerm cperm, Edges4 uedges, Edges4 dedges);
int getCornSlicePrun(CPerm cperm, Edges4 sslice);

void initFSTwistPrun();
void initCornEdPrun();
void initCornSlicePrun();

void initPrun();

#endif
