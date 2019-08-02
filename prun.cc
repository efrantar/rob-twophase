#include "prun.h"

#include <algorithm>
#include <bitset>
#include <queue>

#define EMPTY8 0xff

Prun *fstwist_prun;
uint8_t *corned_prun;
uint8_t *cornslice_prun;

uint8_t mm_key[N_SYMS_SUB][N_AXES];
MMChunk mm_map[2][N_INFO][1 << (N_PER_AXIS + 1)];

int rev(int bitmask, int len, int count = 1) {
  int rev = 0;
  for (int i = 0; i < len; i += count) {
    rev = (rev << count) | (bitmask & ((1 << count) - 1));
    bitmask >>= count;
  }
  return rev;
}

void initPrun() {
  for (int s = 0; s < N_SYMS_SUB; s++) {
    for (int ax = 0; ax < N_AXES; ax++) {
      mm_key[s][ax] = N_INFO * (conj_move[N_PER_AXIS * ax][inv_sym[s]] / N_PER_AXIS);
      mm_key[s][ax] |= conj_move[N_PER_AXIS * ax][inv_sym[s]] % 3 != 0;
      #ifdef AXIAL
        mm_key[s][ax] |= (conj_move[N_PER_AXIS * ax][inv_sym[s]] % N_PER_AXIS >= 12) << 1;
      #endif
    }
  }

  for (int s = 0; s < N_SYMS_SUB; s++) {
    for (int info = 0; info < N_INFO; info++) {
      for (int chunk = 0; chunk < (1 << (N_PER_AXIS + 1)); chunk++) {
        int mm = chunk;
        #ifndef QUARTER
          mm >>= 1;
        #endif

        #if AXIAL
          #ifdef QUARTER
            if (info & 1) {
              mm =
                rev(mm, 4, 2) |
                (rev(mm >> 4, 8, 2) << 4) |
                (rev(mm >> 12, 4, 2) << 12)
              ;
            }
            if (info & 2) {
              int mm1 = (mm & 0xf) << 8;
              for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++)
                  mm1 |= ((mm >> 2 * (2 * i + j + 2)) & 0x3) << 2 * (2 * j + i);
              }
              mm = (mm1 << 4) | ((mm >> 12) & 0xf);
            }
          #else
            if (info & 1) {
              mm =
                rev(mm, 3) |
                (rev(mm >> 3, 9) << 3) |
                (rev(mm >> 12, 3) << 12)
              ;
            }
            if (info & 2) {
              int mm1 = (mm & 0x7) << 9;
              for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++)
                  mm1 |= ((mm & (1 << 3 * i + j + 3)) != 0) << 3 * j + i;
              }
              mm = (mm1 << 3) | ((mm >> 12) & 0x7);
            }
          #endif
        #else
          if (info & 1)
            mm = rev(mm, N_PER_AXIS, N_PER_MOVE);
        #endif

      #ifdef QUARTER
        mm_map[0][info][chunk] = 0;
        mm_map[1][info][chunk] = 0;
        for (int i = N_PER_AXIS - 1; i >= 0; i--) {
          mm_map[0][info][chunk] <<= 1;
          mm_map[1][info][chunk] <<= 1;
          if ((all_movemask & MOVEBIT(i)) != 0) {
            mm_map[0][info][chunk] |= (mm & (0x3 << (N_PER_AXIS - 1))) == 0;
            mm_map[1][info][chunk] |= (mm & (0x3 << (N_PER_AXIS - 1))) <= (1 << (N_PER_AXIS - 1));
            mm <<= 2;
          }
        }
      #else
        mm_map[0][info][chunk] = (chunk & 1) ? 0 : ~mm & AXIS_BITMASK;
        mm_map[1][info][chunk] = (chunk & 1) ? ~mm & AXIS_BITMASK : AXIS_BITMASK;
      #endif
      }
    }
  }
}

int getFSTwistPrun(int flip, Edges4 sslice, int twist, int togo, MoveMask &mm) {
  int fslice = FSLICE(flip, sslice.comb);
  int s = SYM(fslice_sym[fslice]);
  int fstwist = FSTWIST(
    COORD(fslice_sym[fslice]), conj_twist[twist][s]
  );

  Prun prun = fstwist_prun[fstwist];
  int dist = DIST(prun);
  prun >>= 8;

  int delta = togo - dist;
  if (delta < 0)
    mm = 0;
  else if (delta > 1)
    mm = all_movemask;
  else {
    mm = 0;
    for (int ax = 0; ax < N_AXES; ax++) {
      mm |= MoveMask(mm_map[delta][INFO(mm_key[s][ax])][prun & ((1 << (N_PER_AXIS + 1)) - 1)]) << OFF(mm_key[s][ax]);
      prun >>= N_PER_AXIS + 1;
    }
  }

  return dist;
}

int getCornEdPrun(CPerm cperm, Edges4 uedges, Edges4 dedges) {
  int tmp = cperm_sym[cperm.val()];
  int corned = CORNED(
    COORD(tmp), conj_udedges[mergeUDEdges2(uedges, dedges)][SYM(tmp)]
  );
  return corned_prun[corned];
}

int getCornSlicePrun(CPerm cperm, Edges4 sslice) {
  return cornslice_prun[CORNSLICE(cperm.val(), sslice.perm)];
}

