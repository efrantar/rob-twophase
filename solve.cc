#include "solve.h"

#include <algorithm>
#include <ctime>
#include <mutex>
#include <thread>
#include <vector>

#include "coord.h"
#include "cubie.h"
#include "moves.h"
#include "sym.h"
#include "prun.h"

#define FILENAME "twophase.tbl"

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

std::mutex mutex;
bool done;
std::vector<int> sol;

int len;
int max_depth;
clock_t endtime;

TwoPhaseSolver::TwoPhaseSolver(int rot1, bool inv1) {
  rot = rot1;
  inv = inv1;
}

void TwoPhaseSolver::solve(const CubieCube &cube) {
  CubieCube cube1;

  if (rot == 1) {
    CubieCube tmp;
    mul(sym_cubes[inv_sym[16]], cube, tmp);
    mul(tmp, sym_cubes[16], cube1);
  } else if (rot == 2) {
    CubieCube tmp;
    mul(sym_cubes[inv_sym[32]], cube, tmp);
    mul(tmp, sym_cubes[32], cube1);
  } else
    copy(cube, cube1);
  if (inv)
    cube1 = invCube(cube1);

  flip[0] = getFlip(cube1);
  twist[0] = getTwist(cube1);
  sslice[0] = getSSlice(cube1);
  uedges[0] = getUEdges(cube1);
  dedges[0] = getDEdges(cube1);
  corners[0] = getCorners(cube1);

  corners_depth = 0;
  udedges_depth = 0;

  int dist = getFSTwistDist(flip[0], sslice[0], twist[0]);
  for (int limit = dist; limit < len; limit++)
    phase1(0, dist, limit);
}

void TwoPhaseSolver::phase1(int depth, int dist, int limit) {
  if (done)
    return;
  else if (clock() > endtime) {
    done = true;
    return;
  }

  if (depth == limit) {
    for (int i = corners_depth + 1; i <= depth; i++)
      corners[i] = corners_move[corners[i - 1]][moves[i - 1]];
    if (depth > 0)
      corners_depth = depth - 1;

    int max_limit = std::min(len - 1 - depth, 10);
    if (cornslice_prun[CORNSLICE(corners[depth], sslice[depth])] > max_limit)
      return;
    max_limit += depth;

    for (int i = udedges_depth + 1; i <= depth; i++) {
      uedges[i] = uedges_move[uedges[i - 1]][moves[i - 1]];
      dedges[i] = dedges_move[dedges[i - 1]][moves[i - 1]];
    }
    if (depth > 0)
      udedges_depth = depth - 1;
    udedges[depth] = merge_udedges[uedges[depth]][dedges[depth] % 24];

    int dist1 = getCornUDDist(corners[depth], udedges[depth]);
    for (int limit1 = depth + dist1; limit1 <= max_limit; limit1++)
      phase2(depth, dist1, limit1);

    return;
  }

  for (int m = 0; m < N_MOVES; m++) {
    if (depth > 0 && skip_move[moves[depth - 1]][m])
      continue;
    if (dist == 0 && depth > limit - 5 && kIsPhase2Move[m])
      continue;

    flip[depth + 1] = flip_move[flip[depth]][m];
    sslice[depth + 1] = sslice_move[sslice[depth]][m];
    twist[depth + 1] = twist_move[twist[depth]][m];

    CoordL fslice = FSLICE(
      flip[depth + 1], SS_SLICE(sslice[depth + 1])
    );
    CoordL fstwist = FSTWIST(
      COORD(fslice_sym[fslice]),
      conj_twist[twist[depth + 1]][SYM(fslice_sym[fslice])]
    );
    int dist1 = next_dist[dist][getPrun3(fstwist_prun3, fstwist)];

    if (depth + dist1 < limit) {
      moves[depth] = m;
      phase1(depth + 1, dist1, limit);
      if (done)
        return;
    }
  }

  if (depth > 0 && corners_depth == depth)
    corners_depth--;
  if (depth > 0 && udedges_depth == depth)
    udedges_depth--;
}

