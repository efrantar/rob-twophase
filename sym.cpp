#include "sym.h"

namespace sym {

  using namespace cubie::corner;
  using namespace cubie::edge;

  cubie::cube cubes[COUNT];
  int inv[COUNT];
  int effect[COUNT][3];

  int conj_move[move::COUNT][COUNT];
  uint16_t conj_twist[coord::N_TWIST][COUNT_SUB];
  uint16_t conj_udedges2[coord::N_UDEDGES2][COUNT_SUB];

  uint32_t fslice1_sym[coord::N_FSLICE1];
  uint32_t corners_sym[coord::N_CORNERS];
  uint32_t fslice1_raw[N_FSLICE1];
  uint16_t corners_raw[N_CORNERS];
  uint16_t fslice1_selfs[N_FSLICE1];
  uint16_t corners_selfs[N_CORNERS];

  void init_base() {
    cubie::cube c = cubie::SOLVED_CUBE;
    cubie::cube tmp;

    cubie::cube lr2 = {
      {UFL, URF, UBR, ULB, DLF, DFR, DRB, DBL},
      {UL, UF, UR, UB, DL, DF, DR, DB, FL, FR, BR, BL},
      {3, 3, 3, 3, 3, 3, 3, 3}, {} // special mirror ori
    };
    cubie::cube u4 = {
      {UBR, URF, UFL, ULB, DRB, DFR, DLF, DBL},
      {UB, UR, UF, UL, DB, DR, DF, DL, BR, FR, FL, BL},
      {}, {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1}
    };
    cubie::cube f2 = {
      {DLF, DFR, DRB, DBL, UFL, URF, UBR, ULB},
      {DL, DF, DR, DB, UL, UF, UR, UB, FL, FR, BR, BL},
      {}, {}
    };
    cubie::cube urf3 = {
      {URF, DFR, DLF, UFL, UBR, DRB, DBL, ULB},
      {UF, FR, DF, FL, UB, BR, DB, BL, UR, DR, DL, UL},
      {1, 2, 1, 2, 2, 1, 2, 1},
      {1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1}
    };

    // First 4 symmetries are the ones used in F5 mode
    for (int i = 0; i < COUNT; i++) {
      cubes[i] = c;

      cubie::mul(c, lr2, tmp);
      std::swap(tmp, c);

      if (i % 2 == 1) {
        cubie::mul(c, f2, tmp);
        std::swap(tmp, c);
      }
      if (i % 4 == 3) {
        cubie::mul(c, u4, tmp);
        std::swap(tmp, c);
      }
      if (i % 16 == 15) {
        cubie::mul(c, urf3, tmp);
        std::swap(tmp, c);
      }
    }

    /* Maybe not the most efficient, but overall time spent here completely negligible. */

    for (int i = 0; i < COUNT; i++) {
      for (int j = 0; j < COUNT; j++) {
        cubie::mul(cubes[i], cubes[j], c);
        if (c == cubie::SOLVED_CUBE) {
          inv[i] = j;
          break;
        }
      }
    }

    for (int m = 0; m < move::COUNT; m++) {
      for (int s = 0; s < COUNT; s++) {
        cubie::mul(cubes[s], move::cubes[m], tmp);
        cubie::mul(tmp, cubes[inv[s]], c);
        for (int conj = 0; conj < move::COUNT; conj++) {
          if (c == move::cubes[conj]) {
            conj_move[m][s] = conj;
            break;
          }
        }
      }
    }

    /* TODO */
    /*
    for (int s = 0; s < COUNT; s++) {
      for (int ax = 0; ax < 3; ax++) {
        effect[s][ax] = (conj_move[PER_AXIS * ax][inv[s]] / PER_AXIS) << 2; // shift
        effect[s][ax] |= (conj_move[PER_AXIS * ax][inv[s]] % PER_AXIS >= (PER_AXIS / 2)) << 1; // flip
        effect[s][ax] |= (conj_move[PER_AXIS * ax][inv[s]] % (PER_AXIS / 2) != 0) << 1; // inv
      }
    }
     */
  }

