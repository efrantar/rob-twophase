#include "solve.h"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <sstream>
#include <thread>

#include "coord.h"
#include "cubie.h"
#include "face.h"
#include "moves.h"
#include "sym.h"
#include "prun.h"

/*
 * We always want to skip:
 * - Consecutive moves on the same axis
 * - Consecutive moves on parallel axes but in wrong (decreasing) order
 */
bool skip_move[N_MOVES][N_MOVES];

static bool init() {
  for (int m1 = 0; m1 < N_MOVES; m1++) {
    for (int m2 = 0; m2 < N_MOVES; m2++) {
      int axis_diff = m1 / 3 - m2 / 3;
      skip_move[m1][m2] = axis_diff == 0 || axis_diff == 3;
    }
  }
  return true;
}
static bool inited = init();

std::mutex mutex; // lock for writing solutions
bool done; // signal solver shutdown
std::vector<int> sol; // global shared solution

// Various variables necessary to actually realize a simple timeout ...
std::mutex wait;
std::condition_variable notify;
bool cont;
int ret;

int len; // length of current best solution
int max_depth; // stop as soon as a solution with at most this depth is found

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

  flip[0] = getFlip(cube1);
  twist[0] = getTwist(cube1);
  sslice[0] = getSSlice(cube1);
  uedges[0] = getUEdges(cube1);
  dedges[0] = getDEdges(cube1);
  cperm[0] = getCPerm(cube1);

  cperm_depth = 0;
  udedges_depth = 0;

  int dist = getFSTwistDist(flip[0], sslice[0], twist[0]);
  for (int togo = dist; togo <= len; togo++) {
    // Resetting here saves two `*_depth > 0` checks in every call
    cperm_depth = 0;
    udedges_depth = 0;
    phase1(0, dist, togo);
  }
}

int TwoPhaseSolver::phase1(int depth, int dist, int togo) {
  if (done)
    return 0;

  if (togo == 0) {
    for (int i = cperm_depth + 1; i <= depth; i++)
      cperm[i] = cperm_move[cperm[i - 1]][moves[i - 1]];
    cperm_depth = depth - 1;

    // We ignore phase 2 solutions that are longer than 10 moves
    int max_togo = std::min(len - 1 - depth, 12);
    if (cornslice_prun[CORNSLICE(cperm[depth], sslice[depth])] > max_togo)
      return cornslice_prun[CORNSLICE(cperm[depth], sslice[depth])] > max_togo + 1; // potentially skip to next axis

    for (int i = udedges_depth + 1; i <= depth; i++) {
      uedges[i] = uedges_move[uedges[i - 1]][moves[i - 1]];
      dedges[i] = dedges_move[dedges[i - 1]][moves[i - 1]];
    }
    udedges_depth = depth - 1;
    udedges[depth] = UDEDGES(uedges[depth], dedges[depth]);

    int dist1 = getCornEdDist(cperm[depth], udedges[depth]);

    // As phase 2 solutions often come in pairs, this essentially halves the expensive `getCornEdDist()` calls
    if (dist1 > max_togo + 1)
      return 1;
    for (int togo1 = dist1; togo1 <= max_togo; togo1++)
      phase2(depth, dist1, togo1);

    return 0;
  }

  // Discard shorter phase 1 solutions -> loses optimality but slight speedup
  if (dist == 0)
    return  0;
  for (int m = 0; m < N_MOVES; m++) {
    if (depth > 0 && skip_move[moves[depth - 1]][m])
      continue;

    flip[depth + 1] = flip_move[flip[depth]][m];
    sslice[depth + 1] = sslice_move[sslice[depth]][m];
    twist[depth + 1] = twist_move[twist[depth]][m];

    CCoord fslice = FSLICE(
      flip[depth + 1], SS_SLICE(sslice[depth + 1])
    );
    CCoord fstwist = FSTWIST(
      COORD(fslice_sym[fslice]),
      conj_twist[twist[depth + 1]][SYM(fslice_sym[fslice])]
    );
    int dist1 = next_dist[dist][getPrun3(fstwist_prun3, fstwist)];

    if (dist1 < togo) {
      moves[depth] = m;
      if (phase1(depth + 1, dist1, togo - 1) == 1)
        m = kAxisEnd[m];
      if (done)
        return 0;
    }
  }

  if (cperm_depth == depth)
    cperm_depth--;
  // We need to check this individually as `cperm_depth` might be updated considerably more often
  if (udedges_depth == depth)
    udedges_depth--;

  return 0;
}

