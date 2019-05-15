/**
 * Possible optimizations:
 * - Sym and sym_sym in a single 2D table
 * - Raw and symset in a single 2D table
 * - Axis skipping in pruning table generation
 * - Symset on backward search?
 * - Switch from mod 3 to full 4 bit phase 2 pruning table representation -> 26MB
 */

#include <ctime>
#include <iostream>
#include <stdlib.h>

#include "cubie.h"
#include "coord.h"
#include "face.h"
#include "misc.h"
#include "moves.h"
#include "prun.h"
#include "solve.h"
#include "sym.h"

double tock(clock_t tick) {
  return double(clock() - tick) / CLOCKS_PER_SEC;
}

void initCoordTables() {
  clock_t tick = clock();
  initTwistMove();
  initFlipMove();
  initSSliceMove();
  initUEdgesMove();
  initDEdgesMove();
  initUDEdgesMove();
  initCornersMove();
  initMergeUDEdges();
  std::cout << "Coord tables: " << tock(tick) << "\n";
}

void initSymTables() {
  clock_t tick = clock();
  initConjTwist();
  initConjUDEdges();
  initFlipSliceSyms();
  initCornersSyms();
  std::cout << "Sym tables: " << tock(tick) << "\n";
}

void initPrunTables() {
  clock_t tick = clock();
  initFSTwistPrun3();
  initCornUDPrun3();
  initCornSlicePrun();
  std::cout << "Prun tables: " << tock(tick) << "\n";
}

void testCoord(int n_coords, Coord (*get)(CubieCube &), void (*set)(CubieCube &, Coord)) {
  CubieCube cube;
  for (Coord c = 0; c < n_coords; c++) {
    set(cube, c);
    if (get(cube) != c) {
      std::cout << "error: " << c << "\n";
      return;
    }
  }
  std::cout << "ok\n";
}

void testCube() {
  std::cout << "Testing cube ...\n";
  for (int i = 0; i < 1000; i++) {
    CubieCube c = randomCube();
    int tmp = check(c);
    if (tmp != 0) {
      std::cout << "error 1: " << tmp << "\n";
      return;
    }
    CubieCube c1;
    faceToCubie(cubieToFace(c), c1);
    if (c1 != c) {
      std::cout << "error 2\n";
      return;
    }
  }
  std::cout << "ok\n";
}

void testCoordMove(Coord coord_move[][N_MOVES], int n_coords) {
  for (Coord c = 0; c < n_coords; c++) {
    for (int m = 0; m < N_MOVES; m++) {
      if (coord_move[coord_move[c][m]][kInvMove[m]] != c) {
        std::cout << "error: " << c << "\n";
        return;
      }
    }
  }
  std::cout << "ok\n";
}

void testCoordMoveP2(Coord coord_move[][N_MOVES2], int n_coords) {
  for (Coord c = 0; c < n_coords; c++) { 
    for (int m = 0; m < N_MOVES2; m++) {
      int inv = 0;
      while (kPhase2Moves[inv] != kInvMove[kPhase2Moves[m]])
        inv++;
      if (coord_move[coord_move[c][m]][inv] != c) {
        std::cout << "error: " << c << "\n";
        return;
      }
    }
  }
  std::cout << "ok\n";
}

void testCoords() {
  std::cout << "Testing coords ...\n";
  testCoord(N_TWIST, getTwist, setTwist);
  testCoord(N_FLIP, getFlip, setFlip);
  testCoord(N_SSLICE, getSSlice, setSSlice);
  testCoord(N_UEDGES, getUEdges, setUEdges);
  testCoord(N_DEDGES, getDEdges, setDEdges);
  testCoord(N_UDEDGES2, getUDEdges, setUDEdges);
  testCoord(N_CORNERS_C, getCorners, setCorners);
  testCoord(N_SLICE, getSlice, setSlice);
}

void testCoordMoves() {
  std::cout << "Testing coord moves ...\n";
  testCoordMove(twist_move, N_TWIST);
  testCoordMove(flip_move, N_FLIP);
  testCoordMove(sslice_move, N_SSLICE);
  testCoordMove(uedges_move, N_UEDGES);
  testCoordMove(dedges_move, N_DEDGES);
  testCoordMoveP2(udedges_move, N_UEDGES2);
  testCoordMove(corners_move, N_CORNERS_C);
}

void testMergeUDEdges() {
  std::cout << "Testing udedges merging ...\n";
  CubieCube cube;
  for (Coord c = 0; c < N_UDEDGES2; c++) {
    setUDEdges(cube, c);
    CubieCube cube1;
    copy(cube, cube1);
    if (c != merge_udedges[getUEdges(cube)][getDEdges(cube) % 24]) {
      std::cout << "error: " << c << "\n";
      return;
    }
  }
  std::cout << "ok\n";
}

