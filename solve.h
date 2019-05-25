#ifndef SOLVE_H_
#define SOLVE_H_

#include <string>
#include <vector>
#include "coord.h"
#include "moves.h"

#define N 31
#define FILE_TWOPHASE "twophase.tbl"
#define FILE_OPTIM "/home/elias/projects/twophase/optim.tbl"

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

    int phase1(int depth, int dist, int togo);
    void phase2(int depth, int dist, int togo);

  public:
    TwoPhaseSolver(int rot, bool inv);
    void solve(const CubieCube &cube);

};

int twophase(const CubieCube &cube, int max_depth, int timelimit, std::vector<int> &sol);
int optim(const CubieCube &cube, int max_depth, int timelimit, std::vector<int> &sol);
std::vector<int> scramble(int timelimit);
void initTwophase(bool file = true);
void initOptim(bool file = true);

std::string twophaseStr(std::string cube, int max_depth = -1, int timelimit = 10);
std::string optimStr(std::string cube, int max_depth = 20, int timelimit = 3600);
std::string scrambleStr(int timelimit = 10);

#endif
