#include "prun.h"

#include <algorithm>
#include <bitset>
#include <queue>
#include <string.h>

#ifdef QUARTER
  #define PER_AXIS 4
#else
  #define PER_AXIS 6
#endif

#define EMPTY 0xff

uint8_t mm_key[N_SYMS_SUB][3];
uint8_t mm_map1[2][4][256];
#ifdef AXIAL
  #ifdef QUARTER
    uint8_t mm_map2[2][4][256];
  #else
    uint16_t mm_map2[2][4][1024];
  #endif
#endif

Prun *fstwist_prun;
uint8_t *corned_prun;
uint8_t *cornslice_prun;

int reverse(int bitmask, int len, int step = 1) {
  int rev = 0;
  for (int i = 0; i < len; i += step) {
    rev = (rev << step) | (bitmask & ((1 << step) - 1));
    bitmask >>= step;
  }
  return rev;
}

void initPrun() {
  for (int s = 0; s < N_SYMS_SUB; s++) {
    for (int ax = 0; ax < 3; ax++) {
      mm_key[s][ax] = (conj_move[PER_AXIS * ax][inv_sym[s]] / PER_AXIS) << 2; // offset
      mm_key[s][ax] |= (conj_move[PER_AXIS * ax][inv_sym[s]] % (PER_AXIS / 2) != 0) << 1; // invert
      mm_key[s][ax] |= conj_move[PER_AXIS * ax][inv_sym[s]] % PER_AXIS >= (PER_AXIS / 2); // flip
    }
  }

  for (int info = 0; info < 4; info++) {
    for (int chunk = 0; chunk < 256; chunk++) {
      int chunk1 = chunk & 0xf;
      int chunk2 = chunk >> 4;
      if (info & 1)
        std::swap(chunk1, chunk2);

      #ifdef QUARTER
        if (info & 2) {
          chunk1 = reverse(chunk1, 4, 2);
          chunk2 = reverse(chunk2, 4, 2);
        }
        int chunk3 = (chunk2 << 4) | chunk1;

        mm_map1[0][info][chunk] = 0;
        mm_map1[1][info][chunk] = 0;
        for (int i = 0; i < 4; i++) {
          mm_map1[0][info][chunk] |= ((chunk3 & 0x3) == 0) << i;
          mm_map1[1][info][chunk] |= ((chunk3 & 0x3) <= 1) << i;
          chunk3 >>= 2;
        }
      #else
        int mm1 = chunk1 >> 1;
        int mm2 = chunk2 >> 1;
        if (info & 2) {
          mm1 = reverse(mm1, 3);
          mm2 = reverse(mm2, 3);
        }

        mm_map1[0][info][chunk] = (chunk1 & 1) ? 0 : ~mm1 & 0x7;
        mm_map1[0][info][chunk] |= (chunk2 & 1) ? 0 : (~mm2 & 0x7) << 3;
        mm_map1[1][info][chunk] = (chunk1 & 1) ? ~mm1 & 0x7 : 0x7;
        mm_map1[1][info][chunk] |= ((chunk2 & 1) ? (~mm2 & 0x7) << 3 : 0x38);
      #endif
    }
  }

  #ifdef AXIAL
    #ifdef QUARTER
      for (int info = 0; info < 4; info++) {
        for (int chunk = 0; chunk < 256; chunk++) {
          int chunk1 = chunk;

          if (info & 1) {
            int chunk2 = 0;
            for (int i = 0; i < 2; i++) {
              for (int j = 0; j < 2; j++)
                chunk2 |= ((chunk1 >> 2 * (2 * i + j)) & 0x3) << 2 * (2 * j + i);
            }
            chunk1 = chunk2;
          }
          if (info & 2)
            chunk1 = reverse(chunk1, 8, 2);

          mm_map2[0][info][chunk] = 0;
          mm_map2[1][info][chunk] = 0;
          for (int i = 0; i < 4; i++) {
            mm_map2[0][info][chunk] |= ((chunk1 & 0x3) == 0) << i;
            mm_map2[1][info][chunk] |= ((chunk1 & 0x3) <= 1) << i;
            chunk1 >>= 2;
          }
        }
      }
    #else
      for (int info = 0; info < 4; info++) {
        for (int chunk = 0; chunk < 1024; chunk++) {
          int mm = chunk >> 1;

          if (info & 1) {
            int mm1 = 0;
            for (int i = 0; i < 3; i++) {
              for (int j = 0; j < 3; j++)
                mm1 |= ((mm >> 3 * i + j) & 1) << 3 * j + i;
            }
            mm = mm1;
          }
          if (info & 2)
            mm = reverse(mm, 9);

          mm_map2[0][info][chunk] = (chunk & 1) ? 0 : ~mm & 0x1ff;
          mm_map2[1][info][chunk] = (chunk & 1) ? ~mm & 0x1ff : 0x1ff;
        }
      }
    #endif
  #endif
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
    mm = phase1_moves;
  else {
    mm = 0;
    #ifdef AXIAL
      Prun prun1 = prun >> 24;
    #endif
    for (int ax = 0; ax < 3; ax++) {
      int tmp = mm_key[s][ax];
      mm |= MoveMask(mm_map1[delta][INFO(tmp)][prun & 0xff]) << PER_AXIS * OFF(tmp);
      prun >>= 8;
      #ifdef AXIAL
        #ifdef QUARTER
          mm |= MoveMask(mm_map2[delta][INFO(tmp)][prun1 & 0xff]) << 12 + 4 * OFF(tmp);
          prun1 >>= 8;
        #else
        mm |= MoveMask(mm_map2[delta][INFO(tmp)][prun1 & 0x3ff]) << 18 + 9 * OFF(tmp);
          prun1 >>= 10;
        #endif
      #endif
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
  std::fill(fstwist_prun, fstwist_prun + N_FSTWIST, EMPTY);

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
        for (int m = 0; m < N_MOVES1; m++) {
          #ifdef FACES5
            if ((phase1_moves & MOVEBIT(m)) == 0) {
              deltas[m] = 0;
              continue;
            }
          #endif

          moveSSlice(sslice, m, sslice1);
          int fslice1 = FSLICE(move_flip[flip][m], sslice1.comb);
          int fssym1 = COORD(fslice_sym[fslice1]);
          int twist1 = conj_twist[move_twist[twist][m]][SYM(fslice_sym[fslice1])];
          int c1 = FSTWIST(fssym1, twist1);

          if (fstwist_prun[c1] == EMPTY)
            fstwist_prun[c1] = dist + 1;
          deltas[m] = DIST(fstwist_prun[c1]) - dist;

          int selfs = fslice_selfs[fssym1] >> 1;
          for (int s = 1; selfs > 0; selfs >>= 1, s++) { // bit 0 is always on -> > 0 to save an iteration
            if (selfs & 1) {
              int c2 = FSTWIST(fssym1, conj_twist[twist1][s]);
              if (fstwist_prun[c2] == EMPTY)
                fstwist_prun[c2] = dist + 1;
            }
          }
        }

        Prun prun = 0;
        #ifdef QUARTER
          for (int m = N_MOVES1 - 1; m >= 0; m--)
              prun = (prun << 2) | (deltas[m] + 1);
        #else
          #ifdef AXIAL
            for (int ax = 3; ax >= 0; ax--) {
              bool away = false;
              for (int i = 18 + 9 * ax; i < 18 + 9 * (ax + 1); i++) {
                if (deltas[i] != 0) {
                  if (deltas[i] > 0)
                    away = true;
                  break;
                }
              }

              int tmp = 0;
              for (int i = 18 + 9 * (ax + 1) - 1; i >= 18 + 9 * ax; i--)
                tmp = (tmp | (away ? deltas[i] : deltas[i] + 1)) << 1;
              tmp |= away;

              prun = (prun << 10) | tmp;
            }
          #endif
          for (int ax = 5; ax >= 0; ax--) {
            bool away = false;
            for (int i = 3 * ax; i < 3 * (ax + 1); i++) {
              if (deltas[i] != 0) {
                if (deltas[i] > 0)
                  away = true;
                break;
              }
            }

            int tmp = 0;
            for (int i = 3 * (ax + 1) - 1; i >= 3 * ax; i--)
              tmp = (tmp | (away ? deltas[i] : deltas[i] + 1)) << 1;
            tmp |= away;

            prun = (prun << 4) | tmp;
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
  std::fill(corned_prun, corned_prun + N_CORNED, EMPTY);

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

        for (int m = 0; m < N_MOVES; m++) {
          if ((phase2_moves & MOVEBIT(m)) == 0)
            continue;

          int dist1 = dist + 1;
          #ifdef QUARTER
            if (m >= N_MOVES - N_DOUBLE2)
              dist1++;
          #endif

          moveCPerm(cperm, m, cperm1);
          moveEdges4(uedges, m, uedges1);
          moveEdges4(dedges, m, dedges1);
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
  std::fill(cornslice_prun, cornslice_prun + N_CORNSLICE, EMPTY);

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

        for (int m = 0; m < N_MOVES; m++) {
          if ((phase2_moves & MOVEBIT(m)) == 0)
            continue;

          int dist1 = dist + 1;
          #ifdef QUARTER
            if (m >= N_MOVES1)
              dist1++;
          #endif

          moveCPerm(cperm, m, cperm1);
          moveSSlice(sslice, m, sslice1);

          int c1 = CORNSLICE(cperm1.val(), sslice1.perm);
          if (cornslice_prun[c1] > dist1)
            cornslice_prun[c1] = dist1;
        }
      }
    }
    std::cout << dist << " " << count << "\n";
  }
}
