#ifndef PRUN_H_
#define PRUN_H_

#include <stdint.h>

#include "coord.h"
#include "sym.h"

#define N_FSSYMTWIST_COORDS (N_FLIPSLICE_SYM_COORDS * N_TWIST_COORDS)

#define FSSYMTWIST(fs_sym, twist) (fs_sym * N_TWIST_COORDS + twist)

extern uint64_t *fssymtwist_prun;

void getDepthFSSymTwist(LargeCoord flipslicetwist);

void initFSSymTwistPrun();

#endif