void initFSTwistPrun() {
  fstwist_prun = new Prun[N_FSTWIST];
  std::fill(fstwist_prun, fstwist_prun + N_FSTWIST, EMPTY8);

  Edges4 sslice1;
  fstwist_prun[0] = 0;

  for (int dist = 0, count = 0; count < N_FSTWIST; dist++) {
    int c = 0;

    for (int fssym = 0; fssym < N_FSLICE_SYM; fssym++) {
      int flip = FS_FLIP(fslice_raw[fssym]);
      Edges4 sslice(FS_SLICE(fslice_raw[fssym]), 0);

      for (int twist = 0; twist < N_TWIST; twist++, c++) {
        if (DIST(fstwist_prun[c]) != dist)
          continue;
        count++;

        int deltas[N_MOVES];
        for (int m = 0; m < N_MOVES; m++) {
          if ((all_movemask & MOVEBIT(m)) == 0) {
            deltas[m] = 0;
            continue;
          }

          moveSSlice(sslice, m, sslice1);
          int fslice1 = FSLICE(move_flip[flip][m], sslice1.comb);
          int fssym1 = COORD(fslice_sym[fslice1]);
          int twist1 = conj_twist[move_twist[twist][m]][SYM(fslice_sym[fslice1])];
          int c1 = FSTWIST(fssym1, twist1);

          if (fstwist_prun[c1] == EMPTY8)
            fstwist_prun[c1] = dist + 1;
          deltas[m] = DIST(fstwist_prun[c1]) - dist;

          int selfs = fslice_selfs[fssym1] >> 1;
          for (int s = 1; selfs > 0; selfs >>= 1, s++) { // bit 0 is always on -> > 0 to save an iteration
            if (selfs & 1) {
              int c2 = FSTWIST(fssym1, conj_twist[twist1][s]);
              if (fstwist_prun[c2] == EMPTY8)
                fstwist_prun[c2] = dist + 1;
            }
          }
        }

        Prun prun = 0;
        #ifdef QUARTER
          for (int m = N_MOVES - 1; m >= 0; m--) {
            if ((all_movemask & MOVEBIT(m)) != 0)
              prun = (prun << 2) | (deltas[m] + 1);
          }
        #else
          for (int ax = N_AXES - 1; ax >= 0; ax--) {
            bool away = false;
            for (int i = N_PER_AXIS * ax; i < N_PER_AXIS * (ax + 1); i++) {
              if (deltas[i] != 0) {
                if (deltas[i] > 0)
                  away = true;
                break;
              }
            }

            int tmp = 0;
            for (int i = N_PER_AXIS * (ax + 1) - 1; i >= N_PER_AXIS * ax; i--)
              tmp = (tmp | (away ? deltas[i] : deltas[i] + 1)) << 1;
            tmp |= away;

            prun = (prun << (N_PER_AXIS + 1)) | tmp;
          }
        #endif
        fstwist_prun[c] |= prun << 8;
      }
    }

    std::cout << dist << " " << count << "\n";
  }
}

void initCornEdPrun() {
  corned_prun = new uint8_t[N_CORNED];
  std::fill(corned_prun, corned_prun + N_CORNED, EMPTY8);

  Edges4 uedges;
  Edges4 dedges;
  Edges4 uedges1;
  Edges4 dedges1;
  CPerm cperm1;

  corned_prun[0] = 0;
  for (int dist = 0, count = 0; count < N_CORNED; dist++) {
    int c = 0;
    for (int i = 0; i < N_CPERM_SYM; i++) {
      CPerm cperm(cperm_raw[i]);

      for (int j = 0; j < N_UDEDGES2; j++, c++) {
        if (corned_prun[c] != dist)
          continue;
        count++;

        splitUDEdges2(j, uedges, dedges);

        for (int m = 0; m < N_MOVES2; m++) {
          int dist1 = dist + 1;
          #ifdef QUARTER
            if ((extra_movemask & MOVEBIT(moves2[m])) != 0)
              dist1++;
            else if ((all_movemask & MOVEBIT(moves2[m])) == 0)
              continue;
          #else
            if ((all_movemask & MOVEBIT(moves2[m])) == 0)
              continue;
          #endif

          moveCPerm(cperm, moves2[m], cperm1);
          moveEdges4(uedges, moves2[m], uedges1);
          moveEdges4(dedges, moves2[m], dedges1);
          int tmp = cperm_sym[cperm1.val()];
          int udedges1 = conj_udedges[mergeUDEdges2(uedges1, dedges1)][SYM(tmp)];
          int csym1 = COORD(tmp);
          int c1 = CORNED(csym1, udedges1);

          if (corned_prun[c1] <= dist1)
            continue;
          corned_prun[c1] = dist1;

          int selfs = cperm_selfs[csym1] >> 1;
          for (int s = 1; selfs > 0; selfs >>= 1, s++) {
            if (selfs & 1) {
              int c2 = CORNED(csym1, conj_udedges[udedges1][s]);
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

  Edges4 sslice1;
  CPerm cperm1;

  cornslice_prun[0] = 0;
  for (int dist = 0, count = 0; count < N_CORNSLICE; dist++) {
    int c = 0;

    for (int i = 0; i < N_CPERM; i++) {
      CPerm cperm(i);

      for (int j = 0; j < N_SSLICE2; j++, c++) {
        if (cornslice_prun[c] != dist)
          continue;
        count++;

        Edges4 sslice(j);

        for (int m = 0; m < N_MOVES2; m++) {
          int dist1 = dist + 1;
          #ifdef QUARTER
            if ((extra_movemask & MOVEBIT(moves2[m])) != 0)
                dist1++;
            else if ((all_movemask & MOVEBIT(moves2[m])) == 0)
                continue;
          #else
            if ((all_movemask & MOVEBIT(moves2[m])) == 0)
              continue;
          #endif

          moveCPerm(cperm, moves2[m], cperm1);
          moveSSlice(sslice, moves2[m], sslice1);

          int c1 = CORNSLICE(cperm1.val(), sslice1.perm);
          if (cornslice_prun[c1] > dist1)
            cornslice_prun[c1] = dist1;
        }
      }
    }
    std::cout << dist << " " << count << "\n";
  }
}
