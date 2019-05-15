#include "prun.h"

#include <algorithm>
#include <bitset>
#include <queue>

#define BACKSEARCH_DEPTH 9
#define MAX_DEPTH_P2 10
#define EMPTY 0x3
#define EMPTY_CELL ~uint64_t(0)

int (*next_dist)[3];

uint64_t *fstwist_prun3;
uint64_t *cornud_prun3;
uint8_t *cornslice_prun;

static bool init() {
 next_dist = new int[22][3];
 
 for (int i = 0; i < 22; i++) {
   if (i > 0)
     next_dist[i][(i - 1) % 3] = i - 1;
   next_dist[i][i % 3] = i;
   next_dist[i][(i + 1) % 3] = i + 1;
 }
}
static bool inited = init();

int getPrun3(uint64_t *prun3, CoordL c) {
  uint64_t tmp = prun3[c / 32];
  tmp >>= (c % 32) * 2;
  return tmp & 0x3;
}

void setPrun3(uint64_t *prun3, CoordL c, int dist) {
  int shift = (c % 32) * 2;
  prun3[c / 32] &= ~(uint64_t(0x3) << shift) | (uint64_t(dist % 3) << shift);
}

int getFSTwistDist(Coord flip, Coord sslice, Coord twist) {
  CoordL fslice = FSLICE(flip, SS_SLICE(sslice));
  CoordL fstwist = FSTWIST(
    fslice_sym[fslice], conj_twist[twist][fslice_sym_sym[fslice]]
  );

  int depth3 = getPrun3(fstwist_prun3, fstwist);
  int depth = 0;

  while (fstwist != 0) {
    if (depth3 == 0)
      depth3 = 3;

    for (int m = 0; m < N_MOVES; m++) {
      Coord flip1 = flip_move[flip][m];
      Coord sslice1 = sslice_move[sslice][m];
      Coord twist1 = twist_move[twist][m];
      CoordL fslice1 = FSLICE(flip1, SS_SLICE(sslice1));
      
      CoordL fstwist1 = FSTWIST(
        fslice_sym[fslice1], conj_twist[twist1][fslice_sym_sym[fslice1]]
      );

      if (getPrun3(fstwist_prun3, fstwist1) == depth3 - 1) {
        flip = flip1;
        sslice = sslice1;
        twist = twist1;
        fstwist = fstwist1;
        break;
      }
    }

    depth3--;
    depth++;
  }

  return depth;
}

int getCORNUDDist(Coord corners, Coord udedges) {
  CoordL cornud = CORNUD(
    corners_sym[corners], conj_udedges[udedges][corners_sym_sym[corners]]
  );

  int depth3 = getPrun3(cornud_prun3, cornud);
  if (depth3 == EMPTY)
    return MAX_DEPTH_P2 + 1;

  int depth = 0;
  while (cornud != 0) {
    if (depth3 == 0)
      depth3 = 3;

    for (int m = 0; m < N_MOVES2; m++) {
      Coord corners1 = corners_move[corners][kPhase2Moves[m]];
      Coord udedges1 = udedges_move[udedges][m];
      CoordL cornud1 = CORNUD(
        corners_sym[corners1], conj_udedges[udedges1][corners_sym_sym[corners1]]
      );

      if (getPrun3(cornud_prun3, cornud1) == depth3 - 1) {
        corners = corners1;
        udedges = udedges1;
        cornud = cornud1;
        break;
      }
    }

    depth3--;
    depth++;
  }

  return depth;
}