void TwoPhaseSolver::phase2(int depth, int dist, int togo) {
  if (done)
    return;

  if (togo == 0) {
    mutex.lock();

    if (depth < len) {
      sol.resize(depth);
      for (int i = 0; i < depth; i++)
        sol[i] = moves[i];
      len = depth; // update so that other threads can already search for shorter solutions

      if (inv_) {
        for (int i = 0; i < depth; i++)
          sol[i] = kInvMove[sol[i]];
        std::reverse(sol.begin(), sol.end());
      }
      if (rot > 0) {
        for (int i = 0; i < depth; i++)
          sol[i] = conj_move[sol[i]][ROT_SYM * rot];
      }

      if (depth <= max_depth) // keep searching if current solution exceeds max-depth
        done = true;
    }

    mutex.unlock();
    return;
  }

  for (int m = 0; m < N_MOVES2; m++) {
    if (depth > 0 && skip_move[moves[depth - 1]][kPhase2Moves[m]])
      continue;

    sslice[depth + 1] = sslice_move[sslice[depth]][kPhase2Moves[m]];
    cperm[depth + 1] = cperm_move[cperm[depth]][kPhase2Moves[m]];
    udedges[depth + 1] = udedges_move2[udedges[depth]][m];

    CCoord corned = CORNED(
      COORD(cperm_sym[cperm[depth + 1]]),
      conj_udedges[udedges[depth + 1]][SYM(cperm_sym[cperm[depth + 1]])]
    );
    int dist1 = next_dist[dist][getPrun3(corned_prun3, corned)];

    int tmp = cornslice_prun[CORNSLICE(cperm[depth + 1], sslice[depth + 1])];
    if (std::max(dist1, tmp) < togo) {
      // There are generally rather few `phase2()` calls, hence axis skipping is not worth it here
      moves[depth] = kPhase2Moves[m];
      phase2(depth + 1, dist1, togo - 1);
      if (done)
        return;
    }
  }
}

int twophase(const CubieCube &cube, int max_depth1, int timelimit, std::vector<int> &sol1) {
  done = false;

  cont = false;
  ret = 0;
  std::thread timeout([timelimit]() {
    std::unique_lock<std::mutex> lock(wait);
    notify.wait_for(lock, std::chrono::milliseconds(timelimit), []{ return cont; });
    done = true;
    ret++;
  }); // timeout thread

  sol.clear();
  max_depth = max_depth1;
  len = max_depth > 0 ? max_depth + 1 : N; // initial reference value for pruning

  std::vector<std::thread> threads;
  for (int rot = 0; rot < 3; rot++) {
    #ifdef FACES5
      if (rot > 1)
        break;
    #endif
    for (int inv = 0; inv < 2; inv++) {
      TwoPhaseSolver solver(rot, (bool) inv);
      threads.push_back(std::thread(&TwoPhaseSolver::solve, solver, cube));
    }
  }

  for (int i = 0; i < threads.size(); i++)
    threads[i].join();
  // All of this is necessary for the timeout to work as expected
  {
    std::lock_guard<std::mutex> lock(wait);
    cont = true;
    if (ret == 0)
      ret--;
  }
  notify.notify_one();
  timeout.join();

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
  initTwistMove();
  initFlipMove();
  initSSliceMove();
  initUEdgesMove();
  initDEdgesMove();
  initUDEdgesMove2();
  initCPermMove();

  initConjTwist();
  initConjUDEdges();
  initFlipSliceSym();
  initCPermSym();

  if (!file) {
    initFSTwistPrun3();
    initCornUDPrun3();
    initCornSlicePrun();
    return;
  }

  FILE *f = fopen(FILE_TWOPHASE, "rb");

  if (f == NULL) {
    initFSTwistPrun3();
    initCornUDPrun3();
    initCornSlicePrun();

    f = fopen(FILE_TWOPHASE, "wb");
    // Use `tmp` to avoid nasty warnings; not clean but we don't want to make this part to complicated
    int tmp = fwrite(fstwist_prun3, sizeof(uint64_t), N_FSTWIST / 32 + 1, f);
    tmp = fwrite(corned_prun3, sizeof(uint64_t), N_CORNED / 32 + 1, f);
    tmp = fwrite(cornslice_prun, sizeof(uint8_t), N_CORNSLICE, f);
  } else {
    fstwist_prun3 = new uint64_t[N_FSTWIST / 32 + 1];
    corned_prun3 = new uint64_t[N_CORNED / 32 + 1];
    cornslice_prun = new uint8_t[N_CORNSLICE];
    int tmp = fread(fstwist_prun3, sizeof(uint64_t), N_FSTWIST / 32 + 1, f);
    tmp = fread(corned_prun3, sizeof(uint64_t), N_CORNED / 32 + 1, f);
    tmp = fread(cornslice_prun, sizeof(uint8_t), N_CORNSLICE, f);
  }

  fclose(f);
}

std::string solToStr(const std::vector<int> &sol) {
  std::ostringstream ss;
  for (int i = 0; i < sol.size(); i++) {
    ss << kMoveNames[sol[i]];
    if (i != sol.size() - 1)
      ss << " ";
  }
  return ss.str();
}

std::string twophaseStr(std::string s, int max_depth, int timelimit) {
  CubieCube cube;
  int err = faceToCubie(s, cube);
  if (err)
    return "FaceError " + std::to_string(err);
  if ((err = check(cube)))
    return "CubieError " + std::to_string(err);

  std::vector<int> sol;
  int ret = twophase(cube, max_depth, timelimit, sol);
  return ret == 0 ? solToStr(sol) : "SolveError " + std::to_string(ret);
}
