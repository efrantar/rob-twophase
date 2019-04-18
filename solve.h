#ifndef SOLVE_H_
#define SOLVE_H_

#define N 100

#include <vector>

bool kPairMove[] = {
  false, false, false, false, false, true,
  false, false, true, false, false, false,
  false, false, true, false, false, true
};

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
    
    bool found;
    int moves[N];
    std::vector<int> sol;

    void phase1(int depth, int dist, int limit);
    void phase2(int depth, int dist, int limit);

  public:
    Solver(int max_depth);
    std::string solve(const CubieCube &cube);

}

#endif

