#include "sym.h"

#include <algorithm>
#include <iostream>

#define EMPTY ~SymCoord(0)

CubieCube sym_cubes[N_SYMS];
int inv_sym[N_SYMS];
int conj_move[N_MOVES][N_SYMS];

Coord (*conj_twist)[N_SYMS_DH4];
Coord (*conj_udedges)[N_SYMS_DH4];
Coord (*conj_flip)[N_SYMS_DH4];

SymCoord *fslice_sym;
SymCoord *corners_sym;
SymCoord *sslice_sym;
CoordL *fslice_raw;
CoordL *corners_raw;
CoordL *sslice_raw;
SelfSyms *fslice_selfs;
SelfSyms *corners_selfs;
SelfSyms *sslice_selfs;

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

void initConjFlip() {
  initConjCoord(&conj_flip, N_FLIP, getFlip, setFlip, mulEdges);
}

void initCoordSym(
  SymCoord **coord_sym,
  CoordL **coord_raw,
  SelfSyms **coord_selfs,
  int n_sym,
  int n_coord,
  Coord (*getCoord)(CubieCube &),
  void (*setCoord)(CubieCube &, Coord),
  void (*mul)(const CubieCube &, const CubieCube &, CubieCube &)
) {
  auto coord_sym1 = new SymCoord[n_coord];
  auto coord_raw1 = new CoordL[n_sym];
  auto coord_selfs1 = new SelfSyms[n_sym];
  std::fill(coord_sym1, coord_sym1 + n_coord, EMPTY);

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;
  int cls = 0;

  copy(kSolvedCube, cube1);
  for (Coord coord = 0; coord < n_coord; coord++) {
    setCoord(cube1, coord);

    if (coord_sym1[coord] != EMPTY)
      continue;

    coord_sym1[coord] = SYMCOORD(cls, 0);
    coord_raw1[cls] = coord;
    coord_selfs1[cls] = 1;

    for (int s = 1; s < N_SYMS_DH4; s++) {
      mul(sym_cubes[inv_sym[s]], cube1, tmp);
      mul(tmp, sym_cubes[s], cube2);
      Coord coord1 = getCoord(cube2);
      if (coord_sym1[coord1] == EMPTY)
        coord_sym1[coord1] = SYMCOORD(cls, s);
      else if (coord1 == coord)
        coord_selfs1[cls] |= 1 << s;
    }
    cls++;
  }

  *coord_sym = coord_sym1;
  *coord_raw = coord_raw1;
  *coord_selfs = coord_selfs1;
}

void initFlipSliceSym() {
  fslice_sym = new SymCoord[N_FSLICE];
  fslice_raw = new CoordL[N_FSLICE_SYM];
  fslice_selfs = new SelfSyms[N_FSLICE_SYM];
  std::fill(fslice_sym, fslice_sym + N_FSLICE, EMPTY);

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

      if (fslice_sym[fslice] != EMPTY)
        continue;

      fslice_sym[fslice] = SYMCOORD(cls, 0);
      fslice_raw[cls] = fslice;
      fslice_selfs[cls] = 1;

      for (int s = 1; s < N_SYMS_DH4; s++) {
        mulEdges(sym_cubes[inv_sym[s]], cube1, tmp);
        mulEdges(tmp, sym_cubes[s], cube2);
        CoordL fslice1 = FSLICE(getFlip(cube2), getSlice(cube2));
        if (fslice_sym[fslice1] == EMPTY)
          fslice_sym[fslice1] = SYMCOORD(cls, s);
        else if (fslice1 == fslice)
          fslice_selfs[cls] |= 1 << s;
      }
      cls++;
    }
  }
}

void initCornersSym() {
  initCoordSym(
    &corners_sym, &corners_raw, &corners_selfs,
    N_CORNERS_SYM, N_CORNERS_C,
    getCorners, setCorners,
    mulCorners
  );
}

void initSSliceSym() {
  initCoordSym(
    &sslice_sym, &sslice_raw, &sslice_selfs,
    N_SSLICE_SYM, N_SSLICE,
    getSSlice, setSSlice,
    mulEdges
  );
}
