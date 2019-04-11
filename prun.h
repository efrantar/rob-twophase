#ifndef PRUN_H_
#define PRUN_H_

#include <stdint.h>

#include "coord.h"
#include "sym.h"

#define N_FSSYMTWIST_COORDS (N_FLIPSLICE_SYM_COORDS * N_TWIST_COORDS)
#define N_UDSYMCORNERS_COORDS (N_UDEDGES_SYM_COORDS * N_CORNERS_COORDS)

#define FSSYMTWIST(fs_sym, twist) (fs_sym * N_TWIST_COORDS + twist)
#define UDSYMCORNERS(ud_sym, corners) (ud_sym * N_CORNERS_COORDS + corners)

extern uint64_t *fssymtwist_prun3;
extern uint64_t *udsymcorners_prun3;

void getDepthFSSymTwist(LargeCoord fssymtwist);
void getDepthUDSymCorners(LargeCoord udsymcorners);

void initFSSymTwistPrun3();
void initUDSymCornersPrun3();

#endif

