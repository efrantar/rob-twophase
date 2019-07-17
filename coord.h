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
#define N_UDEDGES2 40320
#define N_CPERM 40320

#define N_SLICE 495 // SSLICE without perm part
#define N_FSLICE (N_FLIP * N_SLICE)

#define UDEDGES(uedges, dedges) (N_SSLICE2 * uedges + (dedges % N_SSLICE2))

// N_FLIP is a power of 2, hence *%/ is much faster for it
#define FSLICE(flip, slice) (CCoord(slice) * N_FLIP + flip)
#define FS_FLIP(fslice) (fslice % N_FLIP)
#define FS_SLICE(fslice) (fslice / N_FLIP)

#define SSLICE(slice) (slice * N_SSLICE2)
#define SS_SLICE(sslice) (sslice / N_SSLICE2)

typedef uint16_t Coord;
typedef uint32_t CCoord; // for combined coords

extern Coord (*twist_move)[N_MOVES];
extern Coord (*flip_move)[N_MOVES];
extern Coord (*sslice_move)[N_MOVES];
extern Coord (*uedges_move)[N_MOVES];
extern Coord (*dedges_move)[N_MOVES];
extern Coord (*udedges_move2)[N_MOVES2]; // UDEDGES only used in phase 2
extern Coord (*cperm_move)[N_MOVES];

Coord getTwist(const CubieCube &cube);
Coord getFlip(const CubieCube &cube);
Coord getSSlice(const CubieCube &cube);
Coord getUEdges(const CubieCube &cube);
Coord getDEdges(const CubieCube &cube);
Coord getUDEdges(const CubieCube &cube);
Coord getCPerm(const CubieCube &cube);

void setTwist(CubieCube &cube, Coord twist);
void setFlip(CubieCube &cube, Coord flip);
void setSSlice(CubieCube &cube, Coord sslice);
void setUEdges(CubieCube &cube, Coord uedges);
void setDEdges(CubieCube &cube, Coord dedges);
// Only supposed to be called when the slice is in correct position (i.e. in phase 2)
void setUDEdges(CubieCube &cube, Coord udedges);
void setCPerm(CubieCube &cube, Coord cperm);

void initTwistMove();
void initFlipMove();
void initSSliceMove();
void initUEdgesMove();
void initDEdgesMove();
void initUDEdgesMove2();
void initCPermMove();

Coord sliceMove(Coord slice, int move); // convenience function as we have no table for SLICE (only SSLICE)

#endif
