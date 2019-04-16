#include "prun.h"

#include <algorithm>
#include <bitset>
#include <queue>

#define BACKSEARCH_DEPTH 9
#define MAX_DEPTH_P2 10
#define EMPTY 0x3
#define EMPTY_CELL ~uint64_t(0)

int (*next_depth)[3];

uint64_t *fssymtwist_prun3;
uint64_t *csymudedges_prun3;
uint8_t *cornersslices_prun;

void initPrun() {
 next_depth = new int[22][3];
 for (int i = 1; i < 22; i++) {
   next_depth[i][(i - 1) % 3] = i - 1;
   next_depth[i][i % 3] = i;
   next_depth[i][(i + 1) % 3] = i + 1;
 }
}

int getPrun3(uint64_t *prun3, LargeCoord c) {
  uint64_t tmp = prun3[c / 32];
  tmp >>= (c % 32) * 2;
  return tmp & 0x3;
}

void setPrun3(uint64_t *prun3, LargeCoord c, int depth) {
  int shift = (c % 32) * 2;
  prun3[c / 32] &= ~(uint64_t(0x3) << shift) | (uint64_t(depth % 3) << shift);
}

int getDepthFSSymTwistPrun3(Coord flip, Coord slicesorted, Coord twist) {
  LargeCoord flipslice = FLIPSLICE(flip, SS_SLICE(slicesorted));
  LargeCoord fssymtwist = FSSYMTWIST(
    flipslice_sym[flipslice], conj_twist[twist][flipslice_sym_sym[flipslice]]
  );

  int depth3 = getPrun3(fssymtwist_prun3, fssymtwist);
  int depth = 0;

  while (fssymtwist != 0) {
    if (depth3 == 0)
      depth3 = 3;

    for (int m = 0; m < N_MOVES; m++) {
      Coord flip1 = flip_move[flip][m];
      Coord slicesorted1 = slicesorted_move[slicesorted][m];
      Coord twist1 = twist_move[twist][m];
      LargeCoord flipslice1 = FLIPSLICE(flip1, SS_SLICE(slicesorted1));
      
      LargeCoord fssymtwist1 = FSSYMTWIST(
        flipslice_sym[flipslice1], conj_twist[twist1][flipslice_sym_sym[flipslice1]]
      );

      // std::cout << flip1 << " " << slicesorted1 << " " << twist1 << " " << flipslice1 << "\n";

      if (getPrun3(fssymtwist_prun3, fssymtwist1) == depth3 - 1) {
        // std::cout << flip1 << " " << slicesorted1 << " " << twist1 << " " << flipslice1 << "\n";
        flip = flip1;
        slicesorted = slicesorted1;
        twist = twist1;
        fssymtwist = fssymtwist1;
        break;
      }
    }

    depth3--;
    depth++;

    if (depth == 50)
      return -10000;
  }

  return depth;
}

int getDepthCSymUDEdgesPrun3(Coord corners, Coord udedges) {
  LargeCoord csymudedges = CSYMUDEDGES(
    corners_sym[corners], conj_udedges[udedges][corners_sym_sym[corners]]
  );

  int depth3 = getPrun3(csymudedges_prun3, csymudedges);
  if (depth3 == EMPTY)
    return MAX_DEPTH_P2 + 1;

  int depth = 0;
  while (csymudedges != 0) {
    if (depth3 == 0)
      depth3 = 3;

    for (int m = 0; m < N_MOVES_P2; m++) {
      Coord corners1 = corners_move[corners][kPhase2Moves[m]];
      Coord udedges1 = udedges_move[udedges][m];
      LargeCoord csymudedges1 = CSYMUDEDGES(
        corners_sym[corners1], conj_udedges[udedges1][corners_sym_sym[corners1]]
      );

      if (getPrun3(csymudedges_prun3, csymudedges1) == depth3 - 1) {
        corners = corners1;
        udedges = udedges1;
        csymudedges = csymudedges1;
        break;
      }
    }

    depth3--;
    depth++;
  }

  return depth;
}

