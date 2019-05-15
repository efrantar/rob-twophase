#include "sym.h"

#include <iostream>

#define EMPTY 0xffff

CubieCube sym_cubes[N_SYMS];
Sym inv_sym[N_SYMS];
int conj_move[N_MOVES][N_SYMS];

Coord (*conj_twist)[N_SYMS_DH4];
Coord (*conj_udedges)[N_SYMS_DH4];

SymCoord *fslice_sym;
Sym *fslice_sym_sym;
CoordL *fslice_raw;
SymSet *fslice_symset;

SymCoord *corners_sym;
Sym *corners_sym_sym;
Coord *corners_raw;
SymSet *corners_symset;

static bool init() {
  CubieCube cube;
  CubieCube tmp;

  copy(kSolvedCube, cube);
  for (Sym i = 0; i < N_SYMS; i++) {
    copy(cube, sym_cubes[i]);

    mul(cube, kLR2Cube, tmp);
    copy(tmp, cube);

    if (i % 2 == 1) {
      mul(cube, kU4Cube, tmp);
      copy(tmp, cube);
    }
    if (i % 8 == 7) {
      mul(cube, kF2Cube, tmp);
      copy(tmp, cube);
    }
    if (i % 16 == 15) {
      mul(cube, kURF3Cube, tmp);
      copy(tmp, cube);
    }
  }

  for (Sym i = 0; i < N_SYMS; i++) {
    for (Sym j = 0; j < N_SYMS; j++) {
      mul(sym_cubes[i], sym_cubes[j], cube);
      if (cube.cp[URF] == URF && cube.cp[UFL] == UFL && cube.cp[ULB] == ULB) {
        inv_sym[i] = j;
        break;
      }
    }
  }

  for (int m = 0; m < N_MOVES; m++) {
    for (Sym s = 0; s < N_SYMS; s++) {
      mul(sym_cubes[s], move_cubes[m], tmp);
      mul(tmp, sym_cubes[inv_sym[s]], cube);
      for (int conj = 0; conj < N_MOVES; conj++) {
        if (cube == move_cubes[conj]) {
          conj_move[m][s] = conj;
          break;
        }
      }
    }
  }

  return true;
}
static bool inited = init();

void initConjCoord(
  Coord (**conj_coord)[N_SYMS_DH4],
  int n_coords,
  Coord (*getCoord)(CubieCube &),
  void (*setCoord)(CubieCube &, Coord),
  void (*mul)(const CubieCube &, const CubieCube &, CubieCube &)
) {
  auto conj_coord1 = new Coord[n_coords][N_SYMS_DH4];

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;

  copy(kSolvedCube, cube1);
  for (Coord c = 0; c < n_coords; c++) {
    setCoord(cube1, c);
    conj_coord1[c][0] = c;
    for (Sym s = 1; s < N_SYMS_DH4; s++) { 
      mul(sym_cubes[s], cube1, tmp);
      mul(tmp, sym_cubes[inv_sym[s]], cube2);
      conj_coord1[c][s] = getCoord(cube2);
    }
  }

  *conj_coord = conj_coord1;
}

void initConjTwist() {
  initConjCoord(&conj_twist, N_TWIST, getTwist, setTwist, mulCorners);
}

void initConjUDEdges() {
  initConjCoord(&conj_udedges, N_UDEDGES2, getUDEdges, setUDEdges, mulEdges);
}

void initFlipSliceSyms() {
  fslice_sym = new SymCoord[N_FSLICE];
  fslice_sym_sym = new Sym[N_FSLICE];
  fslice_raw = new CoordL[N_FSLICE_SYM];
  fslice_symset = new SymSet[N_FSLICE_SYM];
  std::fill(fslice_sym, fslice_sym + N_FSLICE, EMPTY);

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;
  SymCoord cls = 0;

  copy(kSolvedCube, cube1);
  for (Coord slice = 0; slice < N_SLICE; slice++) {
    setSlice(cube1, slice);
    for (Coord flip = 0; flip < N_FLIP; flip++) {
      setFlip(cube1, flip);
      CoordL fslice = FSLICE(flip, slice);

      if (fslice_sym[fslice] != EMPTY)
        continue;

      fslice_sym[fslice] = cls;
      fslice_sym_sym[fslice] = 0;
      fslice_raw[cls] = fslice;
      fslice_symset[cls] = 1;

      for (Sym s = 1; s < N_SYMS_DH4; s++) {
        mulEdges(sym_cubes[inv_sym[s]], cube1, tmp);
        mulEdges(tmp, sym_cubes[s], cube2);
        CoordL fslice1 = FSLICE(getFlip(cube2), getSlice(cube2));
        if (fslice_sym[fslice1] == EMPTY) {
          fslice_sym[fslice1] = cls;
          fslice_sym_sym[fslice1] = s;
        } else if (fslice1 == fslice)
          fslice_symset[cls] |= 1 << s;
      }
      cls++;
    }
  }
}

void initCornersSyms() {
  corners_sym = new SymCoord[N_CORNERS_C];
  corners_sym_sym = new Sym[N_CORNERS_C];
  corners_raw = new Coord[N_CORNERS_SYM];
  corners_symset = new SymSet[N_CORNERS_SYM];
  std::fill(corners_sym, corners_sym + N_CORNERS_C, EMPTY);

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;
  SymCoord cls = 0;

  copy(kSolvedCube, cube1);
  for (Coord corners1 = 0; corners1 < N_CORNERS_C; corners1++) {
    setCorners(cube1, corners1);
    if (corners_sym[corners1] != EMPTY)
      continue;

    corners_sym[corners1] = cls;
    corners_sym_sym[corners1] = 0;
    corners_raw[cls] = corners1;
    corners_symset[cls] = 1;

    for (Sym s = 1; s < N_SYMS_DH4; s++) {
      mulCorners(sym_cubes[inv_sym[s]], cube1, tmp);
      mulCorners(tmp, sym_cubes[s], cube2);
      Coord corners2 = getCorners(cube2);
      if (corners_sym[corners2] == EMPTY) {
        corners_sym[corners2] = cls;
        corners_sym_sym[corners2] = s;
      } else if (corners2 == corners1)
        corners_symset[cls] |= 1 << s;
    }
    cls++;
  }
}
