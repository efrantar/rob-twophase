/**
 * Coordinate level; move-tables and coordinate en-/decoding
 */

#ifndef COORD_H_
#define COORD_H_

#include <stdint.h>
#include "cubie.h"
#include "moves.h"

#define N_TWIST 2187
#define N_FLIP 2048
#define N_SSLICE 11880
#define N_SSLICE2 24 // ori part is 0 in phase 2; # of 4 cubie permutations

#define N_UEDGES 11880
#define N_DEDGES 11880
#define N_4EDGES2 1680 // only 8 possible positions for UEDGES and DEDGES in phase 2
#define N_UDEDGES2 40320
#define N_CORNERS_C 40320 // _C to avoid a name clash with N_CORNERS

#define N_SLICE 495 // SSLICE without perm part
#define N_FSLICE (N_FLIP * N_SLICE)

#define N_EDGES_C 4790001600

// N_FLIP is a power of 2, hence *%/ is much faster
#define FSLICE(flip, slice) (CoordL(slice) * N_FLIP + flip)
#define FS_FLIP(fslice) (fslice % N_FLIP)
#define FS_SLICE(fslice) (fslice / N_FLIP)

#define SSLICE(slice) (slice * N_SSLICE2)
#define SS_SLICE(sslice) (sslice / N_SSLICE2)

#define DEDGES_SOLVED 1656 // only coordinate that is != 0 in solved state

typedef uint16_t Coord;
typedef uint32_t CoordL; // for combined coords
typedef uint64_t CoordLL; // just for EDGES_C

extern Coord (*twist_move)[N_MOVES];
extern Coord (*flip_move)[N_MOVES];
extern Coord (*sslice_move)[N_MOVES];
extern Coord (*uedges_move)[N_MOVES];
extern Coord (*dedges_move)[N_MOVES];
extern Coord (*udedges_move2)[N_MOVES2]; // UDEDGES only used in phase 2
extern Coord (*corners_move)[N_MOVES];

extern Coord (*merge_udedges)[N_SSLICE2]; // with SLICE and UEDGES fixed, only perm part of DEDGES necessary

Coord getTwist(const CubieCube &cube);
Coord getFlip(const CubieCube &cube);
Coord getSSlice(const CubieCube &cube);
Coord getUEdges(const CubieCube &cube);
Coord getDEdges(const CubieCube &cube);
Coord getUDEdges(const CubieCube &cube);
Coord getSlice(const CubieCube &cube); // faster than using SSLICE with 0 perm part
Coord getCorners(const CubieCube &cube);

void setTwist(CubieCube &cube, Coord twist);
void setFlip(CubieCube &cube, Coord flip);
void setSSlice(CubieCube &cube, Coord sslice);
void setUEdges(CubieCube &cube, Coord uedges);
void setDEdges(CubieCube &cube, Coord dedges);
void setUDEdges(CubieCube &cube, Coord udedges);
void setCorners(CubieCube &cube, Coord corners);
void setEdges(CubieCube &cube, CoordLL edges); // EDGES_C only for random generation, not get() necessary
void setSlice(CubieCube &cube, Coord slice);

void initTwistMove();
void initFlipMove();
void initSSliceMove();
void initUEdgesMove();
void initDEdgesMove();
void initUDEdgesMove2();
void initCornersMove();

void initMergeUDEdges();

Coord sliceMove(Coord slice, int move); // convenience as we have no table for SLICE (only SSLICE)

#endif
