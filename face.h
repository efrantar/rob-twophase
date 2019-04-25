#ifndef FACE_H_
#define FACE_H_

#include <string>
#include "cubie.h"

#define N_COLORS 6
#define N_FACELETS 54

#define U 0
#define R 1
#define F 2
#define D 3
#define L 4
#define B 5

const char kColorNames[] = {'U', 'R', 'F', 'D', 'L', 'B'};

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
std::string fromScramble(const std::string &scramble);

#endif
