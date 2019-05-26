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
uint64_t *fsstwist_prun3;

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

int getPrun3(uint64_t *prun3, CoordL c) {
  uint64_t tmp = prun3[c / 32];
  tmp >>= (c % 32) * 2;
  return tmp & EMPTY;
}

void setPrun3(uint64_t *prun3, CoordL c, int dist) {
  int shift = (c % 32) * 2;
  prun3[c / 32] &= ~(uint64_t(EMPTY) << shift) | (uint64_t(dist % 3) << shift);
}

int getFSTwistDist(Coord flip, Coord sslice, Coord twist) {
  CoordL fslice = FSLICE(flip, SS_SLICE(sslice));
  CoordL fstwist = FSTWIST(
    COORD(fslice_sym[fslice]), conj_twist[twist][SYM(fslice_sym[fslice])]
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
        COORD(fslice_sym[fslice1]), conj_twist[twist1][SYM(fslice_sym[fslice1])]
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

int getCornUDDist(Coord corners, Coord udedges) {
  CoordL cornud = CORNUD(
    COORD(corners_sym[corners]), conj_udedges[udedges][SYM(corners_sym[corners])]
  );

  int depth3 = getPrun3(cornud_prun3, cornud);
  if (depth3 == EMPTY) // we do not fully fill the pruning table
    return MAX_DEPTH_P2 + 1; // lower bound for actual pruning value

  int depth = 0;
  while (cornud != 0) {
    if (depth3 == 0)
      depth3 = 3;

    for (int m = 0; m < N_MOVES2; m++) {
      Coord corners1 = corners_move[corners][kPhase2Moves[m]];
      Coord udedges1 = udedges_move2[udedges][m];
      CoordL cornud1 = CORNUD(
        COORD(corners_sym[corners1]), conj_udedges[udedges1][SYM(corners_sym[corners1])]
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

int getFSSTwistDist(Coord flip, Coord sslice, Coord twist) {
  CoordL fsstwist = FSSTWIST(
    conj_flip[flip][SYM(sslice_sym[sslice])][COORD(sslice_sym[sslice])],
    COORD(sslice_sym[sslice]),
    conj_twist[twist][SYM(sslice_sym[sslice])]
  );

  int depth3 = getPrun3(fsstwist_prun3, fsstwist);
  int depth = 0;

  while (fsstwist != 0) {
    if (depth3 == 0)
      depth3 = 3;

    for (int m = 0; m < N_MOVES; m++) {
      Coord flip1 = flip_move[flip][m];
      Coord sslice1 = sslice_move[sslice][m];
      Coord twist1 = twist_move[twist][m];

      CoordL fsstwist1 = FSSTWIST(
        conj_flip[flip1][SYM(sslice_sym[sslice1])][COORD(sslice_sym[sslice1])],
        COORD(sslice_sym[sslice1]),
        conj_twist[twist1][SYM(sslice_sym[sslice1])]
      );

      if (getPrun3(fsstwist_prun3, fsstwist1) == depth3 - 1) {
        flip = flip1;
        sslice = sslice1;
        twist = twist1;
        fsstwist = fsstwist1;
        break;
      }
    }

    depth3--;
    depth++;
  }

  return depth;
}

void initFSTwistPrun3() {
  fstwist_prun3 = new uint64_t[N_FSTWIST / 32 + 1]; // add + 1 to be safe
  std::fill(fstwist_prun3, fstwist_prun3 + N_FSTWIST / 32 + 1, EMPTY_CELL);

  int count = 1;
  int depth = 0;
  auto *done = new std::bitset<N_FSTWIST>(); // need to keep track of already expanded notes (table only stores mod 3)
  bool backsearch = false;

  setPrun3(fstwist_prun3, 0, 0);
  while (count < N_FSTWIST) {
    CoordL c = 0; // increment this in the inner loop to avoid always recomputing the inde
    int depth3 = depth % 3;
    
    for (Coord fssym = 0; fssym < N_FSLICE_SYM; fssym++) {
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
          if (done->test(c) || getPrun3(fstwist_prun3, c) != depth3)
            continue;
          done->set(c);
        } else if (getPrun3(fstwist_prun3, c) != EMPTY)
          continue;

        for (int m = 0; m < N_MOVES; m++) {
          Coord flip1 = flip_move[flip][m];
          Coord slice1 = sliceMove(slice, m);
          CoordL fslice1 = FSLICE(flip1, slice1);
          Coord twist1 = twist_move[twist][m];

          Coord fssym1 = COORD(fslice_sym[fslice1]);
          twist1 = conj_twist[twist1][SYM(fslice_sym[fslice1])];
          CoordL c1 = FSTWIST(fssym1, twist1);

          if (backsearch) {
            if (getPrun3(fstwist_prun3, c1) != depth3)
              continue;
            setPrun3(fstwist_prun3, c, depth + 1);
            count++;
            break; // self-symmetries are not applicable during backsearch
          } else if (getPrun3(fstwist_prun3, c1) != EMPTY)
            continue;
     
          setPrun3(fstwist_prun3, c1, depth + 1);
          count++;
            
          int selfs = fslice_selfs[fssym1] >> 1;
          for (int s = 1; selfs > 0; selfs >>= 1, s++) { // bit 0 is always on -> > 0 to save an iteration
            if (selfs & 1) {
              CoordL c2 = FSTWIST(fssym1, conj_twist[twist1][s]);
              if (getPrun3(fstwist_prun3, c2) == EMPTY) {
                setPrun3(fstwist_prun3, c2, depth + 1);
                done->set(c2); // expanding self-symmetries is redundant
                count++;
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

  delete done;
}

void initCornUDPrun3() {
  cornud_prun3 = new uint64_t[N_CORNUD / 32 + 1];
  std::fill(cornud_prun3, cornud_prun3 + N_CORNUD / 32 + 1, EMPTY_CELL);

  int count = 1;
  int depth = 0;
  auto *done = new std::bitset<N_CORNUD>();

  setPrun3(cornud_prun3, 0, 0);
  while (depth < MAX_DEPTH_P2) {
    CoordL c = 0;
    int depth3 = depth % 3;

    for (Coord csym = 0; csym < N_CORNERS_SYM; csym++) {
      for (Coord udedges = 0; udedges < N_UDEDGES2; udedges++, c++) {
        if (c % 32 == 0 && cornud_prun3[c / 32] == EMPTY_CELL) {
          int tmp = std::min(31, N_UDEDGES2 - udedges - 1);
          udedges += tmp;
          c += tmp;
          continue;
        }
        if (done->test(c) || getPrun3(cornud_prun3, c) != depth3)
          continue;
        done->set(c);

        for (int m = 0; m < N_MOVES2; m++) {
          Coord corners1 = corners_move[corners_raw[csym]][kPhase2Moves[m]];
          Coord udedges1 = udedges_move2[udedges][m];
          Coord csym1 = COORD(corners_sym[corners1]);
          udedges1 = conj_udedges[udedges1][SYM(corners_sym[corners1])];
          CoordL c1 = CORNUD(csym1, udedges1);

          if (getPrun3(cornud_prun3, c1) != EMPTY)
            continue;
    
          setPrun3(cornud_prun3, c1, depth + 1);
          count++;
            
          int selfs = corners_selfs[csym1] >> 1;
          for (int s = 1; selfs > 0; selfs >>= 1, s++) {
            if (selfs & 1) {
              CoordL c2 = CORNUD(csym1, conj_udedges[udedges1][s]);
              if (getPrun3(cornud_prun3, c2) == EMPTY) {
                setPrun3(cornud_prun3, c2, depth + 1);
                done->set(c2);
                count++;
              }
            }
          }
        }
      }
    }

    depth++;
  }

  delete done;
}

// This table is rather small, hence we can populate it with standard BFS
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

void initFSSTwistPrun3() {
  fsstwist_prun3 = new uint64_t[N_FSSTWIST / 32 + 1];
  std::fill(fsstwist_prun3, fsstwist_prun3 + N_FSSTWIST / 32 + 1, EMPTY_CELL);

  uint32_t count = 1;
  int depth = 0;
  auto *done = new std::bitset<N_FSSTWIST>();
  bool backsearch = false;

  setPrun3(fsstwist_prun3, 0, 0);
  while (count < N_FSSTWIST) {
    CoordL c = 0;
    int depth3 = depth % 3;

    for (Coord sssym = 0; sssym < N_SSLICE_SYM; sssym++) {
      Coord sslice = sslice_raw[sssym];

      for (Coord flip = 0; flip < N_FLIP; flip++) {
        for (Coord twist = 0; twist < N_TWIST; twist++, c++) {
          if (!backsearch) {
            if (c % 32 == 0 && fsstwist_prun3[c / 32] == EMPTY_CELL) {
              int tmp = std::min(31, N_TWIST - twist - 1);
              twist += tmp;
              c += tmp;
              continue;
            }
            if (done->test(c) || getPrun3(fsstwist_prun3, c) != depth3)
              continue;
            done->set(c);
          } else if (getPrun3(fsstwist_prun3, c) != EMPTY)
            continue;

          for (int m = 0; m < N_MOVES; m++) {
            Coord flip1 = flip_move[flip][m];
            Coord sslice1 = sslice_move[sslice][m];
            Coord twist1 = twist_move[twist][m];

            Coord sssym1 = COORD(sslice_sym[sslice1]);
            flip1 = conj_flip[flip1][SYM(sslice_sym[sslice1])][sssym1];
            twist1 = conj_twist[twist1][SYM(sslice_sym[sslice1])];
            CoordL c1 = FSSTWIST(flip1, sssym1, twist1);

            if (backsearch) {
              if (getPrun3(fsstwist_prun3, c1) != depth3)
                continue;
              setPrun3(fsstwist_prun3, c, depth + 1);
              count++;
              break;
            } else if (getPrun3(fsstwist_prun3, c1) != EMPTY)
              continue;

            setPrun3(fsstwist_prun3, c1, depth + 1);
            count++;

            int selfs = sslice_selfs[sssym1] >> 1;
            for (int s = 1; selfs > 0; selfs >>= 1, s++) {
              if (selfs & 1) {
                CoordLL c2 = FSSTWIST(conj_flip[flip1][s][sssym1], sssym1, conj_twist[twist1][s]);
                if (getPrun3(fsstwist_prun3, c2) == EMPTY) {
                  setPrun3(fsstwist_prun3, c2, depth + 1);
                  done->set(c2);
                  count++;
                }
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

  delete done;
}
