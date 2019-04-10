#include "prun.h"

#include <bitset>

#define BACKSEARCH_DEPTH 9
#define EMPTY 0x3

uint64_t *fssymtwist_prun;

int getFSSymTwistPrun(LargeCoord c) {
  uint64_t tmp = fssymtwist_prun[c / 16];
  tmp >>= (c % 16) * 2;
  return tmp & 0x3;
}

void setFSSymTwistPrun(LargeCoord c, int depth) {
  int shift = (c % 16) * 2;
  fssymtwist_prun[c / 16] &= ~(0x3 << shift) | (depth % 3) << shift;
}

void initFSSymTwistPrun() {
  fssymtwist_prun = new uint64_t[N_FSSYMTWIST_COORDS / 4];
  std::fill(fssymtwist_prun, fssymtwist_prun + N_FSSYMTWIST_COORDS / 4, 0xffffffff);
  
  int filled = 0;
  int depth = 0;
  std::bitset<N_FSSYMTWIST_COORDS> bits;
  bool backsearch = false;

  setFSSymTwistPrun(0, 0);
  while (filled < N_FSSYMTWIST_COORDS) {
    LargeCoord c = 0;
    for (SymCoord fssym = 0; fssym < N_FLIPSLICE_SYM_COORDS; fssym++) {
      Coord flip = FS_FLIP(flipslice_raw[fssym]);
      Coord slice = FS_SLICE(flipslice_raw[fssym]);

      for (Coord twist = 0; twist < N_TWIST_COORDS; ) {
        if (!backsearch) {
          if (c % 16 == 0 && fssymtwist_prun[c] == 0xffffffff) {
            twist += 16;
            c += 16;
            continue;
          }
          if (bits.test(c) || getFSSymTwistPrun(c) == EMPTY)
            continue;
          bits.set(c);
        } else if (getFSSymTwistPrun(c) != EMPTY)
         continue;

        for (int m = 0; m < N_MOVES; m++) {
          Coord flip1 = flip_move[flip][m];
          Coord slice1 = sliceMove(slice1, m);
          Coord twist1 = twist_move[twist][m];
          SymCoord fssym1 = flipslice_sym[FLIPSLICE(flip1, slice1)];

          LargeCoord c1 = FSSYMTWIST(fssym1, twist1);
          if (backsearch) {
            if (getFSSymTwistPrun(c1) != EMPTY)
              continue;
            c1 = c;
            fssym1 = fssym;
          } else if (getFSSymTwistPrun(c1) == EMPTY)
            continue;
     
          setFSSymTwistPrun(c1, depth + 1);
          filled++;
            
          SymSet symset = flipslice_symset[fssym1] >> 1;
          for (Sym s = 1; symset > 0; symset >>= 1, s++) {
            if (symset & 1) {
              LargeCoord c2 = FSSYMTWIST(fssym1, conj_twist[twist1][s]);
              if (getFSSymTwistPrun(c2) == EMPTY) {
                setFSSymTwistPrun(c2, depth + 1);
                filled++;
              }
            }
          }

          if (backsearch)
            break;
        }

        twist++;
        c++;
      }

      depth++;
      if (depth == BACKSEARCH_DEPTH)
        backsearch = true;
    }
  } 
}

