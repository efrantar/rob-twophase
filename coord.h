#ifndef COORD_H_
#define COORD_H_

#include <stdint.h>
#include "cubie.h"
#include "moves.h"

#define N_TWIST_COORDS 2187
#define N_FLIP_COORDS 2048
#define N_SLICE_COORDS 11880
#define N_SLICE_COORDS_P2 24

#define N_UEDGES_COORDS 11880
#define N_DEDGES_COORDS 11880
#define N_UDEDGES_COORDS_P2 40320

#define N_CORNERS_COORDS 40320
#define N_EDGES_COORDS 4790001600

typedef uint16_t coord;

extern coord twist_move[N_TWIST_COORDS][N_MOVES];
extern coord flip_move[N_FLIP_COORDS][N_MOVES];
extern coord slice_move[N_SLICE_COORDS][N_MOVES];
extern coord uedges_move[N_UEDGES_COORDS][N_MOVES];
extern coord dedges_move[N_DEDGES_COORDS][N_MOVES];
extern coord udedges_move[N_UDEDGES_COORDS_P2][N_MOVES];
extern coord corners_move[N_CORNERS_COORDS][N_MOVES];

extern coord merge_uedges_dedges[N_UEDGES_COORDS][N_DEDGES_COORDS];

coord getTwist(const CubieCube &cube);
coord getFlip(const CubieCube &cube);
coord getSlice(const CubieCube &cube);
coord getUEdges(const CubieCube &cube);
coord getDEdges(const CubieCube &cube);
coord getUDEdges(const CubieCube &cube);
coord getCorners(const CubieCube &cube);

void setTwist(CubieCube &cube, coord twist);
void setFlip(CubieCube &cube, coord flip);
void setSlice(CubieCube &cube, coord slice);
void setUEdges(CubieCube &cube, coord uedges);
void setDEdges(CubieCube &cube, coord dedges);
void setUDEdges(CubieCube &cube, coord udedges);
void setCorners(CubieCube &cube, coord corners);
void setEdges(CubieCube &cube, coord edges);

void initTwistMove();
void initFlipMove();
void initSliceMove();
void initUEdgesMove();
void initDEdgesMove();
void initUDEdgesMove();
void initCornersMove();

void initMergeUEdgesDEdges();

#endif

