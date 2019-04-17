#define N 100

class Solver {
  
  private:
    Coord flip[N];
    Coord slicesorted[N];
    Coord twist[N];
    Coord uedges[N];
    Coord dedges[N];

    int max_depth;
    int moves[N];

    void phase1(int depth, int dist, int limit);
    void phase2(int depth, int dist, int limit);

  public:
    std::string solve(const CubieCube &cube);

}

