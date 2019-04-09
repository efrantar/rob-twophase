#ifndef PRUN_H_
#define PRUN_H_

#include <stdint.h>

#define FSSYMTWIST(fs_sym, twist) (fs_sym * N_TWIST_COORDS + twist)
#define FSST_FSS(fssymtwist) (fssymtwist / N_TWIST_COORDS)
#define FSST_FSS(fssymtwist) (fssymtwist % N_TWIST_COORDS)

extern bool safe_memory;

extern uint8_t *fssymtwist_prun;

void getDepthFSSymTwist(LargeCoord flipslicetwist);

void initFSSymTWistPrun();

#endif

