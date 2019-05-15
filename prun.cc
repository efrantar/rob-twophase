#include "prun.h"

#include <algorithm>
#include <bitset>
#include <queue>

#define BACKSEARCH_DEPTH 9
#define MAX_DEPTH_P2 10
#define EMPTY 0x3
#define EMPTY_CELL ~uint64_t(0)

int (*next_dist)[3];

uint64_t *fssymtwist_prun3;
uint64_t *csymudedges_prun3;
uint8_t *cornersslices_prun;

void initPrun() {
 next_dist = new int[22][3];
 
 for (int i = 0; i < 22; i++) {
   if (i > 0)
     next_dist[i][(i - 1) % 3] = i - 1;
   next_dist[i][i % 3] = i;
   next_dist[i][(i + 1) % 3] = i + 1;
 }
}

int getPrun3(uint64_t *prun3, CoordL c) {
  uint64_t tmp = prun3[c / 32];
  tmp >>= (c % 32) * 2;
  return tmp & 0x3;
}

void setPrun3(uint64_t *prun3, CoordL c, int depth) {
  int shift = (c % 32) * 2;
  prun3[c / 32] &= ~(uint64_t(0x3) << shift) | (uint64_t(depth % 3) << shift);
}

int getDepthFSSymTwistPrun3(Coord flip, Coord slicesorted, Coord twist) {
  CoordL flipslice = FSLICE(flip, SS_SLICE(slicesorted));
  CoordL fssymtwist = FSSYMTWIST(
    flipslice_sym[flipslice], conj_twist[twist][flipslice_sym_sym[flipslice]]
  );

  int depth3 = getPrun3(fssymtwist_prun3, fssymtwist);
  int depth = 0;

  while (fssymtwist != 0) {
    if (depth3 == 0)
      depth3 = 3;

    for (int m = 0; m < N_MOVES; m++) {
      Coord flip1 = flip_move[flip][m];
      Coord slicesorted1 = sslice_move[slicesorted][m];
      Coord twist1 = twist_move[twist][m];
      CoordL flipslice1 = FSLICE(flip1, SS_SLICE(slicesorted1));
      
      CoordL fssymtwist1 = FSSYMTWIST(
        flipslice_sym[flipslice1], conj_twist[twist1][flipslice_sym_sym[flipslice1]]
      );

      if (getPrun3(fssymtwist_prun3, fssymtwist1) == depth3 - 1) {
        flip = flip1;
        slicesorted = slicesorted1;
        twist = twist1;
        fssymtwist = fssymtwist1;
        break;
      }
    }

    depth3--;
    depth++;
  }

  return depth;
}

int getDepthCSymUDEdgesPrun3(Coord corners, Coord udedges) {
  CoordL csymudedges = CSYMUDEDGES(
    corners_sym[corners], conj_udedges[udedges][corners_sym_sym[corners]]
  );

  int depth3 = getPrun3(csymudedges_prun3, csymudedges);
  if (depth3 == EMPTY)
    return MAX_DEPTH_P2 + 1;

  int depth = 0;
  while (csymudedges != 0) {
    if (depth3 == 0)
      depth3 = 3;

    for (int m = 0; m < N_MOVES2; m++) {
      Coord corners1 = corners_move[corners][kPhase2Moves[m]];
      Coord udedges1 = udedges_move[udedges][m];
      CoordL csymudedges1 = CSYMUDEDGES(
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
    CoordL c = 0;
    int depth3 = depth % 3;
    
    for (SymCoord fssym = 0; fssym < N_FLIPSLICE_SYM_COORDS; fssym++) {
      Coord flip = FS_FLIP(flipslice_raw[fssym]);
      Coord slice = FS_SLICE(flipslice_raw[fssym]);

      for (Coord twist = 0; twist < N_TWIST; twist++, c++) {
        if (!backsearch) {
          if (
            c % 32 == 0 && 
            fssymtwist_prun3[c / 32] == EMPTY_CELL && 
            twist < N_TWIST - 32
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
          CoordL flipslice1 = FSLICE(flip1, slice1);
          Coord twist1 = twist_move[twist][m];

          SymCoord fssym1 = flipslice_sym[flipslice1];
          twist1 = conj_twist[twist1][flipslice_sym_sym[flipslice1]];
          CoordL c1 = FSSYMTWIST(fssym1, twist1);

          if (backsearch) {
            if (getPrun3(fssymtwist_prun3, c1) != depth3)
              continue;
            setPrun3(fssymtwist_prun3, c, depth + 1);
            filled++;
            break;
          } else if (getPrun3(fssymtwist_prun3, c1) != EMPTY)
            continue;
     
          setPrun3(fssymtwist_prun3, c1, depth + 1);
          filled++;
            
          SymSet symset = flipslice_symset[fssym1] >> 1;
          for (Sym s = 1; symset > 0; symset >>= 1, s++) {
            if (symset & 1) {
              CoordL c2 = FSSYMTWIST(fssym1, conj_twist[twist1][s]);
              if (getPrun3(fssymtwist_prun3, c2) == EMPTY) {
                setPrun3(fssymtwist_prun3, c2, depth + 1);
                bits->set(c2);
                filled++;
              }
            }
          }
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
    CoordL c = 0;
    int depth3 = depth % 3;

    for (SymCoord csym = 0; csym < N_CORNERS_SYM_COORDS; csym++) {
      for (Coord udedges = 0; udedges < N_UDEDGES2; udedges++, c++) {
        if (
          c % 32 == 0 && 
          csymudedges_prun3[c / 32] == EMPTY_CELL && 
          udedges < N_UDEDGES2 - 32
        ) {
          udedges += 31;
          c += 31;
          continue;
        }
        if (bits->test(c) || getPrun3(csymudedges_prun3, c) != depth3)
          continue;
        bits->set(c);

        for (int m = 0; m < N_MOVES2; m++) {
          Coord corners1 = corners_move[corners_raw[csym]][kPhase2Moves[m]];
          Coord udedges1 = udedges_move[udedges][m];
          SymCoord csym1 = corners_sym[corners1];
          udedges1 = conj_udedges[udedges1][corners_sym_sym[corners1]];
          CoordL c1 = CSYMUDEDGES(csym1, udedges1);

          if (getPrun3(csymudedges_prun3, c1) != EMPTY)
            continue;
    
          setPrun3(csymudedges_prun3, c1, depth + 1);
          filled++;
            
          SymSet symset = corners_symset[csym1] >> 1;
          for (Sym s = 1; symset > 0; symset >>= 1, s++) {
            if (symset & 1) {
              CoordL c2 = CSYMUDEDGES(csym1, conj_udedges[udedges1][s]);
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

  std::queue<CoordL> q;
  cornersslices_prun[0] = 0;
  q.push(0);
  
  while (q.size() > 0) {
    CoordL c = q.front();
    q.pop();
    Coord corners = CS_CORNERS(c);
    Coord slicesorted = CS_SLICESORTED(c);

    for (int m : kPhase2Moves) {
      CoordL c1 = CORNERSSLICES(
        corners_move[corners][m], sslice_move[slicesorted][m]
      );
      if (cornersslices_prun[c1] == 0xff) {
        cornersslices_prun[c1] = cornersslices_prun[c] + 1;
        q.push(c1);
      }
    }
  }
}
