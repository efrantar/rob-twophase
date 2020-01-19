#include "prun.h"

#include <bitset>
#include <iostream>
#include <strings.h>

namespace prun {

  const std::string SAVE = "twophase.tbl";
  const int EMPTY = 0xff;

  #ifdef AX
    const int BITS_PER_AX = 16; // bits used for encoding an axis in the ext. phase 1 table
  #else
    const int BITS_PER_AX = 8;
  #endif
  #ifdef QT
    const int BITS_PER_M = 2; // bits per move
    const int N_SIMP = 4; // number of simple moves
    const int N_AX = 4; // number of axial moves
  #else
    const int BITS_PER_M = 1;
    const int N_SIMP = 6;
    const int N_AX = 9;
  #endif

  // Used to remap symmetry ext. phase 1 table entries back to actual situation
  move::mask remap[2][16][1 << BITS_PER_AX];

  prun1 *phase1;
  uint8_t *phase2;
  uint8_t *precheck;

  inline int ones(int count) { return (1 << count) - 1; }

  int rev(int movec, int count, int off = 0, int step = BITS_PER_M) {
    movec >>= step * off;

    int rev = 0;
    for (int i = 0; i < step * count; i += step) {
      rev = (rev << step) | (movec & ones(step));
      movec >>= step;
    }
    return rev << step * off;
  }

  int inv(int mask) {
    int n_per_face = N_SIMP / 2;

    int inv = rev(mask, n_per_face) | rev(mask, n_per_face, n_per_face);
    #ifdef AX
      inv |= rev(mask, N_AX, N_SIMP);
    #endif
    return inv;
  }
  
  int flip(int mask) {
    int per_face = N_SIMP / 2;
    int flipped = rev(mask, 2, 0, BITS_PER_M * per_face);

    #ifdef AX
      mask >>= BITS_PER_M * N_SIMP;

      // Flipping an axis means to transpose the axial move-mask (M N) (M N2) (M N') ... to (N M) (N M2) (N M') ...
      int fax = 0;
      for (int i = 0; i < per_face; i++) {
        for (int j = 0; j < per_face; j++)
          fax |= ((mask >> BITS_PER_M * (per_face * i + j)) & ones(BITS_PER_M)) << BITS_PER_M * (per_face * j + i);
      }
      flipped |= fax << BITS_PER_M * N_SIMP;
    #endif

    return flipped;
  }

  void init_base() {
    /* It is probably cleanest to simply handle the special HT case individually */
    #ifndef AX
      #ifndef QT
        for (int eff = 0; eff < 16; eff++) {
          for (int mask = 0; mask < 256; mask++) {
            int mask1 = mask & 0xf;
            int mask2 = (mask & 0xf0) >> 4;

            if (sym::eff_inv(eff)) {
              mask1 = rev(mask1, 3, 1) | (mask1 & 1);
              mask2 = rev(mask2, 3, 1) | (mask2 & 1);
            }
            if (sym::eff_flip(eff))
              std::swap(mask1, mask2);

            remap[0][eff][mask] = ((mask1 & 1) ? 0 : ~(mask1 >> 1) & 0x7) << 6 * sym::eff_shift(eff);
            remap[1][eff][mask] = ((mask1 & 1) ? ~(mask1 >> 1) & 0x7 : 0x7) << 6 * sym::eff_shift(eff);
            remap[0][eff][mask] |= ((mask2 & 1) ? 0 : ~(mask2 >> 1) & 0x7) << 6 * sym::eff_shift(eff) + 3;
            remap[1][eff][mask] |= ((mask2 & 1) ? ~(mask2 >> 1) & 0x7 : 0x7) << 6 * sym::eff_shift(eff) + 3;
          }
        }
        return;
      #endif
    #endif

    for (int eff = 0; eff < 16; eff++) {
      for (int mask = 0; mask < (1 << BITS_PER_AX); mask++) {
        move::mask mask1 = mask;
        #ifndef QT
          mask1 >>= 1; // first bit encodes direction in HT
        #endif

        if (sym::eff_inv(eff))
          mask1 = inv(mask1);
        if (sym::eff_flip(eff))
          mask1 = flip(mask1);

        #ifdef QT
          remap[0][eff][mask] = 0;
          remap[1][eff][mask] = 0;
          for (int i = 0; i < BITS_PER_AX / 2; i++) {
            remap[0][eff][mask] |= ((mask1 & 0x3) == 0) << i;
            remap[1][eff][mask] |= ((mask1 & 0x3) <= 1) << i;
            mask1 >>= 2;
          }
          remap[0][eff][mask] <<= (BITS_PER_AX / 2) * sym::eff_shift(eff);
          remap[1][eff][mask] <<= (BITS_PER_AX / 2) * sym::eff_shift(eff);
        #else
          move::mask o = ones(BITS_PER_AX - 1); // first bit indicates direction but is not a move
          remap[0][eff][mask] = ((mask & 1) ? 0 : ~mask1 & o) << (BITS_PER_AX - 1) * sym::eff_shift(eff);
          remap[1][eff][mask] = ((mask & 1) ? ~mask1 & o : o) << (BITS_PER_AX - 1) * sym::eff_shift(eff);
        #endif
      }
    }
  }

