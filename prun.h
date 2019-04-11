#ifndef PRUN_H_
#define PRUN_H_

#include <stdint.h>

#include "coord.h"
#include "sym.h"

#define N_FSSYMTWIST_COORDS (N_FLIPSLICE_SYM_COORDS * N_TWIST_COORDS)
#define N_CSYMUDEDGES_COORDS (N_CORNERS_SYM_COORDS * N_UDEDGES_COORDS_P2)

#define FSSYMTWIST(fssym, twist) (fssym * N_TWIST_COORDS + twist)
#define CSYMUDEDGES(csym, udedges) (csym * N_UDEDGES_COORDS_P2 + udedges)

extern uint64_t *fssymtwist_prun3;
extern uint64_t *csymudedges_prun3;

void getDepthFSSymTwist(LargeCoord fssymtwist);
void getDepthCSymUDEdges(LargeCoord udsymcorners);

void initFSSymTwistPrun3();
void initCSymUDEdgesPrun3();

#endif

