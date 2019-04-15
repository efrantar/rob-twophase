#include <ctime>
#include <iostream>
#include <stdlib.h>

#include "cubie.h"
#include "coord.h"
#include "misc.h"
#include "moves.h"
#include "prun.h"
#include "sym.h"

double tock(clock_t tick) {
  return double(clock() - tick) / CLOCKS_PER_SEC;
}

void initCoordTables() {
  initMoveCubes();

  clock_t tick = clock();
  initTwistMove();
  initFlipMove();
  initSliceSortedMove();
  initUEdgesMove();
  initDEdgesMove();
  initUDEdgesMove();
  initCornersMove();
  initMergeUDEdges();
  std::cout << "Coord tables: " << tock(tick) << "\n";
}

void initSymTables() {
  initSyms();

  clock_t tick = clock();
  initConjTwist();
  initConjUDEdges();
  initFlipSliceSyms();
  initCornersSyms();
  std::cout << "Sym tables: " << tock(tick) << "\n";
}

void initPrunTables() {
  initPrun();

  clock_t tick = clock();
//  initFSSymTwistPrun3();
  initCSymUDEdgesPrun3();
  initCornersSliceSPrun();
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

void testCoordMoveP2(Coord coord_move[][N_MOVES_P2], int n_coords) {
  for (Coord c = 0; c < n_coords; c++) { 
    for (int m = 0; m < N_MOVES_P2; m++) {
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
  testCoord(N_TWIST_COORDS, getTwist, setTwist);
  testCoord(N_FLIP_COORDS, getFlip, setFlip);
  testCoord(N_SLICESORTED_COORDS, getSliceSorted, setSliceSorted);
  testCoord(N_UEDGES_COORDS, getUEdges, setUEdges);
  testCoord(N_DEDGES_COORDS, getDEdges, setDEdges);
  testCoord(N_UDEDGES_COORDS_P2, getUDEdges, setUDEdges);
  testCoord(N_CORNERS_COORDS, getCorners, setCorners);
  testCoord(N_SLICE_COORDS, getSlice, setSlice);
}

void testCoordMoves() {
  std::cout << "Testing coord moves ...\n";
  testCoordMove(twist_move, N_TWIST_COORDS);
  testCoordMove(flip_move, N_FLIP_COORDS);
  testCoordMove(slicesorted_move, N_SLICESORTED_COORDS);
  testCoordMove(uedges_move, N_UEDGES_COORDS);
  testCoordMove(dedges_move, N_DEDGES_COORDS);
  testCoordMoveP2(udedges_move, N_UEDGES_COORDS_P2);
  testCoordMove(corners_move, N_CORNERS_COORDS);
}

void testMergeUDEdges() {
  std::cout << "Testing udedges merging ...\n";
  CubieCube cube;
  for (Coord c = 0; c < N_UDEDGES_COORDS_P2; c++) {
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

  for (Coord flip = 0; flip < N_FLIP_COORDS; flip++) {
    for (Coord slice = 0; slice < N_SLICE_COORDS; slice++) {
      LargeCoord flipslice = FLIPSLICE(flip, slice);
      for (int m = 0; m < N_MOVES; m++) {
        LargeCoord flipslice1 = FLIPSLICE(
          flip_move[flip][m],
          SS_SLICE(slicesorted_move[SLICESORTED(slice)][m])
        );
        
        LargeCoord flipslice2 = flipslice_raw[flipslice_sym[flipslice]];
        int m_conj = conj_move[m][flipslice_sym_sym[flipslice]];
        flipslice2 = FLIPSLICE(
          flip_move[FS_FLIP(flipslice2)][m_conj],
          sliceMove(FS_SLICE(flipslice2), m_conj)
        );

        if (flipslice_sym[flipslice1] != flipslice_sym[flipslice2]) {
          std::cout << "error: " << flipslice << " " << m << "\n";
          return;
        }
      }
    }
  }

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;

  for (SymCoord c = 0; c < N_FLIPSLICE_SYM_COORDS; c++) {
    setFlip(cube1, FS_FLIP(flipslice_raw[c]));
    setSlice(cube1, FS_SLICE(flipslice_raw[c]));
    for (Sym s = 0; s < N_SYMS_DH4; s++) {
      mulEdges(sym_cubes[s], cube1, tmp);
      mulEdges(tmp, sym_cubes[inv_sym[s]], cube2);
      if (
        FLIPSLICE(getFlip(cube2), getSlice(cube2)) == flipslice_raw[c] && 
        (flipslice_symset[c] & (1 << s)) == 0
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
  
  for (Coord c = 0; c < N_CORNERS_COORDS; c++) {
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

  for (SymCoord c = 0; c < N_CORNERS_SYM_COORDS; c++) {
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
    mulCubes(sym_cubes[s], sym_cubes[inv_sym[s]], cube);
    if (!equal(cube, sym_cubes[0])) {
      std::cout << "error: " << (int) s << "\n";
      break;
    }
    mulCubes(sym_cubes[inv_sym[s]], sym_cubes[s], cube);
    if (!equal(cube, sym_cubes[0])) {
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

  /*
  int count1[14] = {};
  for (SymCoord fssym = 0; fssym < N_FLIPSLICE_SYM_COORDS; fssym++) {
    for (Coord twist = 0; twist < N_TWIST_COORDS; twist++) {
      count1[getDepthFSSymTwistPrun3(
        FS_FLIP(flipslice_raw[fssym]), SLICESORTED(FS_SLICE(flipslice_raw[fssym])), twist
      )]++; 
    }
    std::cout << "test\n";
  }
  std::cout << "fssymtwist:\n";
  for (int i = 0; i < 14; i++)
    std::cout << "depth " << i << ": " << count1[i] << "\n";
  */
 
  std::cout << "csymudedges:\n";
  int count2[12] = {};
  for (SymCoord csym = 0; csym < N_CORNERS_SYM_COORDS; csym++) {
    for (Coord udedges = 0; udedges < N_UDEDGES_COORDS_P2; udedges++)
      count2[getDepthCSymUDEdgesPrun3(corners_raw[csym], udedges)]++;
  }
  for (int i = 0; i < 12; i++)
    std::cout << "depth " << i << ": " << count2[i] << "\n";

  std::cout << "cornersslices:\n";
  int count3[15] = {};
  for (LargeCoord cornersslices = 0; cornersslices < N_CORNERSSLICES_COORDS; cornersslices++)
      count3[cornersslices_prun[cornersslices]]++;
  for (int i = 0; i < 15; i++)
    std::cout << "depth " << i << ": " << count3[i] << "\n";
}

int main() {
  initMisc();
  initCoordTables();
  initSymTables();
  initPrunTables();

  testCoords();
  testCoordMoves();
  testMergeUDEdges();
  testSyms();
  testFlipSliceSyms();
  testCornersSyms();
  testPrunTables();

  return 0;
}

