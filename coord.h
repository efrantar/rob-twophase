#ifndef COORD_H_
#define COORD_H_

#include <stdint.h>
#include "cubie.h"
#include "moves.h"

#define N_FLIP 2048
#define N_TWIST 2187
#define N_SLICE 495
#define N_SSLICE2 24
#define N_UDEDGES2 40320
#define N_CPERM 40320
#define N_FSLICE 1013760

#define FSLICE(flip, slice) (N_FLIP * slice + flip)
#define FS_FLIP(fslice) (fslice % N_FLIP)
#define FS_SLICE(fslice) (fslice / N_FLIP)

typedef struct Edges4 {
  int comb;
  int perm;

  Edges4() : comb(0), perm(0) {};
  Edges4(int comb1, int perm1) : comb(comb1), perm(perm1) {};
  Edges4(int edges4);

  void set(int edges4);
  int val() const;
} Edges4;

typedef struct CPerm {
  int comb1;
  int perm1;
  int comb2;
  int perm2;

  CPerm() : comb1(0), perm1(0), comb2(0), perm2(0) {}
  CPerm(int cperm);

  void set(int cperm);
  int val() const;
} CPerm;

extern uint16_t (*move_flip)[N_MOVES];
extern uint16_t (*move_twist)[N_MOVES];

int getTwist(const CubieCube &cube);
int getFlip(const CubieCube &cube);
int getSSlice(const CubieCube &cube);
int getUEdges(const CubieCube &cube);
int getDEdges(const CubieCube &cube);
int getUDEdges2(const CubieCube &cube);
int getCPerm(const CubieCube &cube);
int getFSlice(const CubieCube &cube);

void setFlip(CubieCube &cube, int flip);
void setTwist(CubieCube &cube, int twist);
void setSlice(CubieCube &cube, int slice);
void setUDEdges2(CubieCube &cube, int udedges);
void setCPerm(CubieCube &cube, int cperm);

void moveSSlice(const Edges4 &sslice, int move, Edges4 &sslice1);
void moveEdges4(const Edges4 &edges4, int move, Edges4 &moved);
void moveCPerm(const CPerm &cperm, int move, CPerm &cperm1);

int mergeUDEdges2(const Edges4 &uedges, const Edges4 &dedges);
void splitUDEdges2(int udedges, Edges4 &uedges, Edges4 &dedges);

void initCoord();
void initCoordTables();

#endif
