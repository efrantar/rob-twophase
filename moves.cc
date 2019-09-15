/**
 * This file now actually generates all constant move related tables. While this should generally be quite
 * straight-forward, things are complicated considerably by having to support 3 different solving modes that can be
 * combined arbitrarily and that all the available move-set.
 *
 * We generate and number the moves in a very particular manner. First come all single-faces moves in the face-order
 * U, D, R, L, F, B that are part of the used metric (i.e. normally these are 18, but in the quarter-turn metric
 * only 12). Note that the back-moves are always generated (even if we use the `FACES5` option) and that parallel faces
 * are subsequent in the ordering. This makes an efficient implementation of the extended phase 1 pruning table
 * (see "prun.cc") much easier. If `AXIAL` is defined there now follow all allowed axial moves (which again depends
 * on whether or not the quarter-turn metric is used). Finally, only when `QUARTER` exists, we also add all necessary
 * double moves for phase 2 (potentially also including their axial variants). Doing so makes the actual solver both
 * easier to implement and also a bit more efficient. Everything is defined based on this ordering, moves that are not
 * needed in the used metric do not exist.
 *
 * The move-mask of some move `m` disables the following moves:
 * - All moves on the same faces (standard move) / axis (axial move)
 * - If `m` is a (non-axial) move on face D, L or B, then also all moves on the corresponding parallel face (in the
 *   axial metric, this happens for all simple moves)
 * - All axial moves involving the face of `m` (standard move) / all standard moves on both faces of the axis (axial)
 * Note that in the quarter-turn metric we always allow the same move again (but no other moves on the same face or
 * axis) and we also permit both single face moves `m1` and `m2` after an axial move `(m1 m2)`, otherwise we might
 * lose (good) solutions.
 *
 * The data from this file is used almost everywhere throughout the solver, in particular in "solve.cc" and "prun.cc".
 */

#include "sym.h"

CubieCube move_cubes[N_MOVES];
std::string move_names[N_MOVES];
int inv_move[N_MOVES];

MoveMask next_moves[N_MOVES];
MoveMask phase1_moves;
MoveMask phase2_moves;

/*
 * The following code is neither elegant nor efficient, however it gets the job done well enough. The setup time here
 * is very much negligible (especially compared to the sym-tables) and it sets everything up exactly as we want it.
 */