  void init_conjcoord(
    uint16_t conj_coord[][COUNT_SUB],
    int n_coords,
    int (*get_coord)(const cubie::cube&),
    void (*set_coord)(cubie::cube&, int),
    void (*mul)(const cubie::cube&, const cubie::cube&, cubie::cube&)
  ) {
    cubie::cube c1 = cubie::SOLVED_CUBE; // make sure all multiplications will work
    cubie::cube c2;
    cubie::cube tmp;

    for (int coord = 0; coord < n_coords; coord++) {
      set_coord(c1, coord);
      conj_coord[coord][0] = coord; // sym 0 is identity
      for (int s = 1; s < COUNT_SUB; s++) {
        mul(cubes[s], c1, tmp);
        mul(tmp, cubes[inv[s]], c2);
        conj_coord[coord][s] = get_coord(c2);
      }
    }
  }

  void init_fslice1() {
    std::fill(fslice1_sym, fslice1_sym + coord::N_FSLICE1, N_FSLICE1); // `N_FSLICE1` represents empty

    cubie::cube c1 = cubie::SOLVED_CUBE;
    cubie::cube c2;
    cubie::cube tmp;
    int cls = 0;

    for (int slice1 = 0; slice1 < coord::N_SLICE1; slice1++) {
      coord::set_slice1(c1, slice1); // SLICE is slightly more expensive to set
      for (int flip = 0; flip < coord::N_FLIP; flip++) {
        coord::set_flip(c1, flip);
        int fslice1 = coord::fslice1(flip, slice1);

        if (fslice1_sym[fslice1] != N_FSLICE1)
          continue;
        fslice1_sym[fslice1] = COUNT_SUB * cls;
        fslice1_raw[cls] = fslice1;
        fslice1_selfs[cls] = 1; // symmetry 0 is identity and always a self-sym

        for (int s = 1; s < COUNT_SUB; s++) {
          cubie::edge::mul(cubes[inv[s]], c1, tmp);
          cubie::edge::mul(tmp, cubes[s], c2);
          int fslice11 = coord::fslice1(coord::get_flip(c2), coord::get_slice1(c2));
          if (fslice1_sym[fslice11] == N_FSLICE1)
            fslice1_sym[fslice11] = COUNT_SUB * cls + s;
          else if (fslice11 == fslice1) // collect self-symmetries
            fslice1_selfs[cls] |= 1 << s;
        }
        cls++;
      }
    }
  }

  void init_corners() {
    std::fill(corners_sym, corners_sym + coord::N_CORNERS, N_CORNERS);

    cubie::cube c1 = cubie::SOLVED_CUBE;
    cubie::cube c2;
    cubie::cube tmp;
    int cls = 0;

    for (int corners = 0; corners < coord::N_CORNERS; corners++) {
      coord::set_corners(c1, corners);

      if (corners_sym[corners] != N_CORNERS)
        continue;
      corners_sym[corners] = COUNT_SUB * cls;
      corners_raw[cls] = corners;
      corners_selfs[cls] = 1;

      for (int s = 1; s < COUNT_SUB; s++) {
        cubie::corner::mul(cubes[inv[s]], c1, tmp);
        cubie::corner::mul(tmp, cubes[s], c2);
        int corners1 = coord::get_corners(c2);
        if (corners_sym[corners1] == N_CORNERS)
          corners_sym[corners1] = COUNT_SUB * cls + s;
        else if (corners1 == corners)
          corners_selfs[cls] |= 1 << s;
      }
      cls++;
    }
  }

  void init() {
    init_base();
    init_conjcoord(conj_twist, coord::N_TWIST, coord::get_twist, coord::set_twist, cubie::corner::mul);
    init_conjcoord(conj_udedges2, coord::N_UDEDGES2, coord::get_udedges2, coord::set_udedges2, cubie::edge::mul);
    init_fslice1();
    init_corners();
  }

}
