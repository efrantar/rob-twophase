#ifndef __SOLVE__
#define __SOLVE__

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>
#include <thread>
# include "move.h"

namespace solve {

  using solution = std::pair<std::vector<int>, int>;
  inline bool less_than(const solution& s1, const solution&  s2) { return s1.first.size() < s2.first.size(); }

  struct coord_cube {
    int flip;
    int slice;
    int twist;
    int uedges;
    int dedges;
    int corners;
  };

  #ifdef F5
    const int N_DIRS = 4; // number of search directions
  #else
    const int N_DIRS = 6;
  #endif

  class Engine {

    int n_threads;
    int n_splits;
    int n_sols;
    int max_len;
    int tlim;

    coord_cube dirs[N_DIRS];
    move::mask masks[move::COUNT1];
    int depths[N_DIRS];
    int splits[N_DIRS];

    bool done;
    int lenlim;
    std::mutex job_mtx;
    std::mutex sol_mtx;
    std::priority_queue<solution, std::vector<solution>, decltype(&less_than)> sols {less_than};
    std::vector<std::thread> threads;

    std::mutex tout_mtx;
    std::condition_variable tout_cvar;

  public:
    Engine(
      int n_threads, int tlim,
      int n_sols = 1, int max_len = -1, int n_splits = 1
    );
    void thread();
    void prepare();
    std::vector<std::vector<int>> solve(const cubie::cube& c);
    void report_sol(solution& sol);
    void finish();

  };

  class Search {

    int dir;
    const coord_cube& cube;
    int p1depth;
    move::mask d0moves;
    bool& done;
    int& lenlim;
    Engine& solver;

    int uedges[50];
    int dedges[50];
    int edges_depth;
    int moves[50];

  private:
    void phase1(
      int depth, int togo, int flip, int slice, int twist, int corners, move::mask next, move::mask qt_skip
    );
    bool phase2(
      int depth, int togo, int slice, int udedges2, int corners, move::mask next, move::mask qt_skip
    );

  public:
    Search(
      int dir,
      const coord_cube& cube,
      int p1depth, move::mask d0moves,
      bool& done, int& lenlim, Engine& solver
    ) : dir(dir), cube(cube), p1depth(p1depth), d0moves(d0moves), done(done), lenlim(lenlim), solver(solver) {};
    void run();

  };
}

#endif
