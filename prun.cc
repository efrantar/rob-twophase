#include "prun.h"

#include <algorithm>
#include <bitset>
#include <queue>

#define BACKSEARCH_PERCENT 0.2

#define EMPTY2 0x3
#define EMPTY8 0xff
#define EMPTY_CELL ~uint64_t(0)

int (*next_dist)[3];

uint64_t *fstwist_prun3;
uint8_t *corned_prun;
uint8_t *cornslice_prun;

static bool init() {
 next_dist = new int[22][3];
 for (int i = 0; i < 22; i++) {
   if (i > 0)
     next_dist[i][(i - 1) % 3] = i - 1;
   next_dist[i][i % 3] = i;
   next_dist[i][(i + 1) % 3] = i + 1;
 }
 return true;
}
static bool inited = init();

int getFSTwistPrun3(CCoord c) {
  uint64_t tmp = fstwist_prun3[c / 32];
  tmp >>= (c % 32) * 2;
  return tmp & EMPTY2;
}

void setFSTwistPrun3(CCoord c, int dist) {
  int shift = (c % 32) * 2;
  fstwist_prun3[c / 32] &= ~(uint64_t(EMPTY2) << shift) | (uint64_t(dist % 3) << shift);
}

int getCornEdPrun(CCoord c) {
  uint64_t tmp = corned_prun[c / 16];
  tmp >>= (c % 16) * 4;
  return tmp & EMPTY8;
}

void setCornEdPrun(CCoord c, int dist) {
  int shift = (c % 16) * 4;
  corned_prun[c / 16] &= ~(uint64_t(EMPTY8) << shift) | (uint64_t(dist) << shift);
}

