#ifndef COORD_H_
#define COORD_H_

#include <stdint.h>
#include "cubie.h"
#include "moves.h"

#define N_TWIST 2187
#define N_FLIP 2048
#define N_SSLICE 11880
#define N_SSLICE2 24

#define N_UEDGES 11880
#define N_DEDGES 11880
#define N_4EDGES2 1680
#define N_UDEDGES2 40320
#define N_CORNERS_C 40320

#define N_SLICE 495
#define N_FSLICE (N_FLIP * N_SLICE)

#define N_EDGES_C 4790001600

#define FSLICE(flip, slice) (CoordL(slice) * N_FLIP + CoordL(flip))
#define FS_FLIP(fslice) (fslice % N_FLIP)
#define FS_SLICE(fslice) (fslice / N_FLIP)

#define SSLICE(slice) (slice * N_SSLICE2)
#define SS_SLICE(sslice) (sslice / N_SSLICE2)

#define FSSLICE(flip, sslice) (CoordL(flip) * N_FLIP + CoordL(sslice))
#define FSS_FLIP(fsslice) (fsslice % N_FLIP)
#define FSS_SSLICE(fsslice) (fsslice % N_FLIP)

typedef uint16_t Coord;
typedef uint32_t CoordL;
typedef uint64_t CoordLL;

extern Coord (*twist_move)[N_MOVES];
extern Coord (*flip_move)[N_MOVES];
extern Coord (*sslice_move)[N_MOVES];
extern Coord (*uedges_move)[N_MOVES];
extern Coord (*dedges_move)[N_MOVES];
extern Coord (*udedges_move)[N_MOVES2];
extern Coord (*corners_move)[N_MOVES];

extern Coord (*merge_udedges)[N_SSLICE2];

Coord getTwist(CubieCube &cube);
Coord getFlip(CubieCube &cube);
Coord getSSlice(CubieCube &cube);
Coord getUEdges(CubieCube &cube);
Coord getDEdges(CubieCube &cube);
Coord getUDEdges(CubieCube &cube);
Coord getSlice(CubieCube &cube);
Coord getCorners(CubieCube &cube);

void setTwist(CubieCube &cube, Coord twist);
void setFlip(CubieCube &cube, Coord flip);
void setSSlice(CubieCube &cube, Coord sslice);
void setUEdges(CubieCube &cube, Coord uedges);
void setDEdges(CubieCube &cube, Coord dedges);
void setUDEdges(CubieCube &cube, Coord udedges);
void setCorners(CubieCube &cube, Coord corners);
void setEdges(CubieCube &cube, CoordLL edges);
void setSlice(CubieCube &cube, Coord slice);

void initTwistMove();
void initFlipMove();
void initSSliceMove();
void initUEdgesMove();
void initDEdgesMove();
void initUDEdgesMove();
void initCornersMove();

void initMergeUDEdges();

Coord sliceMove(Coord slice, int move);

#endif
