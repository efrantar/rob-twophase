#ifndef PRUN_H_
#define PRUN_H_

#include <stdint.h>

#include "coord.h"
#include "sym.h"

#define N_FSSYMTWIST_COORDS (N_FLIPSLICE_SYM_COORDS * N_TWIST_COORDS)
#define N_CSYMUDEDGES_COORDS (N_CORNERS_SYM_COORDS * N_UDEDGES_COORDS_P2)
#define N_CORNERSSLICES_COORDS (N_CORNERS_COORDS * N_SLICESORTED_COORDS_P2)

#define FSSYMTWIST(fssym, twist) (fssym * N_TWIST_COORDS + twist)
#define CSYMUDEDGES(csym, udedges) (csym * N_UDEDGES_COORDS_P2 + udedges)

#define CORNERSSLICES(corners, slicesorted) \
  (LargeCoord(corners) * N_SLICESORTED_COORDS_P2 + LargeCoord(slicesorted))
#define CS_CORNERS(cornersslices) (cornersslices / N_SLICESORTED_COORDS_P2)
#define CS_SLICESORTED(cornersslices) (cornersslices % N_SLICESORTED_COORDS_P2)

extern int (*next_depth)[3];

extern uint64_t *fssymtwist_prun3;
extern uint64_t *csymudedges_prun3;
extern uint8_t *cornersslices_prun;

void getDepthFSSymTwist(LargeCoord fssymtwist);
void getDepthCSymUDEdges(LargeCoord udsymcorners);

void initPrun();
void initFSSymTwistPrun3();
void initCSymUDEdgesPrun3();
void initCornersSliceSPrun();

#endif