  void init_phase1() {
    int n_moves = std::bitset<64>(move::p1mask).count(); // make sure not to consider B-moves in F5-mode

    phase1 = new prun1[N_FS1TWIST];
    std::fill(phase1, phase1 + N_FS1TWIST, EMPTY);

    phase1[coord::N_TWIST * sym::coord_c(sym::fslice1_sym[coord::fslice1(0, coord::SLICE1_SOLVED)])] = 0;
    int count = 0;
    int dist = 0;

    while (count < N_FS1TWIST) {
      int coord = 0;

      for (int fs1sym = 0; fs1sym < sym::N_FSLICE1; fs1sym++) {
        int fslice1 = sym::fslice1_raw[fs1sym];
        int flip = coord::fslice1_to_flip(fslice1);
        int slice = coord::slice1_to_slice(coord::fslice1_to_slice1(fslice1));

        for (int twist = 0; twist < coord::N_TWIST; twist++) {
          if ((phase1[coord] & 0xff) == dist) {
            count++;
            int deltas[move::COUNT1]; // easier encoding if B-face always exists (F5-mode ignores it anyways)

            for (int m = 0; m < n_moves; m++) {
              int slice11 = coord::slice_to_slice1(coord::move_edges4[slice][m]);
              int fslice11 = coord::fslice1(coord::move_flip[flip][m], slice11);
              int tmp = sym::fslice1_sym[fslice11];
              int twist1 = sym::conj_twist[coord::move_twist[twist][m]][sym::coord_s(tmp)];
              int fs1sym1 = sym::coord_c(tmp);
              int coord1 = coord::N_TWIST * fs1sym1 + twist1;

              if (phase1[coord1] == EMPTY)
                phase1[coord1] = dist + 1;
              deltas[m] = (phase1[coord1] & 0xff) - dist;
              coord1 -= twist1; // only TWIST part changes below

              int selfs = sym::fslice1_selfs[fs1sym1] >> 1;
              for (int s = 1; selfs > 0; s++) { // bit 0 is always on
                if (selfs & 1) {
                  int coord2 = coord1 + sym::conj_twist[twist1][s];
                  if (phase1[coord2] == EMPTY)
                    phase1[coord2] = dist + 1;
                }
                selfs >>= 1;
              }
            }

            prun1 prun = 0;
            #ifdef QT
              // In QT there is enough space to simply encode the effect of every move in 2 bits
              for (int m = n_moves - 1; m >= 0; m--)
                  prun = (prun << 2) | (deltas[m] + 1);
            #else
              #ifndef AX
                int n_ax = 6; // in standard (HT) mode we have to treat every face as an individual axis for encoding
                int bits_per_ax = 4;
              #else
                int n_ax = 3;
                int bits_per_ax = BITS_PER_AX;
              #endif
                /* Encode from left to right to preserve indexing of moves */
                for (int ax = n_ax - 1; ax >= 0; ax--) {
                  bool away = false; // first bit of axis encoding (whether any move brings us further from the goal)
                  for (int i = ax * (bits_per_ax - 1); i < (ax + 1) * (bits_per_ax - 1); i++) {
                    if (deltas[i] != 0) {
                      if (deltas[i] > 0)
                        away = true;
                      break; // stop immediately once we found a value != 0
                    }
                  }

                  int tmp = 0;
                  for (int i = (ax + 1) * (bits_per_ax - 1) - 1; i >= ax * (bits_per_ax - 1); i--)
                    tmp = (tmp | (away ? deltas[i] : deltas[i] + 1)) << 1;
                  tmp |= away;

                  prun = (prun << bits_per_ax) | tmp;
                }
            #endif
            phase1[coord] |= prun << 8;
          }
          coord++;
        }
      }

      std::cout << dist << " " << count << std::endl;
      dist++;
    }
  }

