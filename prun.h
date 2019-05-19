#ifndef PRUN_H_
#define PRUN_H_

#include <stdint.h>

#include "coord.h"
#include "sym.h"

#define N_FSTWIST (N_FSLICE_SYM * N_TWIST)
#define N_CORNUD (N_CORNERS_SYM * N_UDEDGES2)
#define N_CORNSLICE (N_CORNERS_C * N_SSLICE2)
#define N_FSSTWIST (uint64_t(N_FLIP) * N_SSLICE_SYM * N_TWIST)

#define FSTWIST(fssym, twist) (CoordL(fssym) * N_TWIST + twist)
#define CORNUD(csym, udedges) (CoordL(csym) * N_UDEDGES2 + udedges)

#define CORNSLICE(corners, sslice) (CoordL(corners) * N_SSLICE2 + sslice)
#define CS_CORNERS(cornslice) (cornslice / N_SSLICE2)
#define CS_SSLICE(cornslice) (cornslice % N_SSLICE2)

#define FSSTWIST(flip, sssym, twist) ((CoordLL(sssym) * N_FLIP + flip) * N_TWIST + twist)

extern int (*next_dist)[3];

extern uint64_t *fstwist_prun3;
extern uint64_t *cornud_prun3;
extern uint8_t *cornslice_prun;
extern uint64_t *fsstwist_prun3;

int getPrun3(uint64_t *prun3, CoordL c);
int getFSTwistDist(Coord flip, Coord sslice, Coord twist);
int getCornUDDist(Coord corners, Coord udedges);
int getFSSTwistDist(Coord flip, Coord sslice, Coord twist);

void initFSTwistPrun3();
void initCornUDPrun3();
void initCornSlicePrun();
void initFSSTwistPrun3();

#endif
