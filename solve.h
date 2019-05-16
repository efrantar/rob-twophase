#ifndef SOLVE_H_
#define SOLVE_H_

#include <string>
#include <vector>
#include "coord.h"
#include "moves.h"

#define N 100

extern bool skip_move[N_MOVES][N_MOVES];

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

    int corners_depth;
    int udedges_depth;
    int moves[N];

    void phase1(int depth, int dist, int limit);
    void phase2(int depth, int dist, int limit);

  public:
    TwoPhaseSolver(int rot, bool inv);
    void solve(const CubieCube &cube);

};

std::vector<int> twophase(const CubieCube &cube, int max_depth, int timelimit);
void initTwophase();

#endif
