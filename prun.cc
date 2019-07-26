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

// TODO: AXIAL + QUARTER TURN mode!
void initPrun() {
  for (int s = 0; s < N_SYMS_SUB; s++) {
    for (int ax = 0; ax < N_AXES; ax++) {
    mm_key[s][ax] = (conj_move[N_PER_AXIS * ax][inv_sym[s]] / N_PER_AXIS) << 1;
    mm_key[s][ax] |= conj_move[N_PER_AXIS * ax][inv_sym[s]] % N_PER_AXIS != 0;
    }
  }

  for (int s = 0; s < N_SYMS_SUB; s++) {
    for (int info = 0; info < N_INFO; info++) {
      for (int chunk = 0; chunk < (1 << (N_PER_AXIS + 1)); chunk++) {
        int mm = chunk;
        #ifndef QUARTER
          mm >>= 1;
        #endif
        if (info & 1) {
          int rev = 0;
          for (int i = 0; i < N_PER_AXIS; i++) {
            if ((all_movemask & MOVEBIT(i)) != 0) {
              rev = (rev << N_PER_MOVE) | (mm & ((1 << N_PER_MOVE) - 1));
              mm >>= N_PER_MOVE;
            }
          }
          mm = rev;
        }

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

Prun getFSTwistPrun(Coord flip, Coord sslice, Coord twist) {
  CCoord fslice = FSLICE(flip, SS_SLICE(sslice));
  CCoord fstwist = FSTWIST(
    COORD(fslice_sym[fslice]), conj_twist[twist][SYM(fslice_sym[fslice])]
  );
  return fstwist_prun[fstwist];
}

MoveMask getFSTwistMoves(Coord flip, Coord sslice, Coord twist, int togo) {
  CCoord fslice = FSLICE(flip, SS_SLICE(sslice));
  int s = SYM(fslice_sym[fslice]);
  CCoord fstwist = FSTWIST(
    COORD(fslice_sym[fslice]), conj_twist[twist][s]
  );
  Prun prun = fstwist_prun[fstwist];

  int delta = togo - DIST(prun);
  if (delta < 0)
    return 0;
  if (delta > 1)
    return all_movemask;
  prun >>= 8;

  MoveMask mm = 0;
  for (int ax = 0; ax < N_AXES; ax++) {
    mm |= mm_map[delta][INFO(mm_key[s][ax])][prun & ((1 << (N_PER_AXIS + 1)) - 1)] << OFF(mm_key[s][ax]);
    prun >>= N_PER_AXIS + 1;
  }

  return mm;
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

void initFSTwistPrun() {
  fstwist_prun = new Prun[N_FSTWIST];
  std::fill(fstwist_prun, fstwist_prun + N_FSTWIST, EMPTY8);

  fstwist_prun[0] = 0;
  for (int dist = 0, count = 0; count < N_FSTWIST; dist++) {
    CCoord c = 0;

    for (CCoord fssym = 0; fssym < N_FSLICE_SYM; fssym++) {
      Coord flip = FS_FLIP(fslice_raw[fssym]);
      Coord slice = FS_SLICE(fslice_raw[fssym]);

      for (Coord twist = 0; twist < N_TWIST; twist++, c++) {
        if (DIST(fstwist_prun[c]) != dist)
          continue;
        count++;

        int deltas[N_MOVES];
        for (int m = 0; m < N_MOVES; m++) {
          if ((all_movemask & MOVEBIT(m)) == 0) {
            deltas[m] = 0;
            continue;
          }

          Coord flip1 = flip_move[flip][m];
          Coord slice1 = sliceMove(slice, m);
          CCoord fslice1 = FSLICE(flip1, slice1);
          Coord twist1 = twist_move[twist][m];

          CCoord fssym1 = COORD(fslice_sym[fslice1]);
          twist1 = conj_twist[twist1][SYM(fslice_sym[fslice1])];
          CCoord c1 = FSTWIST(fssym1, twist1);

          if (fstwist_prun[c1] == EMPTY8)
            fstwist_prun[c1] = dist + 1;
          deltas[m] = DIST(fstwist_prun[c1]) - dist;

          int selfs = fslice_selfs[fssym1] >> 1;
          for (int s = 1; selfs > 0; selfs >>= 1, s++) { // bit 0 is always on -> > 0 to save an iteration
            if (selfs & 1) {
              CCoord c2 = FSTWIST(fssym1, conj_twist[twist1][s]);
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

  corned_prun[0] = 0;
  for (int dist = 0, count = 0; count < N_CORNED; dist++) {
    CCoord c = 0;
    for (Coord csym = 0; csym < N_CPERM_SYM; csym++) {
      for (Coord udedges = 0; udedges < N_UDEDGES2; udedges++, c++) {
        if (corned_prun[c] != dist)
          continue;
        count++;

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

          CCoord c1 = CORNSLICE(
            cperm_move[cperm][moves2[m]], sslice_move[sslice][moves2[m]]
          );
          if (cornslice_prun[c1] > dist1)
            cornslice_prun[c1] = dist1;
        }
      }
    }
    std::cout << dist << " " << count << "\n";
  }
}
