#include "solve.h"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <sstream>
#include <cstring>
#include <thread>

#include "coord.h"
#include "cubie.h"
#include "face.h"
#include "moves.h"
#include "sym.h"
#include "prun.h"

std::mutex mutex; // lock for writing solutions
bool done; // signal solver shutdown
std::vector<int> sol; // global shared solution

int len; // length of current best solution
int max_depth; // stop as soon as a solution with at most this depth is found

std::vector<std::thread> threads;
std::vector<TwoPhaseSolver> solvers;
int dists[6]; // current min dist that has not been tried for this axis yet; used for thread scheduling
std::mutex start; // different lock than `mutex` for task assignment to minimize interference with solving threads

// Variables used to realize a simple timeout
std::mutex wait_mutex;
std::condition_variable notify;

TwoPhaseSolver::TwoPhaseSolver(int rot1, bool inv1, const CubieCube &cube) {
  rot = rot1;
  inv_ = inv1;

  CubieCube cube1;
  CubieCube tmp;

  mul(sym_cubes[inv_sym[ROT_SYM * rot]], cube, tmp);
  mul(tmp, sym_cubes[ROT_SYM * rot], cube1);
  if (inv_) {
    tmp = cube1; // results should be in `cube1` hence we need to copy
    inv(tmp, cube1);
  }

  flip = getFlip(cube1);
  twist = getTwist(cube1);
  sslice.set(getSSlice(cube1));
  cperm.set(getCPerm(cube1));
  uedges[0].set(getUEdges(cube1));
  dedges[0].set(getDEdges(cube1));
}

int TwoPhaseSolver::lower_bound() {
    MoveMask mm; // just a dummy
    return getFSTwistPrun(flip, sslice, twist, 0, mm);
}

void TwoPhaseSolver::solve(int togo) {
  edges_depth = 0; // always need to reset as it might be negative after a previous search
  phase1(0, togo, flip, twist, sslice, cperm, phase1_moves);
}

void TwoPhaseSolver::phase1(
  int depth, int togo, int flip, int twist, Edges4 sslice, CPerm cperm, MoveMask movemask
) {
  if (togo == 0) {
    int tmp = getCornSlicePrun(cperm, sslice);
    if (tmp >= len - depth) // phase 2 precheck, only reconstruct edges if successful
      return;

    for (int i = edges_depth + 1; i <= depth; i++) {
      moveEdges4(uedges[i - 1], moves[i - 1], uedges[i]);
      moveEdges4(dedges[i - 1], moves[i - 1], dedges[i]);
    }
    edges_depth = depth - 1;
    Edges4 uedges1 = uedges[depth];
    Edges4 dedges1 = dedges[depth];

    for (int togo1 = std::max(getCornEdPrun(cperm, uedges1, dedges1), tmp); togo1 < len - depth; togo1++) {
      // We don't want to block any moves here as this might cause us to require another full search with a
      // a higher depth if we get unlucky (~10% performance loss)
      if (phase2(depth, togo1, sslice, uedges1, dedges1, cperm, phase2_moves))
        return; // once we have found a phase 2 solution, there cannot be any shorter ones -> quit
    }
    return;
  }

  MoveMask mm;
  int dist = getFSTwistPrun(flip, sslice, twist, togo, mm);
  if (dist == togo || dist + togo >= 5) { // small optimization to avoid exploring too similar solutions
    mm &= phase1_moves & movemask;

    depth++;
    togo--;
    Edges4 sslice1;
    CPerm cperm1;

    while (mm && !done) {
      int m = ffsll(mm) - 1; // get rightmost move index (`ffsll()` uses 1-based indexing)
      mm &= mm - 1; // delete rightmost bit

      int flip1 = move_flip[flip][m];
      int twist1 = move_twist[twist][m];
      moveSSlice(sslice, m, sslice1);
      moveCPerm(cperm, m, cperm1);
      moves[depth - 1] = m;

      phase1(depth, togo, flip1, twist1, sslice1, cperm1, next_moves[m]);
    }

    // We always want to maintain the maximum number of already reconstructed EDGES coordinates, hence we only
    // decrement when the depth level gets lower than the current valid index (note that we will typically also
    // visit other deeper branches in between that might not have an effect on this)
    if (edges_depth == depth - 1)
      edges_depth--;
  }
}

