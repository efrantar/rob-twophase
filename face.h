/*
 * Face level:
 * The cube is represented by a 54-character string describing the colors at the corresponding facelet positions.
 * This representation is only used to more conveniently interface with the outside world.
 */

/*
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
 * A facelet string is simply lists the colors of every facelet position with the faces being in order U, R, F, D, L, B
 * and the facelets within a face sorted by their index, i.e. U1U2U3U4U5U6U7U8U9R1R2... where U1, U2, ... are the
 * colors of the corresponding facelets.
 *
 * Note that facelet X5 (i.e. the center sticker of face X) must always be of color X.
 */

#ifndef FACE_H_
#define FACE_H_

#include <string>
#include <unordered_map>
#include "cubie.h"

#define N_COLORS 6 // number of colors/faces of a cube
#define N_FACELETS 54 // number of facelets (stickers) = 9 * 6

/* Color/Face ordering */
#define U 0
#define R 1
#define F 2
#define D 3
#define L 4
#define B 5

// Maps color ID to corresponding character
const char kColorNames[] = {'U', 'R', 'F', 'D', 'L', 'B'};

// Maps color character to corresponding color ID
const std::unordered_map<char, int> kNameToColor = {
  {'U', U}, {'R', R}, {'F', F}, {'D', D}, {'L', L}, {'B', B}
};

/* Map corner/edge IDs to corresponding facelet positions */
const int kCornlets[][3] = {
  {8, 9, 20}, {6, 18, 38}, {0, 36, 47}, {2, 45, 11},
  {29, 26, 15}, {27, 44, 24}, {33, 53, 42}, {35, 17, 51}
};
const int kEdgelets[][2] = {
  {5, 10}, {7, 19}, {3, 37}, {1, 46}, {32, 16}, {28, 25},
  {30, 43}, {34, 52}, {23, 12}, {21, 41}, {50, 39}, {48, 14}
};

/* Routines for converting a facelet-string to a CubieCube and vice-versa */
int faceToCubie(const std::string &s, CubieCube &cube);
std::string cubieToFace(const CubieCube &cube);

// Initializes the face-level; to be called before accessing anything from this file
void initFace();

#endif