void initMoves() {
  /*
   * We simply work out the information about the 18 basic moves by hand and hard-code it to avoid making an already
   * relatively complicated function any more complicated.
   */
  std::string move_names1[45] = {
    "U", "U2", "U'", "D", "D2", "D'",
    "R", "R2", "R'", "L", "L2", "L'",
    "F", "F2", "F'", "B", "B2", "B'"
  };
  bool is_quarter[45] = {
    true, false, true, true, false, true,
    true, false, true, true, false, true,
    true, false, true, true, false, true,
    true, false, true, true, false, true
  };
  bool in_phase2[45] = {
    true, true, true, true, true, true,
    false, true, false, false, true, false,
    false, true, false, false, true, false
  };
  bool uses_back[45] = {};
  for (int m = 15; m < 18; m++)
    uses_back[m] = true;

  CubieCube move_cubes1[45];
  move_cubes1[0] = kUCube;
  move_cubes1[3] = kDCube;
  move_cubes1[6] = kRCube;
  move_cubes1[9] = kLCube;
  move_cubes1[12] = kFCube;
  move_cubes1[15] = kBCube;

  for (int i = 0; i < 18; i += 3) {
    mul(move_cubes1[i], move_cubes1[i], move_cubes1[i + 1]);
    mul(move_cubes1[i], move_cubes1[i + 1], move_cubes1[i + 2]);
  }

  /*
   * To make things a lot simpler, we start by always generating all 45 possible moves and only then filtering out
   * those included by the selected metrics.
   */
  int i = 18;
  for (int ax1 = 0; ax1 < 18; ax1 += 6) {
    int ax2 = ax1 + 3;
    for (int pow1 = 0; pow1 < 3; pow1++) {
      for (int pow2 = 0; pow2 < 3; pow2++) {
        move_names1[i] = "(" + move_names1[ax1 + pow1] + " " + move_names1[ax2 + pow2] + ")";
        is_quarter[i] = is_quarter[ax1 + pow1] && is_quarter[ax2 + pow2];
        in_phase2[i] = in_phase2[ax1 + pow1] && in_phase2[ax2 + pow2];
        uses_back[i] = uses_back[ax1 + pow1] || uses_back[ax2 + pow2];
        mul(move_cubes1[ax1 + pow1], move_cubes1[ax2 + pow2], move_cubes1[i]);
        i++;
      }
    }
  }

  MoveMask skip_moves[45];

  for (int off = 0; off < 18; off += 6) {
    // Skip moves on the same face for U, R, F moves
    for (int i = off; i < off + 3 && i < 18; i++)
      skip_moves[i] = MoveMask(0x7) << off;
    // Skip moves same and opposite face for D, L and B moves
    for (int i = off + 3; i < off + 6 && i < 18; i++)
      skip_moves[i] = MoveMask(0x3f) << off;
  }
  // Skip all axial moves involving the face of standard move `m`
  for (int m = 0; m < 18; m++)
    skip_moves[i] |= MoveMask(0x1ff) << 18 + 9 * (m / 6);

  // Skip all moves on the same axis for axial moves
  for (int off = 18; off < 45; off += 9) {
    for (int i = off; i < off + 9 && i < 45; i++)
      skip_moves[i] = MoveMask(0x1ff) << off;
  }
  // As half-slice moves commute, fix an ordering
  skip_moves[40] |= MOVEBIT(31) | MOVEBIT(22);
  skip_moves[31] |= MOVEBIT(22);

  #ifdef AXIAL
    // Always skip all moves on the same axis (normally not for U, R and F faces) in axial mode
    for (int off = 0; off < 18; off += 6) {
      for (int i = off; i < off + 3 && i < 18; i++)
        skip_moves[i] |= MoveMask(0x7) << off + 3;
    }
  #endif
  #ifdef QUARTER
    // Allow repetitions of the same clockwise turns
    for (int m : {0, 3, 6, 9, 12, 15, 18, 27, 36})
      skip_moves[m] ^= MOVEBIT(m);
    // Allow clockwise standard parts of an axial move to be repeated directly afterwards
    for (int m = 18; m < 45; m++) {
      int tmp1 = (m - 18) / 9;
      int tmp2 = (m - 18) % 9;
      if (tmp2 / 3 == 0)
        skip_moves[m] ^= MOVEBIT(6 * tmp1 + (tmp2 / 3));
      if (tmp2 % 3 == 0)
        skip_moves[m] ^= MOVEBIT(6 * tmp1 + 3 + tmp2 % 3);
    }
  #endif

  int index[45];
  std::fill(index, index + 45, -1);
  i = 0;

  int max = 18;
  #ifdef AXIAL
      max = 45;
  #endif

  /* Filter out all moves that do not exist in the configured metric by assigning new indices */
  for (int m = 0; m < max; m++) {
    #ifdef QUARTER
      if (!is_quarter[m])
        continue;
    #endif
    index[m] = i++;
  }
  #ifdef QUARTER
    // Add phase 2 double moves in quarter-turn mode
    int i1 = i;
    for (int m : {7, 10, 13, 16, 31, 40}) {
      if (m < max)
        index[m] = i1++;
    }
  #endif

  /* Now convert everything to using those new indices */
  for (int m = 0; m < max; m++) {
    if (index[m] != -1) {
      move_names[index[m]] = move_names1[m];
      move_cubes[index[m]] = move_cubes1[m];

      next_moves[index[m]] = 0;
      for (int m1 = 0; m1 < max; m1++) {
        if (index[m1] != -1 && (skip_moves[m] & MOVEBIT(m1)) == 0)
          next_moves[index[m]] |= MOVEBIT(index[m1]);
      }
    }
  }

  CubieCube tmp;
  for (int m1 = 0; m1 < N_MOVES; m1++) {
    inv(move_cubes[m1], tmp);
    // Slow but a very simple way to find the inverses without any tricky calculation that depends on the used metric
    for (int m2 = 0; m2 < N_MOVES; m2++) {
      if (move_cubes[m2] == tmp)
        inv_move[m1] = m2;
    }
  }

  phase1_moves = 0;
  phase2_moves = 0;

  for (int m = 0; m < max; m++) {
    if (index[m] != -1) {
      #ifdef FACES5
        if (uses_back[m])
          continue;
      #endif
      if (index[m] < i)
        phase1_moves |= MOVEBIT(index[m]);
      if (in_phase2[m])
        phase2_moves |= MOVEBIT(index[m]);
    }
  }
}