void initFSSymTwistPrun3() {
  fssymtwist_prun3 = new uint64_t[N_FSSYMTWIST_COORDS / 32 + 1];
  std::fill(fssymtwist_prun3, fssymtwist_prun3 + N_FSSYMTWIST_COORDS / 32 + 1, EMPTY_CELL);

  int filled = 1;
  int depth = 0;
  std::bitset<N_FSSYMTWIST_COORDS> *bits = new std::bitset<N_FSSYMTWIST_COORDS>();
  bool backsearch = false;

  setPrun3(fssymtwist_prun3, 0, 0);
  while (filled < N_FSSYMTWIST_COORDS) {
    LargeCoord c = 0;
    int depth3 = depth % 3;

    for (SymCoord fssym = 0; fssym < N_FLIPSLICE_SYM_COORDS; fssym++) {
      Coord flip = FS_FLIP(flipslice_raw[fssym]);
      Coord slice = FS_SLICE(flipslice_raw[fssym]);

      for (Coord twist = 0; twist < N_TWIST_COORDS; twist++, c++) {
        if (!backsearch) {
          if (
            c % 32 == 0 && 
            fssymtwist_prun3[c / 32] == EMPTY_CELL && 
            twist < N_TWIST_COORDS - 32
          ) {
            twist += 31;
            c += 31;
            continue;
          }
          if (bits->test(c) || getPrun3(fssymtwist_prun3, c) != depth3)
            continue;
          bits->set(c);
        } else if (getPrun3(fssymtwist_prun3, c) != EMPTY)
          continue;

        for (int m = 0; m < N_MOVES; m++) {
          Coord flip1 = flip_move[flip][m];
          Coord slice1 = sliceMove(slice, m);
          LargeCoord flipslice1 = FLIPSLICE(flip1, slice1);
          Coord twist1 = twist_move[twist][m];

          SymCoord fssym1 = flipslice_sym[flipslice1];
          twist1 = conj_twist[twist1][flipslice_sym_sym[flipslice1]];
          LargeCoord c1 = FSSYMTWIST(fssym1, twist1);

          if (backsearch) {
            if (getPrun3(fssymtwist_prun3, c1) != depth3)
              continue;
            c1 = c;
            fssym1 = fssym;
          } else if (getPrun3(fssymtwist_prun3, c1) != EMPTY)
            continue;
     
          setPrun3(fssymtwist_prun3, c1, depth + 1);
          filled++;
            
          SymSet symset = flipslice_symset[fssym1] >> 1;
          for (Sym s = 1; symset > 0; symset >>= 1, s++) {
            if (symset & 1) {
              LargeCoord c2 = FSSYMTWIST(fssym1, conj_twist[twist1][s]);
              if (getPrun3(fssymtwist_prun3, c2) == EMPTY) {
                setPrun3(fssymtwist_prun3, c2, depth + 1);
                bits->set(c2);
                filled++;
              }
            }
          }

          if (backsearch)
            break;
        }
      }
    }

    depth++;
    if (depth == BACKSEARCH_DEPTH)
      backsearch = true;
  }

  delete bits;
}

void initCSymUDEdgesPrun3() {
  csymudedges_prun3 = new uint64_t[N_CSYMUDEDGES_COORDS / 32 + 1];
  std::fill(csymudedges_prun3, csymudedges_prun3 + N_CSYMUDEDGES_COORDS / 32 + 1, EMPTY_CELL);

  int filled = 1;
  int depth = 0;
  std::bitset<N_CSYMUDEDGES_COORDS> *bits = new std::bitset<N_CSYMUDEDGES_COORDS>();

  setPrun3(csymudedges_prun3, 0, 0);
  while (depth < MAX_DEPTH_P2) {
    LargeCoord c = 0;
    int depth3 = depth % 3;

    for (SymCoord csym = 0; csym < N_CORNERS_SYM_COORDS; csym++) {
      for (Coord udedges = 0; udedges < N_UDEDGES_COORDS_P2; udedges++, c++) {
        if (
          c % 32 == 0 && 
          csymudedges_prun3[c / 32] == EMPTY_CELL && 
          udedges < N_UDEDGES_COORDS_P2 - 32
        ) {
          udedges += 31;
          c += 31;
          continue;
        }
        if (bits->test(c) || getPrun3(csymudedges_prun3, c) != depth3)
          continue;
        bits->set(c);

        for (int m = 0; m < N_MOVES_P2; m++) {
          Coord corners1 = corners_move[corners_raw[csym]][kPhase2Moves[m]];
          Coord udedges1 = udedges_move[udedges][m];
          SymCoord csym1 = corners_sym[corners1];
          udedges1 = conj_udedges[udedges1][corners_sym_sym[corners1]];
          LargeCoord c1 = CSYMUDEDGES(csym1, udedges1);

          if (getPrun3(csymudedges_prun3, c1) != EMPTY)
            continue;
    
          setPrun3(csymudedges_prun3, c1, depth + 1);
          filled++;
            
          SymSet symset = corners_symset[csym1] >> 1;
          for (Sym s = 1; symset > 0; symset >>= 1, s++) {
            if (symset & 1) {
              LargeCoord c2 = CSYMUDEDGES(csym1, conj_udedges[udedges1][s]);
              if (getPrun3(csymudedges_prun3, c2) == EMPTY) {
                setPrun3(csymudedges_prun3, c2, depth + 1);
                bits->set(c2);
                filled++;
              }
            }
          }
        }
      }
    }

    depth++;
  }

  delete bits;
}

void initCornersSliceSPrun() {
  cornersslices_prun = new uint8_t[N_CORNERSSLICES_COORDS];
  std::fill(cornersslices_prun, cornersslices_prun + N_CORNERSSLICES_COORDS, 0xff);

  std::queue<LargeCoord> q;
  cornersslices_prun[0] = 0;
  q.push(0);
  
  while (q.size() > 0) {
    LargeCoord c = q.front();
    q.pop();
    Coord corners = CS_CORNERS(c);
    Coord slicesorted = CS_SLICESORTED(c);

    for (int m : kPhase2Moves) {
      LargeCoord c1 = CORNERSSLICES(
        corners_move[corners][m], slicesorted_move[slicesorted][m]
      );
      if (cornersslices_prun[c1] == 0xff) {
        cornersslices_prun[c1] = cornersslices_prun[c] + 1;
        q.push(c1);
      }
    }
  }
}