  void init_phase2() {
    phase2 = new uint8_t[N_CORNUD2];
    std::fill(phase2, phase2 + N_CORNUD2, EMPTY);

    phase2[0] = 0;
    int count = 0;
    int dist = 0;

    while (count < N_CORNUD2) {
      int coord = 0;

      for (int csym = 0; csym < sym::N_CORNERS; csym++) {
        int corners = sym::corners_raw[csym];

        for (int udedges2 = 0; udedges2 < coord::N_UDEDGES2; udedges2++) {
          if (phase2[coord] == dist) {
            count++;

            for (move::mask moves = move::p2mask; moves; moves &= moves - 1) {
              int m = ffsll(moves) - 1;

              int dist1 = dist + 1;
              #ifdef QT
                if (m >= move::COUNT1)
                  dist1++; // half-turns cost 2 in QTM
              #endif

              int corners1 = coord::move_corners[corners][m];
              int udedges21 = coord::move_udedges2[udedges2][m];
              int tmp = sym::corners_sym[corners1];
              udedges21 = sym::conj_udedges2[udedges21][sym::coord_s(tmp)];
              int csym1 = sym::coord_c(tmp);
              int coord1 = coord::N_UDEDGES2 * csym1 + udedges21;

              if (phase2[coord1] <= dist1)
                continue;
              phase2[coord1] = dist1;
              coord1 -= udedges21;

              int selfs = sym::corners_selfs[csym1] >> 1;
              for (int s = 1; selfs > 0; s++) {
                if (selfs & 1) {
                  int coord2 = coord1 + sym::conj_udedges2[udedges21][s];
                  if (phase2[coord2] > dist1)
                    phase2[coord2] = dist1;
                }
                selfs >>= 1;
              }
            }
          }
          coord++;
        }
      }

      std::cout << dist << " " << count << std::endl;
      dist++;
    }
  }

  void init_precheck() {
    precheck = new uint8_t[N_CSLICE2];
    std::fill(precheck, precheck + N_CSLICE2, EMPTY);

    precheck[0] = 0;
    int dist = 0;
    int count = 0;

    while (count < N_CSLICE2) {
      int coord = 0;

      for (int corners = 0; corners < coord::N_CORNERS; corners++) {
        for (int slice2 = 0; slice2 < coord::N_SLICE2; slice2++) {
          if (precheck[coord] == dist) {
            count++;
            int slice = coord::slice2_to_slice(slice2);

            for (move::mask moves = move::p2mask; moves; moves &= moves - 1) {
              int m = ffsll(moves) - 1;

              int dist1 = dist + 1;
              #ifdef QT
                if (m >= move::COUNT1)
                  dist1++; // half-turns cost 2 in QTM
              #endif

              int corners1 = coord::move_corners[corners][m];
              int slice21 = coord::slice_to_slice2(coord::move_edges4[slice][m]);

              int coord1 = coord::N_SLICE2 * corners1 + slice21;
              if (precheck[coord1] > dist1)
                precheck[coord1] = dist1;
            }
          }
          coord++;
        }
      }

      std::cout << dist << " " << count << std::endl;
      dist++;
    }
  }

  int get_phase1(int flip, int slice, int twist, int togo, move::mask& next) {
    int tmp = sym::fslice1_sym[coord::fslice1(flip, coord::slice_to_slice1(slice))];
    int s = sym::coord_s(tmp);
    prun1 prun = phase1[coord::N_TWIST * sym::coord_c(tmp) + sym::conj_twist[twist][s]];

    int dist = prun & 0xff;
    int delta = togo - dist;

    // `delta` < 0 case can never happen during a real search
    if (delta > 1)
      next = move::p1mask; // all moves are possible
    else {
      prun >>= 8; // get rid of dist
      next = 0;
      for (int ax = 0; ax < 3; ax++) {
        next |= remap[delta][sym::effect[s][ax]][prun & ones(BITS_PER_AX)];
        prun >>= BITS_PER_AX;
      }
    }

    return dist;
  }

  int get_phase2(int corners, int udedges) {
    int tmp = sym::corners_sym[corners];
    return phase2[coord::N_UDEDGES2 * sym::coord_c(tmp) + sym::conj_udedges2[udedges][sym::coord_s(tmp)]];
  }

  int get_precheck(int corners, int slice) {
    return precheck[coord::N_SLICE2 * corners + coord::slice_to_slice2(slice)];
  }

  bool init(bool file) {
    init_base();

    if (!file) {
      init_phase1();
      init_phase2();
      init_precheck();
      return true;
    }

    FILE *f = fopen(SAVE.c_str(), "rb");
    int err = 0;

    if (f == NULL) {
      init_phase1();
      init_phase2();
      init_precheck();

      f = fopen(SAVE.c_str(), "wb");
      if (fwrite(phase1, sizeof(prun1), N_FS1TWIST, f) != N_FS1TWIST)
        err = 1;
      if (fwrite(phase2, sizeof(uint8_t), N_CORNUD2, f) != N_CORNUD2)
        err = 1;
      if (fwrite(precheck, sizeof(uint8_t), N_CSLICE2, f) != N_CSLICE2)
        err = 1;
      if (err)
        remove(SAVE.c_str()); // delete file if there was some error writing it
    } else {
      phase1 = new prun1[N_FS1TWIST];
      phase2 = new uint8_t[N_CORNUD2];
      precheck = new uint8_t[N_CSLICE2];
      if (fread(phase1, sizeof(prun1), N_FS1TWIST, f) != N_FS1TWIST)
        err = 1;
      if (fread(phase2, sizeof(uint8_t), N_CORNUD2, f) != N_CORNUD2)
        err = 1;
      if (fread(precheck, sizeof(uint8_t), N_CSLICE2, f) != N_CSLICE2)
        err = 1;
    }

    fclose(f);
    return err;
  }

}