void initFSTwistPrun3() {
  fstwist_prun3 = new uint64_t[N_FSTWIST / 32 + 1];
  std::fill(fstwist_prun3, fstwist_prun3 + N_FSTWIST / 32 + 1, EMPTY_CELL);

  int filled = 1;
  int depth = 0;
  std::bitset<N_FSTWIST> *bits = new std::bitset<N_FSTWIST>();
  bool backsearch = false;

  setPrun3(fstwist_prun3, 0, 0);
  while (filled < N_FSTWIST) {
    CoordL c = 0;
    int depth3 = depth % 3;
    
    for (SymCoord fssym = 0; fssym < N_FSLICE_SYM; fssym++) {
      Coord flip = FS_FLIP(fslice_raw[fssym]);
      Coord slice = FS_SLICE(fslice_raw[fssym]);

      for (Coord twist = 0; twist < N_TWIST; twist++, c++) {
        if (!backsearch) {
          if (
            c % 32 == 0 && 
            fstwist_prun3[c / 32] == EMPTY_CELL &&
            twist < N_TWIST - 32
          ) {
            twist += 31;
            c += 31;
            continue;
          }
          if (bits->test(c) || getPrun3(fstwist_prun3, c) != depth3)
            continue;
          bits->set(c);
        } else if (getPrun3(fstwist_prun3, c) != EMPTY)
          continue;

        for (int m = 0; m < N_MOVES; m++) {
          Coord flip1 = flip_move[flip][m];
          Coord slice1 = sliceMove(slice, m);
          CoordL fslice1 = FSLICE(flip1, slice1);
          Coord twist1 = twist_move[twist][m];

          SymCoord fssym1 = fslice_sym[fslice1];
          twist1 = conj_twist[twist1][fslice_sym_sym[fslice1]];
          CoordL c1 = FSTWIST(fssym1, twist1);

          if (backsearch) {
            if (getPrun3(fstwist_prun3, c1) != depth3)
              continue;
            setPrun3(fstwist_prun3, c, depth + 1);
            filled++;
            break;
          } else if (getPrun3(fstwist_prun3, c1) != EMPTY)
            continue;
     
          setPrun3(fstwist_prun3, c1, depth + 1);
          filled++;
            
          SymSet symset = fslice_symset[fssym1] >> 1;
          for (Sym s = 1; symset > 0; symset >>= 1, s++) {
            if (symset & 1) {
              CoordL c2 = FSTWIST(fssym1, conj_twist[twist1][s]);
              if (getPrun3(fstwist_prun3, c2) == EMPTY) {
                setPrun3(fstwist_prun3, c2, depth + 1);
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

void initCornUDPrun3() {
  cornud_prun3 = new uint64_t[N_CORNUD / 32 + 1];
  std::fill(cornud_prun3, cornud_prun3 + N_CORNUD / 32 + 1, EMPTY_CELL);

  int filled = 1;
  int depth = 0;
  std::bitset<N_CORNUD> *bits = new std::bitset<N_CORNUD>();

  setPrun3(cornud_prun3, 0, 0);
  while (depth < MAX_DEPTH_P2) {
    CoordL c = 0;
    int depth3 = depth % 3;

    for (SymCoord csym = 0; csym < N_CORNERS_SYM; csym++) {
      for (Coord udedges = 0; udedges < N_UDEDGES2; udedges++, c++) {
        if (
          c % 32 == 0 && 
          cornud_prun3[c / 32] == EMPTY_CELL &&
          udedges < N_UDEDGES2 - 32
        ) {
          udedges += 31;
          c += 31;
          continue;
        }
        if (bits->test(c) || getPrun3(cornud_prun3, c) != depth3)
          continue;
        bits->set(c);

        for (int m = 0; m < N_MOVES2; m++) {
          Coord corners1 = corners_move[corners_raw[csym]][kPhase2Moves[m]];
          Coord udedges1 = udedges_move[udedges][m];
          SymCoord csym1 = corners_sym[corners1];
          udedges1 = conj_udedges[udedges1][corners_sym_sym[corners1]];
          CoordL c1 = CORNUD(csym1, udedges1);

          if (getPrun3(cornud_prun3, c1) != EMPTY)
            continue;
    
          setPrun3(cornud_prun3, c1, depth + 1);
          filled++;
            
          SymSet symset = corners_symset[csym1] >> 1;
          for (Sym s = 1; symset > 0; symset >>= 1, s++) {
            if (symset & 1) {
              CoordL c2 = CORNUD(csym1, conj_udedges[udedges1][s]);
              if (getPrun3(cornud_prun3, c2) == EMPTY) {
                setPrun3(cornud_prun3, c2, depth + 1);
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

void initCornSlicePrun() {
  cornslice_prun = new uint8_t[N_CORNSLICE];
  std::fill(cornslice_prun, cornslice_prun + N_CORNSLICE, 0xff);

  std::queue<CoordL> q;
  cornslice_prun[0] = 0;
  q.push(0);
  
  while (q.size() > 0) {
    CoordL c = q.front();
    q.pop();
    Coord corners = CS_CORNERS(c);
    Coord sslice = CS_SSLICE(c);

    for (int m : kPhase2Moves) {
      CoordL c1 = CORNSLICE(
        corners_move[corners][m], sslice_move[sslice][m]
      );
      if (cornslice_prun[c1] == 0xff) {
        cornslice_prun[c1] = cornslice_prun[c] + 1;
        q.push(c1);
      }
    }
  }
}
