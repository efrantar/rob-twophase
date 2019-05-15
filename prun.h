#ifndef PRUN_H_
#define PRUN_H_

#include <stdint.h>

#include "coord.h"
#include "sym.h"

#define N_FSSYMTWIST_COORDS (N_FLIPSLICE_SYM_COORDS * N_TWIST)
#define N_CSYMUDEDGES_COORDS (N_CORNERS_SYM_COORDS * N_UDEDGES2)
#define N_CORNERSSLICES_COORDS (N_CORNERS_C * N_SSLICE2)

#define FSSYMTWIST(fssym, twist) (fssym * N_TWIST + twist)
#define CSYMUDEDGES(csym, udedges) (csym * N_UDEDGES2 + udedges)

#define CORNERSSLICES(corners, slicesorted) \
  (CoordL(corners) * N_SSLICE2 + CoordL(slicesorted))
#define CS_CORNERS(cornersslices) (cornersslices / N_SSLICE2)
#define CS_SLICESORTED(cornersslices) (cornersslices % N_SSLICE2)

extern int (*next_dist)[3];

extern uint64_t *fssymtwist_prun3;
extern uint64_t *csymudedges_prun3;
extern uint8_t *cornersslices_prun;

int getPrun3(uint64_t *prun3, CoordL c);
int getDepthFSSymTwistPrun3(Coord flip, Coord slice, Coord twist);
int getDepthCSymUDEdgesPrun3(Coord corners, Coord udedges);

void initPrun();
void initFSSymTwistPrun3();
void initCSymUDEdgesPrun3();
void initCornersSliceSPrun();

#endif
