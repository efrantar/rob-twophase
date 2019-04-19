#ifndef SOLVE_H_
#define SOLVE_H_

#include <string>
#include <vector>

#include "coord.h"
#include "moves.h"

#define N 100

bool skip_move[N_MOVES][N_MOVES];

class Solver {
  
  private:
    Coord flip[N];
    Coord slicesorted[N];
    Coord twist[N];

    Coord uedges[N];
    Coord dedges[N];
    Coord corners[N];
    Coord udedges[N];

    int max_depth;

    std::vector<int> sol;
    int moves[N];
    bool found;
    int corners_depth;
    int udedges_depth;

    void phase1(int depth, int dist, int limit);
    void phase2(int depth, int dist, int limit);

  public:
    Solver(int max_depth);
    std::string solve(const CubieCube &cube);

};

void initSolve();

#endif