int getFSTwistDist(Coord flip, Coord sslice, Coord twist) {
  CCoord fslice = FSLICE(flip, SS_SLICE(sslice));
  CCoord fstwist = FSTWIST(
    COORD(fslice_sym[fslice]), conj_twist[twist][SYM(fslice_sym[fslice])]
  );

  int depth3 = getFSTwistPrun3(fstwist);
  int depth = 0;

  while (fstwist != 0) {
    if (depth3 == 0)
      depth3 = 3;

    for (int m = 0; m < N_MOVES; m++) {
      #ifdef QTM
        if (qtm[m] != 0)
          continue;
      #endif

      Coord flip1 = flip_move[flip][m];
      Coord sslice1 = sslice_move[sslice][m];
      Coord twist1 = twist_move[twist][m];
      CCoord fslice1 = FSLICE(flip1, SS_SLICE(sslice1));
      
      CCoord fstwist1 = FSTWIST(
        COORD(fslice_sym[fslice1]), conj_twist[twist1][SYM(fslice_sym[fslice1])]
      );
      if (getFSTwistPrun3(fstwist1) == depth3 - 1) {
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

int getFSTwistPrun3(Coord flip, Coord sslice, Coord twist) {
  CCoord fslice = FSLICE(flip, SS_SLICE(sslice));
  CCoord fstwist = FSTWIST(
    COORD(fslice_sym[fslice]), conj_twist[twist][SYM(fslice_sym[fslice])]
  );
  return getFSTwistPrun3(fstwist);
}

int getCornEdPrun(Coord cperm, Coord udedges) {
  CCoord corned = CORNED(
    COORD(cperm_sym[cperm]), conj_udedges[udedges][SYM(cperm_sym[cperm])]
  );
  return corned_prun[corned];
}

int getCornSlicePrun(Coord cperm, Coord sslice) {
  return cornslice_prun[CORNSLICE(cperm, sslice)];
}

void initFSTwistPrun3() {
  fstwist_prun3 = new uint64_t[N_FSTWIST / 32 + 1]; // add + 1 to be safe
  std::fill(fstwist_prun3, fstwist_prun3 + N_FSTWIST / 32 + 1, EMPTY_CELL);

  int count = 1;
  int depth = 0;
  auto *done = new std::bitset<N_FSTWIST>(); // need to keep track of already expanded notes (table only stores mod 3)
  bool backsearch = false;

  setFSTwistPrun3(0, 0);
  while (count < N_FSTWIST) {
    CCoord c = 0; // increment this in the inner loop to avoid always recomputing the index
    int depth3 = depth % 3;

    // `fssym` needs to `CCoord` so that it also works in 5-face mode
    for (CCoord fssym = 0; fssym < N_FSLICE_SYM; fssym++) {
      Coord flip = FS_FLIP(fslice_raw[fssym]);
      Coord slice = FS_SLICE(fslice_raw[fssym]);

      for (Coord twist = 0; twist < N_TWIST; twist++, c++) {
        if (!backsearch) {
          // Quickly skip fully empty table cells in early iterations
          if (c % 32 == 0 && fstwist_prun3[c / 32] == EMPTY_CELL) {
            // Make sure `fssym` and `c` remain synced
            int tmp = std::min(31, N_TWIST - twist - 1);
            twist += tmp;
            c += tmp;
            continue;
          }
          if (done->test(c) || getFSTwistPrun3(c) != depth3)
            continue;
          done->set(c);
        } else if (getFSTwistPrun3(c) != EMPTY2)
          continue;

        for (int m = 0; m < N_MOVES; m++) {
          #ifdef QTM
            if (qtm[m] != 0)
              continue;
          #endif

          Coord flip1 = flip_move[flip][m];
          Coord slice1 = sliceMove(slice, m);
          CCoord fslice1 = FSLICE(flip1, slice1);
          Coord twist1 = twist_move[twist][m];

          CCoord fssym1 = COORD(fslice_sym[fslice1]);
          twist1 = conj_twist[twist1][SYM(fslice_sym[fslice1])];
          CCoord c1 = FSTWIST(fssym1, twist1);

          if (backsearch) {
            if (getFSTwistPrun3(c1) != depth3)
              continue;
            setFSTwistPrun3(c, depth + 1);
            count++;
            break; // self-symmetries are not applicable during backsearch
          } else if (getFSTwistPrun3(c1) != EMPTY2)
            continue;
     
          setFSTwistPrun3(c1, depth + 1);
          count++;
            
          int selfs = fslice_selfs[fssym1] >> 1;
          for (int s = 1; selfs > 0; selfs >>= 1, s++) { // bit 0 is always on -> > 0 to save an iteration
            if (selfs & 1) {
              CCoord c2 = FSTWIST(fssym1, conj_twist[twist1][s]);
              if (getFSTwistPrun3(c2) == EMPTY2) {
                setFSTwistPrun3(c2, depth + 1);
                done->set(c2); // expanding self-symmetries is redundant
                count++;
              }
            }
          }
        }
      }
    }

    depth++;
    if (count > N_FSTWIST * BACKSEARCH_PERCENT)
      backsearch = true;

    std::cout << depth << " " << count << "\n";
  }

  delete done;
}

void initCornEdPrun() {
  corned_prun = new uint8_t[N_CORNED];
  std::fill(corned_prun, corned_prun + N_CORNED, EMPTY8);

  corned_prun[0] = 0;
  for (int dist = 0, count = 0; count < N_CORNED; dist++) {
    CCoord c = 0;
    for (Coord csym = 0; csym < N_CPERM_SYM; csym++) {
      for (Coord udedges = 0; udedges < N_UDEDGES2; udedges++, c++) {
        if (corned_prun[c] != dist)
          continue;
        count++;

        for (int m = 0; m < N_MOVES2; m++) {
          #ifdef QTM
            if (qtm[moves2[m]] > 1)
                continue;
          #endif

          int dist1 = dist + 1;
          #ifdef QTM
            dist1 += qtm[moves2[m]];
          #endif
          Coord corners1 = cperm_move[cperm_raw[csym]][moves2[m]];
          Coord udedges1 = udedges_move2[udedges][m];
          Coord csym1 = COORD(cperm_sym[corners1]);
          udedges1 = conj_udedges[udedges1][SYM(cperm_sym[corners1])];
          CCoord c1 = CORNED(csym1, udedges1);

          if (corned_prun[c1] <= dist1)
            continue;
          corned_prun[c1] = dist1;

          int selfs = cperm_selfs[csym1] >> 1;
          for (int s = 1; selfs > 0; selfs >>= 1, s++) {
            if (selfs & 1) {
              CCoord c2 = CORNED(csym1, conj_udedges[udedges1][s]);
              if (corned_prun[c2] > dist1)
                corned_prun[c2] = dist1;
            }
          }
        }
      }
    }
    std::cout << dist << " " << count << "\n";
  }
}

void initCornSlicePrun() {
  cornslice_prun = new uint8_t[N_CORNSLICE];
  std::fill(cornslice_prun, cornslice_prun + N_CORNSLICE, EMPTY8);

  cornslice_prun[0] = 0;
  for (int dist = 0, count = 0; count < N_CORNSLICE; dist++) {
    CCoord c = 0;
    for (Coord cperm = 0; cperm < N_CPERM; cperm++) {
      for (Coord sslice = 0; sslice < N_SSLICE2; sslice++, c++) {
        if (cornslice_prun[c] != dist)
          continue;
        count++;

        for (int m = 0; m < N_MOVES2; m++) {
          #ifdef QTM
            if (qtm[moves2[m]] > 1)
              continue;
          #endif

          CCoord c1 = CORNSLICE(
            cperm_move[cperm][moves2[m]], sslice_move[sslice][moves2[m]]
          );
          int dist1 = dist + 1;
          #ifdef QTM
            dist1 += qtm[moves2[m]];
          #endif
          if (cornslice_prun[c1] > dist1)
            cornslice_prun[c1] = dist1;
        }
      }
    }
  }
}
