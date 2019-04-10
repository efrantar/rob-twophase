#ifndef COORD_H_
#define COORD_H_

#include <stdint.h>
#include "cubie.h"
#include "moves.h"

#define N_TWIST_COORDS 2187
#define N_FLIP_COORDS 2048
#define N_SLICESORTED_COORDS 11880
#define N_SLICESORTED_COORDS_P2 24

#define N_UEDGES_COORDS 11880
#define N_UEDGES_COORDS_P2 1680
#define N_DEDGES_COORDS 11880
#define N_DEDGES_COORDS_P2 1680
#define N_UDEDGES_COORDS_P2 40320
#define N_CORNERS_COORDS 40320

#define N_SLICE_COORDS 495
#define N_FLIPSLICE_COORDS (N_FLIP_COORDS * N_SLICE_COORDS)

#define N_EDGES_COORDS 4790001600

#define FLIPSLICE(flip, slice) (LargeCoord(slice) * N_FLIP_COORDS + flip)
#define FS_FLIP(flipslice) (flipslice % N_FLIP_COORDS)
#define FS_SLICE(flipslice) (flipslice / N_FLIP_COORDS)

#define SLICESORTED(slice) (slice * N_SLICESORTED_COORDS_P2)
#define SS_SLICE(slicesorted) (slicesorted / N_SLICESORTED_COORDS_P2)

typedef uint16_t Coord;
typedef int LargeCoord;

extern Coord (*twist_move)[N_MOVES];
extern Coord (*flip_move)[N_MOVES];
extern Coord (*slicesorted_move)[N_MOVES];
extern Coord (*uedges_move)[N_MOVES];
extern Coord (*dedges_move)[N_MOVES];
extern Coord (*udedges_move)[N_MOVES_P2];
extern Coord (*corners_move)[N_MOVES];

extern Coord (*merge_udedges)[24];

Coord getTwist(CubieCube &cube);
Coord getFlip(CubieCube &cube);
Coord getSliceSorted(CubieCube &cube);
Coord getUEdges(CubieCube &cube);
Coord getDEdges(CubieCube &cube);
Coord getUDEdges(CubieCube &cube);
Coord getSlice(CubieCube &cube);
Coord getCorners(CubieCube &cube);

void setTwist(CubieCube &cube, Coord twist);
void setFlip(CubieCube &cube, Coord flip);
void setSliceSorted(CubieCube &cube, Coord slicesorted);
void setUEdges(CubieCube &cube, Coord uedges);
void setDEdges(CubieCube &cube, Coord dedges);
void setUDEdges(CubieCube &cube, Coord udedges);
void setCorners(CubieCube &cube, Coord corners);
void setEdges(CubieCube &cube, Coord edges);
void setSlice(CubieCube &cube, Coord slice);

void initTwistMove();
void initFlipMove();
void initSliceSortedMove();
void initUEdgesMove();
void initDEdgesMove();
void initUDEdgesMove();
void initCornersMove();

void initMergeUDEdges();

Coord sliceMove(Coord slice, int move);

#endif

