/**
 * Coordinate level:
 *
 * The actual solver only represents the cube by a few numbers, which not only allows easy indexing into pruning tables
 * but also enables extremly fast manipulation of precalculated move and symmetry tables.
 *
 * This particular implementation uses the following primitive coordinates:
 * - TWIST: corner orientation
 * - FLIP: edge orientation
 * - SSLICE: position as well as permutation of the 4 edges in the UD-slice (i.e. FR, FL, BL, BR)
 * - UEDGES: position and location of the 4 edges of the U-face
 * - DEDGES: position and location of the 4 edges of the D-face
 * - UDEDGES: permutation of all but the UD-slice in phase 2
 * - CPERM: permutation of the 8 corners of the cube
 *
 * Furthermore, we define some additional coordinates (mostly for convenience reasons):
 * - SLICE: only the position of the 4 UD-slice edges
 * - FSLICE: a combination of FLIP and SLICE
 */

#ifndef COORD_H_
#define COORD_H_

#include <stdint.h>
#include "cubie.h"
#include "moves.h"

/* Number of different values for every coordinate */

#define N_EDGES4 11880 // binomial(12, 4) * 4!; for all coordinates encoding position and permutation of 4 edges

#define N_TWIST 2187 // 3^(8 - 1)
#define N_FLIP 2048 // 2^(12 - 1)
#define N_SSLICE N_EDGES4 // binomial(12, 4) * 4!
#define N_SSLICE2 24 // 4!; orientation part is 0 in phase 2, hence only the permutation remains to be solved

#define N_UEDGES N_EDGES4
#define N_DEDGES N_EDGES4
#define N_UDEDGES2 40320 // 8! (as the position of the edges is fixed in phase 2)
#define N_CPERM 40320 // 8!

#define N_SLICE 495 // binomial(12, 4); SSLICE without the permutation part
#define N_FSLICE (N_FLIP * N_SLICE)

// Macro for merging UEDGES and DEDGES to UDEDGES
#define UDEDGES(uedges, dedges) (N_SSLICE2 * uedges + (dedges % N_SSLICE2))

// Create a FSLICE coordinate from a FLIP and a SLICE; note that N_FLIP is a power of 2 and hence */% very fast
#define FSLICE(flip, slice) (CCoord(slice) * N_FLIP + flip)
/* Macros for extracting individual components from an FSLICE */
#define FS_FLIP(fslice) (fslice % N_FLIP)
#define FS_SLICE(fslice) (fslice / N_FLIP)

/* Macros for conveniently converting between SLICE and SSLICE coordinates */
#define SSLICE(slice) (slice * N_SSLICE2)
#define SS_SLICE(sslice) (sslice / N_SSLICE2)

/* Coordinate data-type definitions */
typedef uint16_t Coord; // we will create huge tables of this type
typedef uint32_t CCoord; // for combined coords (primarily indexing into pruning tables)

/* Move-table definitions */
extern Coord (*twist_move)[N_MOVES];
extern Coord (*flip_move)[N_MOVES];
extern Coord (*sslice_move)[N_MOVES];
extern Coord (*edges4_move)[N_MOVES]; // the table for UEDGES and DEDGES is exactly the same
extern Coord (*udedges_move2)[N_MOVES2]; // UDEDGES only used in phase 2
extern Coord (*cperm_move)[N_MOVES];

/* Methods for computing the coordinates of a CubieCube */
Coord getTwist(const CubieCube &cube);
Coord getFlip(const CubieCube &cube);
Coord getSSlice(const CubieCube &cube);
Coord getUEdges(const CubieCube &cube);
Coord getDEdges(const CubieCube &cube);
Coord getUDEdges(const CubieCube &cube);
Coord getCPerm(const CubieCube &cube);

/* Methods for creating a CubieCube with a certain coordinate */
void setTwist(CubieCube &cube, Coord twist);
void setFlip(CubieCube &cube, Coord flip);
void setSSlice(CubieCube &cube, Coord sslice);
void setUEdges(CubieCube &cube, Coord uedges);
void setDEdges(CubieCube &cube, Coord dedges);
void setUDEdges(CubieCube &cube, Coord udedges);
void setCPerm(CubieCube &cube, Coord cperm);

// Initializes the coord level, to be called before any other method of this file
void initCoord();
// Generates all move-tables
void initCoordMove();

// Simple convenience function for performing SLICE moves (since we only have move-table for SSLICE)
Coord sliceMove(Coord slice, int move);

#endif
