#include "solve.h"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <sstream>
#include <string.h>
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
CubieCube cube1;
std::mutex start_mutex;
std::condition_variable start_var;
bool start;

// Various variables necessary to actually realize a simple timeout ...
std::mutex wait;
std::condition_variable notify;
int ret;

TwoPhaseSolver::TwoPhaseSolver(int rot1, bool inv1) {
  rot = rot1;
  inv_ = inv1;
}

void TwoPhaseSolver::solve(const CubieCube &cube) {
  CubieCube cube1;

  CubieCube tmp;
  mul(sym_cubes[inv_sym[ROT_SYM * rot]], cube, tmp);
  mul(tmp, sym_cubes[ROT_SYM * rot], cube1);
  if (inv_) {
    CubieCube tmp = cube1;
    inv(tmp, cube1);
  }

  int flip = getFlip(cube1);
  int twist = getTwist(cube1);
  Edges4 sslice(getSSlice(cube1));
  uedges[0].set(getUEdges(cube1));
  dedges[0].set(getDEdges(cube1));
  cperm[0].set(getCPerm(cube1));

  MoveMask mm;
  for (int togo = getFSTwistPrun(flip, sslice, twist, 0, mm); togo <= len; togo++) {
    cperm_depth = 0;
    edges_depth = 0;
    phase1(0, togo, flip, twist, sslice, phase1_moves);
  }
}

void TwoPhaseSolver::phase1(
  int depth, int togo, int flip, int twist, const Edges4 &sslice, MoveMask movemask
) {
  if (togo == 0) {
    for (int i = cperm_depth + 1; i <= depth; i++)
      moveCPerm(cperm[i - 1], moves[i - 1], cperm[i]);
    cperm_depth = depth - 1;
    CPerm cperm1 = cperm[depth];

    int tmp = getCornSlicePrun(cperm1, sslice);
    if (tmp >= len - depth)
      return;

    for (int i = edges_depth + 1; i <= depth; i++) {
      moveEdges4(uedges[i - 1], moves[i - 1], uedges[i]);
      moveEdges4(dedges[i - 1], moves[i - 1], dedges[i]);
    }
    edges_depth = depth - 1;
    Edges4 uedges1 = uedges[depth];
    Edges4 dedges1 = dedges[depth];

    for (int togo1 = std::max(getCornEdPrun(cperm1, uedges1, dedges1), tmp); togo1 < len - depth; togo1++) {
      if (phase2(depth, togo1, sslice, uedges1, dedges1, cperm1, movemask))
        return;
    }
    return;
  }

  MoveMask mm;
  int dist = getFSTwistPrun(flip, sslice, twist, togo, mm);
  if (dist == togo || dist + togo >= 5) {
    mm &= movemask;

    depth++;
    togo--;
    Edges4 sslice1;

    while (mm && !done) {
      int m = ffsll(mm) - 1;
      mm &= mm - 1;

      int flip1 = move_flip[flip][m];
      int twist1 = move_twist[twist][m];
      moveSSlice(sslice, m, sslice1);
      moves[depth - 1] = m;

      phase1(depth, togo, flip1, twist1, sslice1, next_moves[m]);
    }

    if (cperm_depth == depth - 1)
      cperm_depth--;
    if (edges_depth == depth - 1)
      edges_depth--;
  }
}

bool TwoPhaseSolver::phase2(
  int depth, int togo,
  const Edges4 &sslice, const Edges4 &uedges, const Edges4 &dedges, const CPerm &cperm,
  MoveMask movemask
) {
  if (togo == 0) {
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
      if (rot > 0) {
        for (int i = 0; i < depth; i++)
          sol[i] = conj_move[sol[i]][ROT_SYM * rot];
      }

      if (depth <= max_depth) { // keep searching if current solution exceeds max-depth
        std::lock_guard<std::mutex> lock(wait);
        done = true;
        notify.notify_one();
      }
    }

    mutex.unlock();
    return true;
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
      int tmp = m - (N_MOVES - N_DOUBLE2);
      if (tmp >= 0) {
        if (getCornEdPrun(cperm1, uedges1, dedges1) < togo - 1) {
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
        return true;
    }
  }

  return done;
}

void prepareSolve() {
  waitForFinish();
  start = false;

  for (int rot = 0; rot < 3; rot++) {
    #ifdef FACES5
      if (rot > 1)
        break;
    #endif
    for (int inv = 0; inv < 2; inv++) {
      std::thread thread([rot, inv]() {
        TwoPhaseSolver solver(rot, (bool) inv);
        {
          std::unique_lock<std::mutex> lock(start_mutex);
          start_var.wait(lock, []{ return start; });
        }
        solver.solve(cube1);
      });
      threads.push_back(std::move(thread));
    }
  }
}

void waitForFinish() {
  for (std::thread& thread : threads)
    thread.join();
  threads.clear();
}

int solve(const CubieCube &cube, int max_depth1, int timelimit, std::vector<int> &sol1) {
  done = false;
  sol.clear();
  max_depth = max_depth1;
  len = max_depth > 0 ? max_depth + 1 : N;

  cube1 = cube;
  {
    std::lock_guard<std::mutex> lock(start_mutex);
    start = true;
    start_var.notify_all();
  }

  ret = 0;
  {
    std::unique_lock<std::mutex> lock(wait);
    notify.wait_for(lock, std::chrono::milliseconds(timelimit), []{ return done; });
    if (!done) {
      done = true;
      ret++;
    }
  }

  std::lock_guard<std::mutex> lock(mutex);
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

std::string twophase(std::string s, int max_depth, int timelimit, bool prepare, bool wait) {
  CubieCube cube;
  int err = faceToCubie(s, cube);
  if (err)
    return "FaceError " + std::to_string(err);
  if ((err = check(cube)))
    return "CubieError " + std::to_string(err);

  if (prepare)
    prepareSolve();
  std::vector<int> sol;
  int ret = solve(cube, max_depth, timelimit, sol);
  if (wait)
    waitForFinish();

  return ret == 0 ? solToStr(sol) : "SolveError " + std::to_string(ret);
}
