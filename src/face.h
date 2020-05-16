/**
 * Since `cubie::cube`s (especially the orientation part) are quite tricky to deal with, we use a more convenient
 * representation to interface with the outside world, the face-cube. Having defined an ordering over all 54 stickers
 * on the physical cube, a list of the colors for each sticker (in terms of the faces U, R, F, D, L and B not the
 * actual cube colors) uniquely specifies any cube-state.
 *
 * The facelet positions are defined as shown in the folded-up Rubik's cube depicted below.
 *
 *          +--+-----+
 *          |U1|U2|U3|
 *          |--+--+--|
 *          |U4|U5|U6|
 *          |--+--+--|
 *          |U7|U8|U9|
 * +--+--+--+--+--+--+--+--+--+--+--+--+
 * |L1|L2|L3|F1|F2|F3|R1|R2|R3|B1|B2|B3|
 * |--+--+--|--+--+--|--+--+--|--+--+--|
 * |L4|L5|L6|F4|F5|F6|R4|R5|R6|B4|B5|B6|
 * |--+--+--|--+--+--|--+--+--|--+--+--|
 * |L7|L8|L9|F7|F8|F9|R7|R8|R9|B7|B8|B9|
 * +--+--+--+--+--+--+--+--+--+--+--+--+
 *          |D1|D2|D3|
 *          |--+--+--|
 *          |D4|D5|D6|
 *          |--+--+--|
 *          |D7|D8|D9|
 *          +--+--+--+
 *
 * A facelet string simply lists the colors of every facelet position with the faces being in order U, R, F, D, L, B
 * and the facelets within a face sorted by their index, i.e. U1U2U3U4U5U6U7U8U9R1R2... where U1, U2, ... are the
 * colors of the corresponding facelets.
 *
 * Note that facelet X5 (i.e. the center sticker of face X) must always be of color X. It also does not matter which
 * of the actual cube colors (like red, orange, etc.) is assigned to which face, the assignment must only be consistent
 * with respect to the neighborhood relations, i.e. for example if white is considered as the F-face, then yellow must
 * be B (as it is always on the opposite side on a physical cube).
 */

#ifndef __FACE__
#define __FACE__

#include <string>
#include <unordered_map>
#include "cubie.h"

namespace face {

  const int N_FACELETS = 54; // number of facelets (stickers) = 9 * 6

  namespace color {
    const int COUNT = 6; // number of colors/faces of a cube

    /* Color/Face ordering */
    const int U = 0;
    const int R = 1;
    const int F = 2;
    const int D = 3;
    const int L = 4;
    const int B = 5;

    const char NAMES[] = {'U', 'R', 'F', 'D', 'L', 'B'};

    // Maps color character to corresponding color ID
    const std::unordered_map<char, int> FROM_NAME = {
      {'U', U}, {'R', R}, {'F', F}, {'D', D}, {'L', L}, {'B', B}
    };
  }

  /* Map corner/edge IDs to corresponding facelet positions */
  const int CORNLETS[][3] = {
    {8, 9, 20}, {6, 18, 38}, {0, 36, 47}, {2, 45, 11},
    {29, 26, 15}, {27, 44, 24}, {33, 53, 42}, {35, 17, 51}
  };
  const int EDGELETS[][2] = {
    {5, 10}, {7, 19}, {3, 37}, {1, 46}, {32, 16}, {28, 25},
    {30, 43}, {34, 52}, {23, 12}, {21, 41}, {50, 39}, {48, 14}
  };

  /* Routines for converting a facelet-string to a cubie-cube and vice-versa */
  int to_cubie(const std::string& s, cubie::cube &c);
  std::string from_cubie(const cubie::cube &c);

  // Initializes the face-level; to be called before accessing anything from this file
  void init();

}

#endif
