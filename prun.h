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

#define FSTWIST(fssym, twist) (fssym * N_TWIST + twist)
#define CORNED(csym, udedges) (csym * N_UDEDGES2 + udedges)
#define CORNSLICE(cperm, sslice) (cperm * N_SSLICE2 + sslice)

#define DIST(prun) (prun & 0xff)
#define OFF(key) (key >> 2)
#define INFO(key) (key & 0x3)

#ifdef AXIAL
  typedef uint64_t Prun;
#else
  typedef uint32_t Prun;
#endif

extern uint8_t mm_key[N_SYMS_SUB][3];
extern uint8_t mm_map1[2][4][256];
#ifdef AXIAL
  #ifdef QUARTER
    extern uint8_t mm_map2[2][4][256];
  #else
    extern uint16_t mm_map2[2][4][1024];
  #endif
#endif
extern Prun *fstwist_prun;

extern uint8_t *corned_prun;
extern uint8_t *cornslice_prun;

int getFSTwistPrun(int flip, Edges4 sslice, int twist, int togo, MoveMask &movemask);
int getCornEdPrun(CPerm cperm, Edges4 uedges, Edges4 dedges);
int getCornSlicePrun(CPerm cperm, Edges4 sslice);

void initFSTwistPrun();
void initCornEdPrun();
void initCornSlicePrun();

void initPrun();

#endif
