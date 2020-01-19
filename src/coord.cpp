#include "coord.h"

#include <algorithm>
#include <bitset>
#include <strings.h>

#include "cubie.h"

namespace coord {

  const int N_C12K4 = 495; // binom(12, 4)
  const int N_PERM4 = 24; // 4!

  uint16_t move_flip[N_FLIP][move::COUNT];
  uint16_t move_twist[N_TWIST][move::COUNT];
  uint16_t move_edges4[N_SLICE][move::COUNT];
  uint16_t move_corners[N_CORNERS][move::COUNT];
  uint16_t move_udedges2[N_UDEDGES2][move::COUNT];

  /* Used for en-/decoding pos-perm coords */
  uint8_t enc_perm[1 << (4 * 2)]; // encode 4-elem perm as 8 bits
  uint8_t dec_perm[N_PERM4];
  uint16_t enc_comb[1 << 12]; // encode 4-elem comb as 12-bit mask with exactly 4 bits on
  uint16_t dec_comb[N_C12K4];

  int binarize_perm(int perm[]) {
    int bin = 0;
    for (int i = 3; i >= 0; i--)
      bin = (bin << 2) | perm[i];
    return bin;
  }

  void init_encdec() {
    int perm[] = {0, 1, 2, 3};
    for (int i = 0; i < N_PERM4; i++) {
      int bin = binarize_perm(perm);
      enc_perm[bin] = i;
      dec_perm[i] = bin;
      std::next_permutation(perm, perm + 4);
    }

    int i = 0;
    for (int comb = 0; comb < (1 << cubie::edge::COUNT); comb++) {
      if (std::bitset<cubie::edge::COUNT>(comb).count() == 4) {
        enc_comb[comb] = i;
        dec_comb[i] = comb;
        i++;
      }
    }
  }

  int get_ori(const int oris[], int len, int n_oris) {
    int val = 0;
    for (int i = 0; i < len - 1; i++) // last ori can be reconstructed by parity
      val = n_oris * val + oris[i];
    return val;
  }

  void set_ori(int val, int oris[], int len, int n_oris) {
    int par = 0;
    for (int i = len - 2; i >= 0; i--) {
      oris[i] = val % n_oris;
      par += oris[i];
      val /= n_oris;
    }
    // Ori parity must always be 0
    oris[len - 1] = (n_oris - par % n_oris) % n_oris;
  }

  // `mask` indicates which 4 edges to compute the coordinate for
  int get_combperm(const int cubies[], int len, int mask) {
    int min_cubie = ffs(mask) - 1;

    int comb = 0;
    int perm = 0;

    for (int i = len - 1; i >= 0; i--) {
      if (mask & (1 << cubies[i])) {
        comb |= 1 << i;
        perm = (perm << 2) | (cubies[i] - min_cubie);
      }
    }

    return N_PERM4 * enc_comb[comb] + enc_perm[perm];
  }

  void set_combperm(int comb, int perm, int cubies[], int len, int min_cubie) {
    comb = dec_comb[comb];
    perm = dec_perm[perm];

    int cubie = 0;
    for (int i = 0; i < len; i++) {
      if (cubie == min_cubie)
        cubie += 4;
      if (comb & (1 << i)) {
        cubies[i] = (perm & 0x3) + min_cubie;
        perm >>= 2;
      } else
        cubies[i] = cubie++;
    }
  }

  /* Faster than using `*_comperm()` twice */

  int get_perm8(const int cubies[]) {
    int comb1 = 0;
    int perm1 = 0;
    int perm2 = 0;

    for (int i = 7; i >= 0; i--) {
      if (cubies[i] < 4) {
        comb1 |= 1 << i;
        perm1 = (perm1 << 2) | cubies[i];
      } else
        perm2 = (perm2 << 2) | (cubies[i] - 4);
    }

    comb1 = enc_comb[comb1];
    perm1 = enc_perm[perm1];
    perm2 = enc_perm[perm2];
    return N_PERM4 * (N_PERM4 * comb1 + perm1) + perm2;
  }

  void set_perm8(int perm8, int cubies[]) {
    int perm2 = dec_perm[perm8 % N_PERM4];
    int comb1 = dec_comb[(perm8 / N_PERM4) / N_PERM4];
    int perm1 = dec_perm[(perm8 / N_PERM4) % N_PERM4];

    for (int i = 0; i < 8; i++) {
      if (comb1 & (1 << i)) {
        cubies[i] = perm1 & 0x3;
        perm1 >>= 2;
      } else {
        cubies[i] = (perm2 & 0x3) + 4;
        perm2 >>= 2;
      }
    }
  }

