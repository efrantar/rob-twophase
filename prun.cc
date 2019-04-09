#include "prun.h"

#include <algorithm>

bool safe_memory = true;

unit8_t *phase1_prun;

void setFlipSliceTwist

void initFlipSliceTwistPrun() {
  int depth = 0;
  std::deque<LargeCoord> q;

  setPhase1Prun(0, 0);
  q.push(0);

  while (q.size() != 0) {
    LargeCoord c = q.pop();
    

    for (int m = 0; m < N_MOVES; m++) {
    
    }
  }

  bool backsearch = false;

  for (Sym s = 0; s < N_SYMS_DH4; s++)

  for (
    SymCoord flipslice_sym = 0; 
    flipslice_sym < N_FLIPSLICE_SYM_COORDS; 
    flipslice_sym++
  ) {
    for (Coord twist = 0; twist < N_TWIST_COORDS; twist++) {
    }
  }
}