void testFlipSliceSyms() {
  std::cout << "Testing flipslice syms ...\n";

  for (Coord flip = 0; flip < N_FLIP; flip++) {
    for (Coord slice = 0; slice < N_SLICE; slice++) {
      CoordL flipslice = FSLICE(flip, slice);
      for (int m = 0; m < N_MOVES; m++) {
        CoordL flipslice1 = FSLICE(
          flip_move[flip][m],
          SS_SLICE(sslice_move[SSLICE(slice)][m])
        );
        
        CoordL flipslice2 = fslice_raw[fslice_sym[flipslice]];
        int m_conj = conj_move[m][fslice_sym_sym[flipslice]];
        flipslice2 = FSLICE(
          flip_move[FS_FLIP(flipslice2)][m_conj],
          sliceMove(FS_SLICE(flipslice2), m_conj)
        );

        if (fslice_sym[flipslice1] != fslice_sym[flipslice2]) {
          std::cout << "error: " << flipslice << " " << m << "\n";
          return;
        }
      }
    }
  }

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;

  for (SymCoord c = 0; c < N_FSLICE_SYM; c++) {
    setFlip(cube1, FS_FLIP(fslice_raw[c]));
    setSlice(cube1, FS_SLICE(fslice_raw[c]));
    for (Sym s = 0; s < N_SYMS_DH4; s++) {
      mulEdges(sym_cubes[s], cube1, tmp);
      mulEdges(tmp, sym_cubes[inv_sym[s]], cube2);
      if (
        FSLICE(getFlip(cube2), getSlice(cube2)) == fslice_raw[c] &&
        (fslice_symset[c] & (1 << s)) == 0
      ) {
        std::cout << "error: " << c << " " << (int) s << "\n";
        return;
      }
    }
  }
  std::cout << "ok\n";
}

void testCornersSyms() {
  std::cout << "Testing corners syms ...\n";
  
  for (Coord c = 0; c < N_CORNERS_C; c++) {
    for (int m : kPhase2Moves) {
      if (
        corners_sym[corners_move[corners_raw[corners_sym[c]]][conj_move[m][corners_sym_sym[c]]]]
        != corners_sym[corners_move[c][m]]
      ) {
        std::cout << "error: " << c << " " << m << "\n";
        return;
      }
    }
  }

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;

  for (SymCoord c = 0; c < N_CORNERS_SYM; c++) {
    setCorners(cube1, corners_raw[c]);
    for (Sym s = 0; s < N_SYMS_DH4; s++) {
      mulCorners(sym_cubes[s], cube1, tmp);
      mulCorners(tmp, sym_cubes[inv_sym[s]], cube2);
      if (getCorners(cube2) == corners_raw[c] && (corners_symset[c] & (1 << s)) == 0) {
        std::cout << "error: " << c << " " << (int) s << "\n";
        return;
      }
    }
  }

  std::cout << "ok\n";
}

void testSyms() {
  std::cout << "Testing syms ...\n";
  
  CubieCube cube;
  for (Sym s = 0; s < N_SYMS; s++) {
    mul(sym_cubes[s], sym_cubes[inv_sym[s]], cube);
    if (cube != sym_cubes[0]) {
      std::cout << "error: " << (int) s << "\n";
      break;
    }
    mul(sym_cubes[inv_sym[s]], sym_cubes[s], cube);
    if (cube != sym_cubes[0]) {
      std::cout << "error: " << (int) s << "\n";
      break;
    }
  }

  for (int m = 0; m < N_MOVES; m++) {
    for (Sym s = 0; s < N_SYMS; s++) {
      if (conj_move[conj_move[m][s]][inv_sym[s]] != m) {
        std::cout << "error: " << m << " " << (int) s << "\n";
        break;
      }
    }
  }

  std::cout << "ok\n";
}

void testPrunTables() {
  std::cout << "Testing pruning tables ...\n";

  std::cout << "fssymtwist:\n";
  int count1[13] = {};
  for (SymCoord fssym = 0; fssym < N_FSLICE_SYM; fssym++) {
    for (Coord twist = 0; twist < N_TWIST; twist++) {
      count1[getFSTwistDist(
        FS_FLIP(fslice_raw[fssym]), SSLICE(FS_SLICE(fslice_raw[fssym])), twist
      )]++;
    }
  }
  for (int i = 0; i < 13; i++)
    std::cout << "depth " << i << ": " << count1[i] << "\n";
 
  std::cout << "csymudedges:\n";
  int count2[12] = {};
  for (SymCoord csym = 0; csym < N_CORNERS_SYM; csym++) {
    for (Coord udedges = 0; udedges < N_UDEDGES2; udedges++)
      count2[getCORNUDDist(corners_raw[csym], udedges)]++;
  }
  for (int i = 0; i < 12; i++)
    std::cout << "depth " << i << ": " << count2[i] << "\n";

  std::cout << "cornersslices:\n";
  int count3[15] = {};
  for (CoordL cornersslices = 0; cornersslices < N_CORNSLICE; cornersslices++)
      count3[cornslice_prun[cornersslices]]++;
  for (int i = 0; i < 15; i++)
    std::cout << "depth " << i << ": " << count3[i] << "\n";

  for (int i = 1; i < 22; i++) {
    int j1 = (i + 1) % 3;
    int j2 = (i - 1) % 3;
    if (next_dist[i][j1] != i + 1 || next_dist[i][j2] != i - 1) {
      std::cout << "error\n";
      return;
    }
  }

  std::cout << "ok\n";
}

int main() {
  initCoordTables();
  initSymTables();
  initPrunTables();

  /*
  testCoords();
  testCoordMoves();
  testMergeUDEdges();
  testSyms();
  testFlipSliceSyms();
  testCornersSyms();
  testPrunTables();
  */

  // testCube();

  /*
  CubieCube c;
  faceToCubie("BUUDUURBFUFBRRFRUFBLRUFDRBBDDULDBFLLLFDRLLUFFLRDRBBDDL", c);
  solve(c, 20, 10000000);
  */

  for (int i = 0; i < 100; i++) {
    clock_t tick = clock();
    CubieCube c = randomCube();
    // std::cout << cubieToFace(c) << "\n";
    solve(c, 20, 10000000);
    std::cout << "Solve: " << tock(tick) << "\n";
  }

  return 0;
}
