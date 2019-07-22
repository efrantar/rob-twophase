/**
 * Actual solvers; twophase and optimal, public interfaces
 */

#ifndef SOLVE_H_
#define SOLVE_H_

#include <string>
#include <vector>
#include "coord.h"

#define N 50 // longer solutions will not be found in any metric

#define FILE_TWOPHASE "twophase.tbl"

// Class as we want to search in parallel from multiple starting positions and every search needs its own variables
class TwoPhaseSolver {
  
  private:
    int rot;
    bool inv_; // to work around shadowing `inv()`

    Coord flip[N];
    Coord sslice[N];
    Coord twist[N];

    Coord uedges[N];
    Coord dedges[N];
    Coord cperm[N];
    Coord udedges[N];

    // Keep track how far CPERM and UEDGES/DEDGES have already been restored during earlier phase 1 solutions
    int cperm_depth;
    int udedges_depth; // individual variable as we only restore UDEDGES if the precheck did not fail
    int moves[N];

    void phase1(int depth, int dist, int togo);
    int phase2(int depth, int togo);

  public:
    TwoPhaseSolver(int rot, bool inv);
    void solve(const CubieCube &cube);

};

// Low level API
int twophase(const CubieCube &cube, int max_depth, int timelimit, std::vector<int> &sol);

/**
 * Initializes the twophase solver by generating / loading all move and pruning tables. Note that this may take
 * a few seconds (usually around 10)
 * @param file whether or not to load/store them in a file (named "twophase.tbl")
 */
void initTwophase(bool file = true);

/* Higher level API that masks all solver internal structures */

/**
 * Runs the twophase solver on the given cube; initTwophase() must be run before calling this method
 *
 * @param cube facecube representation of the cube to be solved
 *
 * @param max_depth
 * The solver terminates once a solution of at most this depth is found; the solver will inherently
 * not find any solutions of depth larger than 30; uniformly random cubes can usually be solved in 20 moves or less
 * within 1 millisecond; a value of -1 makes the solver search for better solutions until a timeout occurs
 *
 * @param timelimit
 * The maximum search time in milliseconds; while 10ms is usually enough to solve almost all cubes in 20 (or at least
 * in 21) moves, it might take a bit longer for some very hard cubes.
 *
 * @return
 * The solution in standard cube move notation or on of the following errors:
 * - FaceError 1: invalid color
 * - FaceError 2: invalid color of center pieces
 * - FaceError 3: invalid corner cubie
 * - FaceError 4: invalid edge cubie
 * - CubieError 3: invalid corner twist
 * - CubieError 4: missing corner
 * - CubieError 7: invalid edge flip
 * - CubieError 8: missing edge
 * - CubieError 9: parity error
 * - SolveError 2: no solution found within the timelimit
 */
std::string twophaseStr(std::string cube, int max_depth = -1, int timelimit = 10);

#endif
