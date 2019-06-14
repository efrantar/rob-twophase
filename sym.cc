#include "sym.h"

#include <algorithm>
#include <iostream>

#define EMPTY ~SymCoord(0)

CubieCube sym_cubes[N_SYMS];
int inv_sym[N_SYMS];
int conj_move[N_MOVES][N_SYMS];

Coord (*conj_twist)[N_SYMS_DH4];
Coord (*conj_udedges)[N_SYMS_DH4];
Coord (*conj_flip)[N_SYMS_DH4][N_SSLICE_SYM];

SymCoord *fslice_sym;
SymCoord *corners_sym;
SymCoord *sslice_sym;
CCoord *fslice_raw;
Coord *corners_raw;
Coord *sslice_raw;
SelfSyms *fslice_selfs;
SelfSyms *corners_selfs;
SelfSyms *sslice_selfs;

static bool init() {
  CubieCube cube;
  CubieCube tmp;

  cube = kSolvedCube;
  for (int i = 0; i < N_SYMS; i++) {
    sym_cubes[i] = cube;

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
      // Sufficient to check for equality in this special case
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

void initConjCoord(
  Coord (**conj_coord)[N_SYMS_DH4],
  int n_coords,
  Coord (*getCoord)(const CubieCube &),
  void (*setCoord)(CubieCube &, Coord),
  void (*mul)(const CubieCube &, const CubieCube &, CubieCube &)
) {
  auto conj_coord1 = new Coord[n_coords][N_SYMS_DH4];

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;

  cube1 = kSolvedCube;
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
  conj_flip = new Coord[N_FLIP][N_SYMS_DH4][N_SSLICE_SYM];

  CubieCube cube1;
  CubieCube cube2;
  CubieCube cube3;
  CubieCube tmp;

  // Init identity first to save cubie multiplications and coordinate codings
  for (Coord sssym = 0; sssym < N_SSLICE_SYM; sssym++) {
    for (Coord flip = 0; flip < N_FLIP; flip++) {
      conj_flip[flip][0][sssym] = flip;
    }
  }

  cube1 = kSolvedCube;
  for (Coord sssym = 0; sssym < N_SSLICE_SYM; sssym++) {
    setSSlice(cube1, sslice_raw[sssym]); // setting this is more expensive than flip -> outer loop

    for (int s = 1; s < N_SYMS_DH4; s++) {
      // Necessary so that result after setting flip and conjugating by `s` has indeed SSLICE `sssym`
      mulEdges(sym_cubes[inv_sym[s]], cube1, tmp);
      mulEdges(tmp, sym_cubes[s], cube2);

      for (Coord flip = 0; flip < N_FLIP; flip++) {
        setFlip(cube2, flip);
        mulEdges(sym_cubes[s], cube2, tmp);
        mulEdges(tmp, sym_cubes[inv_sym[s]], cube3);
        conj_flip[flip][s][sssym] = getFlip(cube3);
      }
    }
  }
}

void initCoordSym(
  SymCoord **coord_sym,
  Coord **coord_raw,
  SelfSyms **coord_selfs,
  int n_sym,
  int n_coord,
  Coord (*getCoord)(const CubieCube &),
  void (*setCoord)(CubieCube &, Coord),
  void (*mul)(const CubieCube &, const CubieCube &, CubieCube &)
) {
  auto coord_sym1 = new SymCoord[n_coord];
  auto coord_raw1 = new Coord[n_sym];
  auto coord_selfs1 = new SelfSyms[n_sym];
  std::fill(coord_sym1, coord_sym1 + n_coord, EMPTY);

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;
  int cls = 0;

  cube1 = kSolvedCube;
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
      else if (coord1 == coord) // collect self-symmetries here essentially for free
        coord_selfs1[cls] |= 1 << s;
    }
    cls++;
  }

  *coord_sym = coord_sym1;
  *coord_raw = coord_raw1;
  *coord_selfs = coord_selfs1;
}

// We cannot directly reuse initCoordSym() as we want a double loop here
void initFlipSliceSym() {
  fslice_sym = new SymCoord[N_FSLICE];
  fslice_raw = new CCoord[N_FSLICE_SYM];
  fslice_selfs = new SelfSyms[N_FSLICE_SYM];
  std::fill(fslice_sym, fslice_sym + N_FSLICE, EMPTY);

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;
  int cls = 0;

  cube1 = kSolvedCube;
  for (Coord slice = 0; slice < N_SLICE; slice++) {
    setSSlice(cube1, SSLICE(slice)); // SLICE the more expensive one to set -> outer loop
    for (Coord flip = 0; flip < N_FLIP; flip++) {
      setFlip(cube1, flip);
      CCoord fslice = FSLICE(flip, slice);

      if (fslice_sym[fslice] != EMPTY)
        continue;

      fslice_sym[fslice] = SYMCOORD(cls, 0);
      fslice_raw[cls] = fslice;
      fslice_selfs[cls] = 1;

      for (int s = 1; s < N_SYMS_DH4; s++) {
        mulEdges(sym_cubes[inv_sym[s]], cube1, tmp);
        mulEdges(tmp, sym_cubes[s], cube2);
        CCoord fslice1 = FSLICE(getFlip(cube2), SS_SLICE(getSSlice(cube2)));
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
    N_CORNERS_SYM, N_CPERM,
    getCPerm, setCPerm,
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
