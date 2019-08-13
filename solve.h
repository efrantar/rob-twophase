/**
 * This file defines the interfaces of the actual solver
 */

#ifndef SOLVE_H_
#define SOLVE_H_

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "coord.h"

#define N 50 // longer solutions will not be found in any metric

#define FILE_TWOPHASE "twophase.tbl"

/*
 * To find short solutions more quickly we want to perform a parallel search in multiple directions as is achieved by
 * transformed (rotational symmetry and inversion) starting positions. Since each each of those sub-searches needs its
 * own variables, we define a class here. Furthermore, we want to support an arbitrary number of threads. Therefore
 * we do not simply let every thread execute an iterative deepening search but instead distribute tasks of search
 * direction plus particular depth among the available threads, hence the method `solve(int togo)`.
 */
class TwoPhaseSolver {
  
  private:
    int rot;
    bool inv_; // to work around shadowing `inv()`

    int flip;
    int twist;
    Edges4 sslice;
    CPerm cperm; // reconstruction only when phase 1 solved as below not worth it here

    /*
     * We only reconstruct the UEDGES and DEDGES when the phase 2 precheck succeeded. Furthermore, we always keep
     * track and reuse partial reconstructions via `edges_depth` which indicate up to which depth those coordinate
     * are currently correctly initialized.
     */
    Edges4 uedges[N];
    Edges4 dedges[N];
    int edges_depth;

    int moves[N];

    // Passing arguments faster than storing them in arrays; pass by value also slightly faster than by reference
    void phase1(
      int depth, int togo, int flip, int twist, Edges4 sslice, CPerm cperm, MoveMask movemask
    );

    bool phase2(
      int depth, int togo,
      Edges4 sslice, Edges4 uedges, Edges4 dedges, CPerm cperm,
      MoveMask movemask
    );

  public:
    TwoPhaseSolver(int rot, bool inv, const CubieCube &cube);
    int lower_bound(); // minimum number of moves required to solve phase 1 (lower bound for any solution)
    void solve(int togo); // search through all phase 1 solutions with length exactly `togo`
};

/*
 * Since there are several scenarios in which the solving time lies under a single millisecond, things like thread
 * startup delay cannot just be neglected anymore. `prepareSolve()` fully readies all solving threads. As `solve()`
 * return without waiting for all threads to fully `shutdown()` we need `waitForFinish()` to ensure clean program
 * shutdowns.
 */
void prepareSolve(int n_threads);
int solve(const CubieCube &cube, int max_depth, int timelimit, std::vector<int> &sol);
void waitForFinish();

/**
 * Initializes the twophase solver by generating all move- and symmetry-tables as well as generating or loading from
 * disk the big pruning tables (the latter are by default written out to disk in case they had to be generated). This
 * takes considerate computational effort and may take up to several minutes depending on the computer hardware and
 * the selected solving mode (in particular `5FACES` makes it take ~4 times longer).
 * @param file whether or not to load/store them in a file (named "twophase.tbl")
 */
void initTwophase(bool file = true);

/**
 * Runs the twophase solver on the given cube; `initTwophase()` must be run before calling this method
 *
 * @param cube facecube representation of the cube to be solved
 *
 * @param max_depth
 * The solver terminates once a solution of at most this depth is found; a value of -1 makes the solver search for
 * better solutions until a timeout occurs. Note that in this case some solution is always found within < 0.1ms whereas
 * some rare cubes might take several 100ms to be solved within tight move bounds.
 *
 * @param timelimit
 * The maximum search time in milliseconds; 10ms is usually good enough to find very good solutions (i.e. < 1 move
 * away from the optimum on average in the standard mode, probably slightly more in other metrics) on decent hardware.
 *
 * @param prepare
 * Whether new solver threads should be set up; manually call `prepareSolve()` and only afterwards this function with
 * `prepare = false` if you want to optimize away thread startup times.
 *
 * @param wait
 * Whether or not to wait for all solver threads to shutdown; the primary purpose is again to potentially optimize
 * away shutdown times if necessary.
 *
 * @param n_threads number of solver threads (note that scaling is not necessarily linear once this number gets large
 * even if your processor has many cores as the memory access speed might become a bottleneck)
 *
 * @return
 * The solution in standard cube move notation (axial moves are given in brackets) or one of the following errors:
 * - FaceError 1: invalid color
 * - FaceError 2: invalid color of center pieces
 * - FaceError 3: invalid corner cubie
 * - FaceError 4: invalid edge cubie
 * - CubieError 3: invalid corner twist
 * - CubieError 4: missing corner
 * - CubieError 7: invalid edge flip
 * - CubieError 8: missing edge
 * - CubieError 9: parity error
 * - SolveError 2: no solution of the requested length found (or no solution at all found if `max_depth = -1`)
 */
std::string twophase(
  std::string cube, int max_depth = -1, int timelimit = 10,
  bool prepare = true, bool wait = true, int n_threads = 1
);

#endif
