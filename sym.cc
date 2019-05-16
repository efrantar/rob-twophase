#include "sym.h"

#include <algorithm>
#include <iostream>

#define EMPTY 0xffff

CubieCube sym_cubes[N_SYMS];
int inv_sym[N_SYMS];
int conj_move[N_MOVES][N_SYMS];

Coord (*conj_twist)[N_SYMS_DH4];
Coord (*conj_udedges)[N_SYMS_DH4];

SymCoord *fslice_sym;
RawFSlice *fslice_raw;
SymCoord *corners_sym;
RawCorners *corners_raw;

static bool init() {
  CubieCube cube;
  CubieCube tmp;

  copy(kSolvedCube, cube);
  for (int i = 0; i < N_SYMS; i++) {
    copy(cube, sym_cubes[i]);

    mul(cube, kLR2Cube, tmp);
    std::swap(tmp, cube);

    if (i % 2 == 1) {
      mul(cube, kU4Cube, tmp);
      std::swap(tmp, cube);
    }
    if (i % 8 == 7) {
      mul(cube, kF2Cube, tmp);
      std::swap(tmp, cube);
    }
    if (i % 16 == 15) {
      mul(cube, kURF3Cube, tmp);
      std::swap(tmp, cube);
    }
  }

  for (int i = 0; i < N_SYMS; i++) {
    for (int j = 0; j < N_SYMS; j++) {
      mul(sym_cubes[i], sym_cubes[j], cube);
      if (cube.cp[URF] == URF && cube.cp[UFL] == UFL && cube.cp[ULB] == ULB) {
        inv_sym[i] = j;
        break;
      }
    }
  }

  for (int m = 0; m < N_MOVES; m++) {
    for (int s = 0; s < N_SYMS; s++) {
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

void checkSyms(const CubieCube &cube, bool &rot, bool &anti) {
  CubieCube cube1;
  CubieCube tmp;

  for (int s : {16, 20, 24, 28}) {
    mul(sym_cubes[s], cube, tmp);
    mul(tmp, sym_cubes[inv_sym[s]], cube1);
    if (cube1 == cube) {
      rot = true;
      break;
    }
  }
  for (int s = 0; s < N_SYMS; s++) {
    mul(sym_cubes[s], cube, tmp);
    mul(tmp, sym_cubes[inv_sym[s]], cube1);
    if (invCube(cube1) == cube) {
      anti = true;
      break;
    }
  }
}

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
    for (int s = 1; s < N_SYMS_DH4; s++) {
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

void initFlipSliceSym() {
  fslice_sym = new SymCoord[N_FSLICE];
  fslice_raw = new RawFSlice[N_FSLICE_SYM];
  for (int i = 0; i < N_FSLICE; i++)
    fslice_sym[i].coord = EMPTY;

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;
  int cls = 0;

  copy(kSolvedCube, cube1);
  for (Coord slice = 0; slice < N_SLICE; slice++) {
    setSlice(cube1, slice);
    for (Coord flip = 0; flip < N_FLIP; flip++) {
      setFlip(cube1, flip);
      CoordL fslice = FSLICE(flip, slice);

      if (fslice_sym[fslice].coord != EMPTY)
        continue;

      fslice_sym[fslice].coord = cls;
      fslice_sym[fslice].sym = 0;
      fslice_raw[cls].coord = fslice;
      fslice_raw[cls].syms = 1;

      for (int s = 1; s < N_SYMS_DH4; s++) {
        mulEdges(sym_cubes[inv_sym[s]], cube1, tmp);
        mulEdges(tmp, sym_cubes[s], cube2);
        CoordL fslice1 = FSLICE(getFlip(cube2), getSlice(cube2));
        if (fslice_sym[fslice1].coord == EMPTY) {
          fslice_sym[fslice1].coord = cls;
          fslice_sym[fslice1].sym = s;
        } else if (fslice1 == fslice)
          fslice_raw[cls].syms |= 1 << s;
      }
      cls++;
    }
  }
}

void initCornersSym() {
  corners_sym = new SymCoord[N_CORNERS_C];
  corners_raw = new RawCorners[N_CORNERS_SYM];
  for (int i = 0; i < N_CORNERS_C; i++)
    corners_sym[i].coord = EMPTY;

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;
  int cls = 0;

  copy(kSolvedCube, cube1);
  for (Coord corners1 = 0; corners1 < N_CORNERS_C; corners1++) {
    setCorners(cube1, corners1);
    if (corners_sym[corners1].coord != EMPTY)
      continue;

    corners_sym[corners1].coord = cls;
    corners_sym[corners1].sym = 0;
    corners_raw[cls].coord = corners1;
    corners_raw[cls].syms = 1;

    for (int s = 1; s < N_SYMS_DH4; s++) {
      mulCorners(sym_cubes[inv_sym[s]], cube1, tmp);
      mulCorners(tmp, sym_cubes[s], cube2);
      Coord corners2 = getCorners(cube2);
      if (corners_sym[corners2].coord == EMPTY) {
        corners_sym[corners2].coord = cls;
        corners_sym[corners2].sym = s;
      } else if (corners2 == corners1)
        corners_raw[cls].syms |= 1 << s;
    }
    cls++;
  }
}
