/**
 * Obviously, one key question when implementing a Rubik's cube solver is how to realize cube moves. While actually
 * carrying out the moves on cubie-level is implemented in "cubie.cc" and the move-lookup tables used throughout the
 * solving process are computed in "coord.cc", several important global definitions are stated in this file.
 * Especially, since we support several different metrics this information it not that trivial to set up and hence
 * it is cleaner to decouple it into its own file.
 *
 * In total, the solver supports, three different solving modes which strongly affect which moves are considered when
 * searching for a solution. In the `QUARTER` metric, 180 degree turns (double moves) are considered twice as expensive
 * as quarter-turns. When using `AXIAL` any two consecutive rotations on parallel faces are treated as a single move.
 * Ultimately, the `FACES5` mode strictly restricts the solver to only turning 5 faces, i.e. a solution will never
 * contain any B-moves. All of these options can be combined arbitrarily.
 *
 * On the lowest level we represent cube-moves by `CubieCube`s. Move `m` can then by applied to a cube `c` simply by
 * multiplying the corresponding `CubieCube` representations (see also "cubie.cc"). We only define the 6 basic
 * axis-moves here and build all other required compound moves (such as double moves on the same axis) during the
 * setup routine. Note that we generally only want to define the moves applicable in the given metric (rather than
 * masking out invalid moves) since the number of moves affects critical loops that are executed millions of times and
 * also the sizes of several tables. This however makes the setup a bit tricky.
 *
 * TODO: discuss move-masks
 */

#ifndef MOVES_H_
#define MOVES_H_

#include <stdint.h>
#include <string>
#include "cubie.h"

#ifdef QUARTER
  #ifdef AXIAL
    #define N_MOVES 30
    #define N_DOUBLE2 6
  #else
    #define N_MOVES 16
    #define N_DOUBLE2 4
  #endif
#else
  #ifdef AXIAL
    #define N_MOVES 45
  #else
    #define N_MOVES 18
  #endif
  #define N_DOUBLE2 0
#endif
#define N_MOVES1 (N_MOVES - N_DOUBLE2)

#define MOVEBIT(m) (MoveMask(1) << m)

typedef uint64_t MoveMask;

extern CubieCube move_cubes[N_MOVES];
extern std::string move_names[N_MOVES];
extern int inv_move[N_MOVES];

extern MoveMask next_moves[N_MOVES];
extern MoveMask phase1_moves;
extern MoveMask phase2_moves;

const CubieCube kUCube = {
  {UBR, URF, UFL, ULB, DFR, DLF, DBL, DRB},
  {UB, UR, UF, UL, DR, DF, DL, DB, FR, FL, BL, BR},
  {}, {}
};
const CubieCube kDCube = {
  {URF, UFL, ULB, UBR, DLF, DBL, DRB, DFR},
  {UR, UF, UL, UB, DF, DL, DB, DR, FR, FL, BL, BR},
  {}, {}
};
const CubieCube kRCube = {
  {DFR, UFL, ULB, URF, DRB, DLF, DBL, UBR},
  {FR, UF, UL, UB, BR, DF, DL, DB, DR, FL, BL, UR},
  {2, 0, 0, 1, 1, 0, 0, 2}, {}
};
const CubieCube kLCube = {
  {URF, ULB, DBL, UBR, DFR, UFL, DLF, DRB},
  {UR, UF, BL, UB, DR, DF, FL, DB, FR, UL, DL, BR},
  {0, 1, 2, 0, 0, 2, 1, 0}, {}
};
const CubieCube kFCube = {
  {UFL, DLF, ULB, UBR, URF, DFR, DBL, DRB},
  {UR, FL, UL, UB, DR, FR, DL, DB, UF, DF, BL, BR},
  {1, 2, 0, 0, 2, 1, 0, 0},
  {0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0}
};
const CubieCube kBCube = {
  {URF, UFL, UBR, DRB, DFR, DLF, ULB, DBL},
  {UR, UF, UL, BR, DR, DF, DL, BL, FR, FL, UB, DB},
  {0, 0, 1, 2, 0, 0, 2, 1},
  {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1}
};

void initMoves();

#endif
