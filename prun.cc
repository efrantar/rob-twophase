#include "prun.h"

#include <algorithm>

#define EMPTY 0x3

uint8_t *phase1_prun;

void getFSSymTwistPrun(LargeCoord c) {
}

int getFSSymTwistPrun(LargeCoord c, int depth) {
}

void 

void initFSSymTwistPrun() {
  phase1_prun = new uint8_t[N_FSSYMTWIST_COORDS];
  
  int filled = 0;
  int depth = 0;

  while (filled < N_FSSYMTWIST_COORDS) {
    LargeCoord c = 0;
    for (
      SymCoord flipslice_sym = 0;
      flipslice_sym < N_FLIPSLICE_SYM_COORDS;
      flipslice_sym++
    ) {
      Coord flip = FS_FLIP(flipslice_raw[flipslice_sym]);
      Coord slice = FS_SLICE(flipslice_raw[flipslice_sym]);

      for (Coord twist = 0; twist < N_TWIST_COORDS; twist++, c++) { 
        if (bitset[c] || getFSSymTwistPrun(c) != EMPTY)
          continue;

        for (int m = 0; m < N_MOVES; m++) {
          Coord flip1 = flip_move[flip][m];
          Coord slice1 = sliceMove(slice1, m);
          Coord twist1 = twist_move[twist][m];
          SymCoord flipslice1_sym = flipslice_sym[FLIPSLICE(flip1, slice1)];

          LargeCoord c1 = FSSYMTWIST(flipslice1_sym, twist1);
          if (getFSSymTwistPrun(c1) == EMPTY) {
            setFSSymTwistPrun(c1, depth1 + 1);
            filled++;
            
            SymSet symset = flipslice_symset[flipslice1_sym];
            if (symset > 1) {
              Sym s = 1;
              while (smyset > 0) {
                symset >>= 1;
                if (symset & 1) {
                  LargeCoord c2 = FSSYMTWIST(flipslice1_sym, conj_twist[twist1][s]);
                  if (getFSSymTwistPrun(c2) == EMPTY) {
                    setFSSymTWistPrun(c2, depth + 1);
                    filled++;
                  }
                }
                s++;
              }
            }
          }
        }
      }
    }
  } 
}