void TwoPhaseSolver::phase2(int depth, int dist, int limit) {
  if (done)
    return;
  else if (clock() > endtime) {
    done = true;
    return;
  }

  if (depth == limit) {
    mutex.lock();

    if (depth < len) {
      sol.resize(depth);
      for (int i = 0; i < depth; i++)
        sol[i] = moves[i];
      len = depth;

      if (inv) {
        for (int i = 0; i < depth; i++)
          sol[i] = kInvMove[sol[i]];
        std::reverse(sol.begin(), sol.end());
      }
      if (rot > 0) {
        for (int i = 0; i < depth; i++)
          sol[i] = conj_move[sol[i]][16 * rot];
      }

      if (depth <= max_depth)
        done = true;
    }

    mutex.unlock();
    return;
  }

  for (int m = 0; m < N_MOVES2; m++) {
    if (depth > 0 && skip_move[moves[depth - 1]][kPhase2Moves[m]])
      continue;

    sslice[depth + 1] = sslice_move[sslice[depth]][kPhase2Moves[m]];
    corners[depth + 1] = corners_move[corners[depth]][kPhase2Moves[m]];
    udedges[depth + 1] = udedges_move2[udedges[depth]][m];

    CoordL cornud = CORNUD(
      COORD(corners_sym[corners[depth + 1]]),
      conj_udedges[udedges[depth + 1]][SYM(corners_sym[corners[depth + 1]])]
    );
    int dist1 = next_dist[dist][getPrun3(cornud_prun3, cornud)];

    int tmp = cornslice_prun[CORNSLICE(corners[depth + 1], sslice[depth + 1])];
    if (depth + std::max(dist1, tmp) < limit) {
      moves[depth] = kPhase2Moves[m];
      phase2(depth + 1, dist1, limit);
      if (done)
        return;
    }
  }
}

std::vector<int> twophase(const CubieCube &cube, int max_depth1, int timelimit) {
  endtime = clock() + clock_t(CLOCKS_PER_SEC / 1000. * timelimit);

  done = false;
  sol.clear();
  max_depth = max_depth1;
  len = max_depth > 0 ? max_depth + 1 : 100;

  bool rotsym = false;
  bool antisym = false;
  checkSyms(cube, rotsym, antisym);

  std::vector<std::thread> threads;
  for (int rot = 0; rot < 3; rot++) {
    if (rotsym && rot > 0)
      break;
    for (int inv = 0; inv < 2; inv++) {
      if (antisym && inv > 0)
        break;
      TwoPhaseSolver solver(rot, (bool) inv);
      threads.push_back(std::thread(&TwoPhaseSolver::solve, solver, cube));
    }
  }
  for (int i = 0; i < threads.size(); i++)
    threads[i].join();

  return sol;
}

/* TODO: very crude implementation of the file-loading, but sufficient for now */
void initTwophase(bool file) {
  initTwistMove();
  initFlipMove();
  initSSliceMove();
  initUEdgesMove();
  initDEdgesMove();
  initUDEdgesMove2();
  initCornersMove();
  initMergeUDEdges();

  initConjTwist();
  initConjUDEdges();
  initFlipSliceSym();
  initCornersSym();

  if (!file) {
    initFSTwistPrun3();
    initCornUDPrun3();
    initCornSlicePrun();
    return;
  }

  FILE *f = fopen(FILENAME, "rb");
  if (f == NULL) {
    initFSTwistPrun3();
    initCornUDPrun3();
    initCornSlicePrun();
    f = fopen(FILENAME, "wb");
    int tmp = fwrite(fstwist_prun3, 8, N_FSTWIST / 32 + 1, f);
    tmp = fwrite(cornud_prun3, 8, N_CORNUD / 32 + 1, f);
    tmp = fwrite(cornslice_prun, 1, N_CORNSLICE, f);
  } else {
    fstwist_prun3 = new uint64_t[N_FSTWIST / 32 + 1];
    cornud_prun3 = new uint64_t[N_CORNUD / 32 + 1];
    cornslice_prun = new uint8_t[N_CORNSLICE];
    int tmp = fread(fstwist_prun3, 8, N_FSTWIST / 32 + 1, f);
    tmp = fread(cornud_prun3, 8, N_CORNUD / 32 + 1, f);
    tmp = fread(cornslice_prun, 1, N_CORNSLICE, f);
  }
  fclose(f);
}
