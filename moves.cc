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
    for (int i = off; i < off + 3 && i < 18; i++)
      skip_moves[i] = MoveMask(0x7) << off;
    for (int i = off + 3; i < off + 6 && i < 18; i++)
      skip_moves[i] = MoveMask(0x3f) << off;
  }
  for (int m = 0; m < 18; m++)
    skip_moves[i] |= MoveMask(0x1ff) << 18 + 9 * (m / 6);

  for (int off = 18; off < 45; off += 9) {
    for (int i = off; i < off + 9 && i < 45; i++)
      skip_moves[i] = MoveMask(0x1ff) << off;
  }

  #ifdef AXIAL
    for (int off = 0; off < 18; off += 6) {
      for (int i = off; i < off + 3 && i < 18; i++)
        skip_moves[i] |= MoveMask(0x7) << off + 3;
    }
  #endif
  #ifdef QUARTER
    for (int m = 0; m < 45; m++) {
      if (is_quarter[m])
        skip_moves[m] ^= MOVEBIT(m);
    }
    for (int m = 18; m < 45; m++) {
      int tmp1 = (m - 18) / 9;
      int tmp2 = (m - 18) % 9;
      skip_moves[m] ^= MOVEBIT(6 * tmp1 + (tmp2 / 3));
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

  for (int m = 0; m < max; m++) {
    #ifdef QUARTER
      if (!is_quarter[m])
        continue;
    #endif
    index[m] = i++;
  }
  #ifdef QUARTER
    int i1 = i;
    for (int m : {7, 10, 13, 16, 31, 40}) {
      if (m < max)
        index[m] = i1++;
    }
  #endif

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
