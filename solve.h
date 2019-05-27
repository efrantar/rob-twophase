/**
 * Actual solvers; twophase and optimal, public interfaces
 */

#ifndef SOLVE_H_
#define SOLVE_H_

#include <string>
#include <vector>
#include "coord.h"
#include "moves.h"

#define N 31 // a twophase solver will never find a solution with more than 30 moves
#define FILE_TWOPHASE "twophase.tbl"
#define FILE_OPTIM "optim.tbl"

// Class as we want to search in parallel from multiple starting positions and every search needs its own variables
class TwoPhaseSolver {
  
  private:
    int rot;
    bool inv;

    Coord flip[N];
    Coord sslice[N];
    Coord twist[N];

    Coord uedges[N];
    Coord dedges[N];
    Coord corners[N];
    Coord udedges[N];

    // Keep track how far CORNERS and UEDGES/DEDGES have already been restored during earlier phase 1 solutions
    int corners_depth;
    int udedges_depth; // individual variable as we only restore UDEDGES if the precheck did not fail
    int moves[N];

    int phase1(int depth, int dist, int togo);
    void phase2(int depth, int dist, int togo);

  public:
    TwoPhaseSolver(int rot, bool inv);
    void solve(const CubieCube &cube);

};
// This is not necessary for the optimal solver, hence there is no class for it

/* Lower level API */
int twophase(const CubieCube &cube, int max_depth, int timelimit, std::vector<int> &sol);
int optim(const CubieCube &cube, int max_depth, int timelimit, std::vector<int> &sol);
std::vector<int> scramble(int timelimit);

/**
 * Initializes the twophase solver by generating / loading all move and pruning tables. Note that this may take
 * a few seconds (usually around 10)
 * @param file whether or not to load/store them in a file (named "twophase.tbl")
 */
void initTwophase(bool file = true);

/**
 * Initializes the optimal solver by generating / loading all move and pruning tables. Note that this may take
 * a few minutes
 * @param file whether or not to load/store them in a file (named "optim.tbl")
 */
void initOptim(bool file = true);

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

/**
 * Runs the optimal solver on the given cube; initOptim() must be run before calling this method
 *
 * @param cube facecube representation of the cube to be solved
 *
 * @param max_depth the solver searches only up to (including) this depth; no cubes takes more than 20 moves to solve
 *
 * @param timelimit
 * The maximum search time; searching up to depth 15 usually takes a few (hundred) milliseconds, than the search time
 * can increase dramatically up to many minutes or even a few hours for very hard cubes
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
 * - SolveError 1: could not complete the search within the timelimit
 * - SolveError 2: no solution found
 */
std::string optimStr(std::string cube, int max_depth = 20, int timelimit = 3600);

/**
 * Generates a scrambling sequence to a uniformly random cube
 * @param timelimit maximum search time for the underlying twophase solver in milliseconds; larger values lead to
 * shorter scrambling sequences, however the default of 10ms should be sufficient to almost always return ones
 * with less than 20 moves
 * @return the scrambling sequence in standard cube notation (empty if none could be found within the timelimit)
 */
std::string scrambleStr(int timelimit = 10);

#endif