bool TwoPhaseSolver::phase2(
  int depth, int togo,
  Edges4 sslice, Edges4 uedges, Edges4 dedges, CPerm cperm,
  MoveMask movemask
) {
  if (togo == 0) {
    // As the phase 2 pruning table is not 100% exact, we might be able to get here without actually solving the cube.
    // This has essentially no effect on the performance (especially as the solver only spends a small fraction of
    // time in phase 2 anyways) but we need to check it here before registering a solution
    if (sslice.perm != 0 || cperm.val() != 0 || mergeUDEdges2(uedges, dedges) != 0)
      return false;

    mutex.lock();

    if (depth < len) {
      sol.resize(depth);
      for (int i = 0; i < depth; i++)
        sol[i] = moves[i];
      len = depth; // update so that other threads can already search for shorter solutions

      if (inv_) {
        for (int i = 0; i < depth; i++)
          sol[i] = inv_move[sol[i]];
        std::reverse(sol.begin(), sol.end());
      }
      if (rot > 0) { // we need to "undo" the original symmetry transformation by conjugating moves
        for (int i = 0; i < depth; i++)
          sol[i] = conj_move[sol[i]][ROT_SYM * rot];
      }

      if (depth <= max_depth) { // keep searching if current solution exceeds max-depth
        // Notify the timeout thread that a solution has been found (it should not wait any longer)
        std::lock_guard<std::mutex> lock(wait_mutex);
        done = true;
        notify.notify_one();
      }
    }

    mutex.unlock();
    return true; // we cannot find any better solution, hence return true if even our solution was not accepted
  }

  Edges4 sslice1;
  Edges4 uedges1;
  Edges4 dedges1;
  CPerm cperm1;

  MoveMask mm = phase2_moves & movemask;
  while (mm && !done) {
    int m = ffsll(mm) - 1;
    mm &= mm - 1;

    moveSSlice(sslice, m, sslice1);
    moveEdges4(uedges, m, uedges1);
    moveEdges4(dedges, m, dedges1);
    moveCPerm(cperm, m, cperm1);

    #ifdef QUARTER
      // As we never want to leave the set of phase 2 cubes (which we would by doing only a quarter-turn on an axis
      // for which only double-moves are permitted), we need special handling of the double moves. The simplest way
      // to do this is to treat a double moves simply as if two consecutive quarter-turns were added to the current
      // search path.
      int tmp = m - (N_MOVES - N_DOUBLE2);
      if (tmp >= 0) {
        if (getCornEdPrun(cperm1, uedges1, dedges1) < togo - 1) {
          // Transform double move into corresponding simple move; a bit hacky but it avoids having to define any
          // additional global tables that are never used anywhere else
          int split = (tmp < 4) ? 4 + 2 * tmp : 16 + 4 * (tmp - 4);
          moves[depth] = split;
          moves[depth + 1] = split;
          if (phase2(depth + 2, togo - 2, sslice1, uedges1, dedges1, cperm1, next_moves[m]))
            return true;
        }
        continue;
      }
    #endif

    if (getCornEdPrun(cperm1, uedges1, dedges1) < togo) {
      moves[depth] = m;
      if (phase2(depth + 1, togo - 1, sslice1, uedges1, dedges1, cperm1, next_moves[m]))
        return true; // return as soon as we have a solution
    }
  }

  return done; // stop as early as possible
}

void prepareSolve(int n_threads) {
  // This means threads are already prepared and have not been used for a solve so far
  if (threads.size() > 0 && !done)
    return;
  waitForFinish();

  start.lock(); // lock here on the main thread to prevent worker threads from starting to solve
  done = false;

  for (int i = 0; i < n_threads; i++) {
    std::thread thread([]() {
      while (!done) {
        start.lock();
        int i = 0;
        // We always want to work on the most promising search directions, i.e. those with the shortest feasible
        // phase 1 depths still unexplored
        for (int j = 1; j < solvers.size(); j++) {
          if (dists[j]  < dists[i])
            i = j;
        }
        int togo = dists[i]++;
        start.unlock();

        if (togo >= len)
          break;
        // We cannot concurrently do two searches with the same solver object, hence we need to copy
        TwoPhaseSolver solver = solvers[i];
        solver.solve(togo);
      }
    });
    threads.push_back(std::move(thread)); // we don't want to duplicate any resources -> move
  }
}

void waitForFinish() {
  for (std::thread& thread : threads)
    thread.join();
  threads.clear(); // threads are now useless
}

