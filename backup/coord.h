/**
 * The heart of the twophase-algorithm is the encoding of different cube properties by coordinates, plain integer
 * numbers. These are not only required to index easily index into pruning tables but they also enable extremely
 * fast cube manipulation via lookup-tables. This files contains functionality for extracting coordinates from
 * `CubieCube`s as well as for generating the mentioned move-tables.
 *
 * The solver uses the following set of coordinates (+ some extended pruning indices defined in "prun.h"):
 * - FLIP: orientation of the 12 edges; note that parity must always be 0
 * - TWIST: orientation of the 8 corners; note that parity must always be 0
 * - SLICE: position (combination) of the 4 UD-slice edges FR, FL, BR and BL
 * - FSLICE: joined version of FLIP and SLICE
 * - SSLICE: combination and permutation of the 4 UD-slice edges
 * - UEDGES: combination and permutation of the 4 edges on the U-face: UR, UF, UL and UB
 * - DEDGES: combination and permutation of the 4 edges on the D-face: DR, DF, DL and DB
 * - UDEDGES2: permutation of the 8 U- and D-edges in phase 2; note that the UD-slice are always in their correct
 *   positions in phase 2, hence this coordinate is only an 8-element permutation
 * - CPERM: permutation of the 8 corners
 *
 * Since we want to avoid any expensive back and forth conversion between `CubieCube`s and coordinates during the
 * solving procedure, we precompute so called `move_*` tables which simply give coordinate `c1` resulting from
 * applying move `m` to a some cube with coordinate `c`. It is important to mention here that most of the coordinates
 * can indeed be handled separately.
 *
 * Even though coordinates are actually just simple numbers, the majority of them are actually handled via small
 * structs. This is because the larger coordinates are actually split-up into sub-coordinates which can take on
 * much fewer different values resulting in smaller, more cache-friendly, move-tables. This is also the reason why
 * this file provides move-functions for those types rather than explicit tables.
 */

#ifndef COORD_H_
#define COORD_H_

#include <stdint.h>
#include "cubie.h"
#include "moves.h"

#define N_FLIP 2048 // 2^(12 - 1); as 12th orientation given by parity
#define N_TWIST 2187 // 3^(8 - 1); as 8th orientation given by parity
#define N_SLICE 495 // binom(12, 4)
#define N_SSLICE2 24 // 4!
#define N_UDEDGES2 40320 // 8!; as SSLICE fixed to the correct slots in phase 2
#define N_CPERM 40320 // 8!
#define N_FSLICE 1013760 // N_FLIP * N_SLICE

// Macros for easily handling FSLICE coordinates
#define FSLICE(flip, slice) (N_FLIP * slice + flip)
#define FS_FLIP(fslice) (fslice % N_FLIP)
#define FS_SLICE(fslice) (fslice / N_FLIP)

// Struct used to represent position and permutation of 4 edges; used for SSLICE, UEDGES and DEDGES
typedef struct Edges4 {
  int comb; // "position" (combination)
  int perm; // permutation

  Edges4() : comb(0), perm(0) {};
  Edges4(int comb1, int perm1) : comb(comb1), perm(perm1) {};
  Edges4(int edges4);

  // Makes table-calculation more convenient and allows reusing variables for temporary results
  void set(int edges4);
  int val() const;
} Edges4;

// Corner coordinate
typedef struct CPerm {
  int comb1; // location of corners 0 - 3
  int perm1; // permutation of corners 0 - 3
  int comb2; // location of corners 0 - 3 (actually redundant, but moves easier to handle when present)
  int perm2; // permutation of corners 0 - 3

  CPerm() : comb1(0), perm1(0), comb2(0), perm2(0) {}
  CPerm(int cperm);

  void set(int cperm);
  int val() const;
} CPerm;

// Move-tables for FLIP and TWIST
extern uint16_t (*move_flip)[N_MOVES];
extern uint16_t (*move_twist)[N_MOVES];

// Functions for computing coordinates of a given `CubieCube`
int getTwist(const CubieCube &cube);
int getFlip(const CubieCube &cube);
int getSSlice(const CubieCube &cube);
int getUEdges(const CubieCube &cube);
int getDEdges(const CubieCube &cube);
int getUDEdges2(const CubieCube &cube);
int getCPerm(const CubieCube &cube);
int getFSlice(const CubieCube &cube);

// Functions for decoding coordinates actual cubes
void setFlip(CubieCube &cube, int flip);
void setTwist(CubieCube &cube, int twist);
void setSlice(CubieCube &cube, int slice);
void setUDEdges2(CubieCube &cube, int udedges);
void setCPerm(CubieCube &cube, int cperm);

// Functions for performing moves on complex coordinates
void moveSSlice(const Edges4 &sslice, int move, Edges4 &sslice1);
void moveEdges4(const Edges4 &edges4, int move, Edges4 &moved);
void moveCPerm(const CPerm &cperm, int move, CPerm &cperm1);

// Functions for handling UDEDGES
int mergeUDEdges2(const Edges4 &uedges, const Edges4 &dedges);
void splitUDEdges2(int udedges, Edges4 &uedges, Edges4 &dedges);

// Sets up the basic tables; to be called before doing anything else with this file
void initCoord();
// Initializes all move-tables
void initCoordTables();

#endif
