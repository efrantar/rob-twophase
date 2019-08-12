/**
 * This header defines the generation as well as the lookup of the solver's pruning tables.
 */

#ifndef PRUN_H_
#define PRUN_H_

#include <cstdint>
#include "coord.h"
#include "sym.h"

#define N_FSTWIST (N_FSLICE_SYM * N_TWIST) // phase 1 pruning table size
#define N_CORNED (N_CPERM_SYM * N_UDEDGES2) // phase 2 pruning table size
#define N_CORNSLICE (N_CPERM * N_SSLICE2) // phase 2 precheck table size

#define FSTWIST(fssym, twist) (fssym * N_TWIST + twist)
#define CORNED(csym, udedges) (csym * N_UDEDGES2 + udedges)
#define CORNSLICE(cperm, sslice) (cperm * N_SSLICE2 + sslice)

#define DIST(prun) (prun & 0xff) // get dist from phase 1 pruning table entry

// The phase 1 pruning table not only stores the distance to the (phase 1) solved state but also includes how this
// value changes for all moves, hence the size of an entry depends on the number of moves in the chosen metric. Since
// this is easy to handle and the table can be dramatically smaller when we are not in axial mode, we include a simple
// switch here.
#ifdef AXIAL
  typedef uint64_t Prun;
#else
  typedef uint32_t Prun;
#endif
extern Prun *fstwist_prun;

extern uint8_t *corned_prun;
extern uint8_t *cornslice_prun;

int getFSTwistPrun(int flip, Edges4 sslice, int twist, int togo, MoveMask &movemask);
int getCornEdPrun(CPerm cperm, Edges4 uedges, Edges4 dedges);
int getCornSlicePrun(CPerm cperm, Edges4 sslice);

// Pruning table generation routines; may take quite some time to execute
void initFSTwistPrun();
void initCornEdPrun();
void initCornSlicePrun();

// Initializes all auxiliary information necessary for pruning; to be called before accessing anything from this file
void initPrun();

#endif