int solve(const CubieCube &cube, int max_depth1, int timelimit, std::vector<int> &sol1) {
  sol.clear();
  max_depth = max_depth1;
  len = max_depth > 0 ? max_depth + 1 : N;

  solvers.clear();
  for (int rot = 0; rot < 3; rot++) {
    #ifdef FACES5
      // We can unfortunately only use 2 rotational symmetries in 5-face mode
      if (rot > 1)
        break;
    #endif
    for (int inv = 0; inv < 2; inv++) {
      TwoPhaseSolver solver(rot, inv, cube);
      dists[solvers.size()] = solver.lower_bound();
      solvers.push_back(std::move(solver));
    }
  }

  start.unlock();

  int ret = 0; // 1 indicates timeout, 2 no solution of desired length found
  {
    std::unique_lock<std::mutex> lock(wait_mutex);
    notify.wait_for(lock, std::chrono::milliseconds(timelimit), []{ return done; });
    if (!done) {
      // if get here, this was a timeout
      done = true;
      ret++;
    }
  }

  std::lock_guard<std::mutex> lock(mutex); // need to make sure no thread is still trying to write something
  if (sol.size() == len) {
    sol1.resize(len);
    for (int i = 0; i < len; i++)
      sol1[i] = sol[i];
    return max_depth1 > 0 ? ret : 0;
  }
  return 2;
}

// We persist only the pruning tables as files (the other ones can be generated on the fly quickly enough)
void initTwophase(bool file) {
  initFace();
  initMoves();
  initCoord();
  initSym();
  initPrun();

  initCoordTables();
  initSymTables();

  if (!file) {
    initFSTwistPrun();
    initCornEdPrun();
    initCornSlicePrun();
    return;
  }

  FILE *f = fopen(FILE_TWOPHASE, "rb");

  if (f == NULL) {
    initFSTwistPrun();
    initCornEdPrun();
    initCornSlicePrun();

    f = fopen(FILE_TWOPHASE, "wb");
    // Use `tmp` to avoid nasty warnings; not clean but we don't want to make this part too complicated
    int tmp = fwrite(fstwist_prun, sizeof(Prun), N_FSTWIST, f);
    tmp = fwrite(corned_prun, sizeof(uint8_t), N_CORNED, f);
    tmp = fwrite(cornslice_prun, sizeof(uint8_t), N_CORNSLICE, f);
  } else {
    fstwist_prun = new Prun[N_FSTWIST];
    corned_prun = new uint8_t[N_CORNED];
    cornslice_prun = new uint8_t[N_CORNSLICE];
    int tmp = fread(fstwist_prun, sizeof(Prun), N_FSTWIST, f);
    tmp = fread(corned_prun, sizeof(uint8_t), N_CORNED, f);
    tmp = fread(cornslice_prun, sizeof(uint8_t), N_CORNSLICE, f);
  }

  fclose(f);
}

std::string solToStr(const std::vector<int> &sol) {
  std::ostringstream ss;
  for (int i = 0; i < sol.size(); i++) {
    ss << move_names[sol[i]];
    if (i != sol.size() - 1)
      ss << " ";
  }
  return ss.str();
}

std::string twophase(std::string s, int max_depth, int timelimit, bool prepare, bool wait, int n_threads) {
  CubieCube cube;
  int err = faceToCubie(s, cube);
  if (err)
    return "FaceError " + std::to_string(err);
  if ((err = check(cube)))
    return "CubieError " + std::to_string(err);

  if (prepare)
    prepareSolve(n_threads);
  std::vector<int> sol;
  int ret = solve(cube, max_depth, timelimit, sol);
  if (wait)
    waitForFinish();

  return ret == 0 ? solToStr(sol) : "SolveError " + std::to_string(ret);
}

std::string scramble(int timelimit, int n_threads) {
  CubieCube cube;
  shuffle(cube); // generate uniformly random cube

  // For simplicity we simply always setup and tear-down solver threads
  prepareSolve(n_threads);
  std::vector<int> sol;
  solve(cube, -1, timelimit, sol); // we will always find at least some solution
  waitForFinish();

  // Turn solution into scramble
  std::reverse(sol.begin(), sol.end());
  for (int i = 0; i < sol.size(); i++)
    sol[i] = inv_move[sol[i]];
  return solToStr(sol);
}
