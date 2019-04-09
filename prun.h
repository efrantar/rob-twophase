#ifndef PRUN_H_
#define PRUN_H_

#include <stdint.h>

#define N_FSSYMTWIST_COORDS (N_FLIPSLICE_SYM_COORDS * N_TWIST_COORDS)

#define FSSYMTWIST(fs_sym, twist) (fs_sym * N_TWIST_COORDS + twist)

extern uint8_t *fssymtwist_prun;

void getDepthFSSymTwist(LargeCoord flipslicetwist);

void initFSSymTWistPrun();

#endif