  int get_twist(const cubie::cube& c) {
    return get_ori(c.cori, cubie::corner::COUNT, 3);
  }

  void set_twist(cubie::cube& c, int twist) {
    set_ori(twist, c.cori, cubie::corner::COUNT, 3);
  }

  int get_flip(const cubie::cube& c) {
    return get_ori(c.eori, cubie::edge::COUNT, 2);
  }

  void set_flip(cubie::cube& c, int flip) {
    set_ori(flip, c.eori, cubie::edge::COUNT, 2);
  }

  int get_slice(const cubie::cube& c) {
    return get_combperm(c.eperm, cubie::edge::COUNT, 0xf00);
  }

  void set_slice(cubie::cube& c, int slice) {
    set_combperm(slice / N_PERM4, slice % N_PERM4, c.eperm, cubie::edge::COUNT, cubie::edge::FR);
  }

  int get_uedges(const cubie::cube& c) {
    return get_combperm(c.eperm, cubie::edge::COUNT, 0x00f);
  }

  void set_uedges(cubie::cube& c, int uedges) {
    set_combperm(uedges / N_PERM4, uedges % N_PERM4, c.eperm, cubie::edge::COUNT, cubie::edge::UR);
  }

  int get_dedges(const cubie::cube& c) {
    return get_combperm(c.eperm, cubie::edge::COUNT, 0x0f0);
  }

  void set_dedges(cubie::cube& c, int dedges) {
    set_combperm(dedges / N_PERM4, dedges % N_PERM4, c.eperm, cubie::edge::COUNT, cubie::edge::DR);
  }

  int get_corners(const cubie::cube& c) {
    return get_perm8(c.cperm);
  }

  void set_corners(cubie::cube& c, int corners) {
    set_perm8(corners, c.cperm);
  }

  /* Dedicated methods again more efficient than `*_posperm()` */

  int get_slice1(const cubie::cube& c) {
    int slice1 = 0;
    for (int i = cubie::edge::COUNT - 1; i >= 0; i--) {
      if (c.eperm[i] >= cubie::edge::FR)
        slice1 |= 1 << i;
    }
    return enc_comb[slice1];
  }

  void set_slice1(cubie::cube& c, int slice1) {
    slice1 = dec_comb[slice1];
    int j = cubie::edge::FR;
    int cubie = 0;
    for (int i = 0; i < cubie::edge::COUNT; i++)
      c.eperm[i] = (slice1 & (1 << i)) ? j++ : cubie++;
  }

  int get_udedges2(const cubie::cube& c) {
    return get_perm8(c.eperm);
  }

  void set_udedges2(cubie::cube& c, int udedges2) {
    set_perm8(udedges2, c.eperm);
  }

  // Computing only exactly the moves that are needed and storing them tightly would only make things more complicated
  // during solving (in exchange for completely negligible setup/memory-gains)
  void init_move(
    uint16_t move_coord[][move::COUNT],
    int n_coord,
    int (*get_coord)(const cubie::cube&),
    void (*set_coord)(cubie::cube&, int),
    void (*mul)(const cubie::cube&, const cubie::cube&, cubie::cube&),
    bool phase2 = false
  ) {
    cubie::cube c1 = cubie::SOLVED_CUBE; // coords only affect perm or ori -> one would be uninitialized
    cubie::cube c2;

    for (int coord = 0; coord < n_coord; coord++) {
      set_coord(c1, coord);

      if (phase2) { // UDEDGES2 is only defined for phase 2 moves
        for (move::mask moves = move::p2mask; moves; moves &= moves - 1) {
          int m = ffsll(moves) - 1;
          mul(c1, move::cubes[m], c2);
          move_coord[coord][m] = get_coord(c2);
        }
      } else {
        for (int m = 0; m < move::COUNT; m++) {
          mul(c1, move::cubes[m], c2);
          move_coord[coord][m] = get_coord(c2);
        }
      }
    }
  }

  void init() {
    init_encdec();

    init_move(move_flip, N_FLIP, get_flip, set_flip, cubie::edge::mul);
    init_move(move_twist, N_TWIST, get_twist, set_twist, cubie::corner::mul);
    init_move(move_edges4, N_SLICE, get_slice, set_slice, cubie::edge::mul);
    init_move(move_corners, N_CORNERS, get_corners, set_corners, cubie::corner::mul);
    init_move(move_udedges2, N_UDEDGES2, get_udedges2, set_udedges2, cubie::edge::mul, true);
  }

}
