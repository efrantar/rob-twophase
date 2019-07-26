/*
 * Face level; converting between face and cubie representation
 */

/*
 * Facelet layout:
 *
 *              |************|
 *              |*U1**U2**U3*|
 *              |************|
 *              |*U4**U5**U6*|
 *              |************|
 *              |*U7**U8**U9*|
 *              |************|
 * |************|************|************|************|
 * |*L1**L2**L3*|*F1**F2**F3*|*R1**R2**F3*|*B1**B2**B3*|
 * |************|************|************|************|
 * |*L4**L5**L6*|*F4**F5**F6*|*R4**R5**R6*|*B4**B5**B6*|
 * |************|************|************|************|
 * |*L7**L8**L9*|*F7**F8**F9*|*R7**R8**R9*|*B7**B8**B9*|
 * |************|************|************|************|
 *              |************|
 *              |*D1**D2**D3*|
 *              |************|
 *              |*D4**D5**D6*|
 *              |************|
 *              |*D7**D8**D9*|
 *              |************|
 *
 * Face order: U, R, F, D, L, B
 * Facelet String: U1U2U3U4U5U6U7U8U9R1R2... where U1, U2, ... are the colors of the corresponding facelets
 */

#ifndef FACE_H_
#define FACE_H_

#include <string>
#include <unordered_map>
#include "cubie.h"

#define N_COLORS 6
#define N_FACELETS 54

#define U 0
#define R 1
#define F 2
#define D 3
#define L 4
#define B 5

#define SUPERFLIP "UBULURUFURURFRBRDRFUFLFRFDFDFDLDRDBDLULBLFLDLBUBRBLBDB"

const char kColorNames[] = {'U', 'R', 'F', 'D', 'L', 'B'};

const std::unordered_map<char, int> kNameToColor = {
  {'U', U}, {'R', R}, {'F', F}, {'D', D}, {'L', L}, {'B', B}
}; // for decoding input strings

/* Mapping of cperm/edges to corresponding facelets */
const int kCornlets[][3] = {
  {8, 9, 20}, {6, 18, 38}, {0, 36, 47}, {2, 45, 11},
  {29, 26, 15}, {27, 44, 24}, {33, 53, 42}, {35, 17, 51}
};
const int kEdgelets[][2] = {
  {5, 10}, {7, 19}, {3, 37}, {1, 46}, {32, 16}, {28, 25},
  {30, 43}, {34, 52}, {23, 12}, {21, 41}, {50, 39}, {48, 14}
};

int faceToCubie(const std::string &s, CubieCube &cube);
std::string cubieToFace(const CubieCube &cube);

void initFace();

#endif
