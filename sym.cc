#include "sym.h"

#include <algorithm>

#define EMPTY2 ~SymCoord(0)

CubieCube sym_cubes[N_SYMS];
int inv_sym[N_SYMS];
int conj_move[N_MOVES][N_SYMS];

Coord (*conj_twist)[N_SYMS_SUB];
Coord (*conj_udedges)[N_SYMS_SUB];

SymCoord *fslice_sym;
SymCoord *cperm_sym;
CCoord *fslice_raw;
Coord *cperm_raw;
SelfSyms *fslice_selfs;
SelfSyms *cperm_selfs;

void initSym() {
  CubieCube cube;
  CubieCube tmp;

  cube = kSolvedCube;
  for (int i = 0; i < N_SYMS; i++) {
    sym_cubes[i] = cube;

    mul(cube, kLR2Cube, tmp);
    std::swap(tmp, cube);

    // First 4 symmetries usable in 5-face solver, first 16 in the standard solver
    if (i % 2 == 1) {
      mul(cube, kF2Cube, tmp);
      std::swap(tmp, cube);
    }
    if (i % 4 == 3) {
      mul(cube, kU4Cube, tmp);
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
      if (cube == kSolvedCube) {
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
}

void initConjCoord(
  Coord (**conj_coord)[N_SYMS_SUB],
  int n_coords,
  Coord (*getCoord)(const CubieCube &),
  void (*setCoord)(CubieCube &, Coord),
  void (*mul)(const CubieCube &, const CubieCube &, CubieCube &)
) {
  auto conj_coord1 = new Coord[n_coords][N_SYMS_SUB];

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;

  cube1 = kSolvedCube;
  for (Coord c = 0; c < n_coords; c++) {
    setCoord(cube1, c);
    conj_coord1[c][0] = c;
    for (int s = 1; s < N_SYMS_SUB; s++) {
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

// We want a double loop here (for efficiency), hence we cannot easily share code with `initCPermSym()`
void initFlipSliceSym() {
  fslice_sym = new SymCoord[N_FSLICE];
  fslice_raw = new CCoord[N_FSLICE_SYM];
  fslice_selfs = new SelfSyms[N_FSLICE_SYM];
  std::fill(fslice_sym, fslice_sym + N_FSLICE, EMPTY2);

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;
  int cls = 0;

  cube1 = kSolvedCube;
  for (Coord slice = 0; slice < N_SLICE; slice++) {
    setSSlice(cube1, SSLICE(slice)); // SLICE the (slightly) more expensive one to set -> outer loop
    for (Coord flip = 0; flip < N_FLIP; flip++) {
      setFlip(cube1, flip);
      CCoord fslice = FSLICE(flip, slice);

      if (fslice_sym[fslice] != EMPTY2)
        continue;

      fslice_sym[fslice] = SYMCOORD(cls, 0);
      fslice_raw[cls] = fslice;
      fslice_selfs[cls] = 1;

      for (int s = 1; s < N_SYMS_SUB; s++) {
        mulEdges(sym_cubes[inv_sym[s]], cube1, tmp);
        mulEdges(tmp, sym_cubes[s], cube2);
        CCoord fslice1 = FSLICE(getFlip(cube2), SS_SLICE(getSSlice(cube2)));
        if (fslice_sym[fslice1] == EMPTY2)
          fslice_sym[fslice1] = SYMCOORD(cls, s);
        else if (fslice1 == fslice) // collect self-symmetries here essentially for free
          fslice_selfs[cls] |= 1 << s;
      }
      cls++;
    }
  }
}

void initCPermSym() {
  cperm_sym = new SymCoord[N_CPERM];
  cperm_raw = new Coord[N_CPERM_SYM];
  cperm_selfs = new SelfSyms[N_CPERM_SYM];
  std::fill(cperm_sym, cperm_sym + N_CPERM, EMPTY2);

  CubieCube cube1;
  CubieCube cube2;
  CubieCube tmp;
  int cls = 0;

  cube1 = kSolvedCube;
  for (Coord cperm = 0; cperm < N_CPERM; cperm++) {
    setCPerm(cube1, cperm);

    if (cperm_sym[cperm] != EMPTY2)
      continue;

    cperm_sym[cperm] = SYMCOORD(cls, 0);
    cperm_raw[cls] = cperm;
    cperm_selfs[cls] = 1;

    for (int s = 1; s < N_SYMS_SUB; s++) {
      mul(sym_cubes[inv_sym[s]], cube1, tmp);
      mul(tmp, sym_cubes[s], cube2);
      Coord cperm1 = getCPerm(cube2);
      if (cperm_sym[cperm1] == EMPTY2)
        cperm_sym[cperm1] = SYMCOORD(cls, s);
      else if (cperm1 == cperm)
        cperm_selfs[cls] |= 1 << s;
    }
    cls++;
  }
}
