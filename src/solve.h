#ifndef __SOLVE__
#define __SOLVE__

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>
#include <thread>
# include "move.h"

namespace solve {

  using searchres = std::pair<std::vector<int>, int>; // moves + search direction
  inline bool cmp(const searchres& s1, const searchres&  s2) { return s1.first.size() < s2.first.size(); }

  // Container with coords of a starting position
  struct coordc {
    int flip;
    int slice;
    int twist;
    int uedges;
    int dedges;
    int corners;
  };

  // Number of search directions
  #ifdef F5
    const int N_DIRS = 4;
  #else
    const int N_DIRS = 6;
  #endif

  class Engine {

    int n_threads; // number of search threads
    int n_splits; // number of sub-searches every search is split into
    int n_sols; // number of solutions to find
    int max_len; // find solutions with at most this length; -1 means simply search for the full `tlimit`
    int tlim; // search for this amount of milliseconds

    coordc dirs[N_DIRS]; // search directions
    move::mask masks[move::COUNT1]; // split masks
    int depths[N_DIRS]; // current search depths per direction
    int splits[N_DIRS]; // current search splits per direction

    bool done; // indicate that we are done
    int lenlim; // only look for solution that are strictly shorter than this
    std::mutex job_mtx; // thread-safety for selection of the next search task
    std::mutex sol_mtx; // thread-safety for reporting a solution
    std::priority_queue<searchres, std::vector<searchres>, decltype(&cmp)> sols {cmp}; // already found solutions
    std::vector<std::thread> threads; // search threads

    // Tools for implementing a required timeout
    std::mutex tout_mtx;
    std::condition_variable tout_cvar;

    public:
      Engine(
        int n_threads, int tlim,
        int n_sols = 1, int max_len = -1, int n_splits = 1
      );
      void prepare(); // setup all threads
      void solve(const cubie::cube& c, std::vector<std::vector<int>>& res); // actual solve
      void finish(); // wait for all threads to shutdown (mostly for clean program exit)
      void report_sol(searchres& sol); // report a solution; never call this from the outside

    void thread(); // search thread

  };

}

#endif
