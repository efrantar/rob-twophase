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
 * The final important thing defined in this class are the move-masks. During search we do not want to explore
 * maneuvers that we know right away cannot be optimal, for example ones that contain consecutive moves on the same
 * face. Therefore we define for every move a bit-mask indicating allowed follow-up moves. Furthermore, we also
 * specify masks marking all the phase 1 and the phase 2 moves (the former is necessary for the 5-face mode).
 */

#ifndef MOVES_H_
#define MOVES_H_

#include <stdint.h>
#include <string>
#include "cubie.h"

#ifdef QUARTER
  #ifdef AXIAL
    #define N_MOVES 30 // total number of moves
    #define N_DOUBLE2 6 // number of double moves in phase 2 (needed for the quarter-turn metric)
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
// number of phase 1 moves; only differs from `N_MOVES` when `QUARTER` is defined
#define N_MOVES1 (N_MOVES - N_DOUBLE2)

// Produces a move-mask where exactly the given bit is on
#define MOVEBIT(m) (MoveMask(1) << (m))

typedef uint64_t MoveMask; // since we never store a large number of move-masks, we simply use 64 bits in all modes

extern CubieCube move_cubes[N_MOVES]; // `CubieCube`s representing the moves
extern std::string move_names[N_MOVES]; // names of the moves
extern int inv_move[N_MOVES]; // index of the inverse move

extern MoveMask next_moves[N_MOVES]; // move-masks for permitted successor moves
extern MoveMask phase1_moves; // move-mask with all phase 1 moves
extern MoveMask phase2_moves; // move-mask with all phase 2 moves

/* Basic face moves in their `CubieCube` representation */
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

// Initializes all move-related tables; to be called before accessing anything from this file
void initMoves();

#endif
