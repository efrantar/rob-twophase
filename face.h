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

int faceToCubie(const std::string &s, CubieCube &cubiecube);
std::string cubieToFace(const CubieCube &cubiecube);

#endif

