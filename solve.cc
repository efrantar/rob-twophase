#include "solve.h"

#include <ctime>
#include <mutex>
#include <thread>
#include <vector>

#include "coord.h"
#include "cubie.h"
#include "moves.h"
#include "sym.h"
#include "prun.h"

bool skip_move[N_MOVES][N_MOVES];

std::mutex mutex;
bool done;
std::vector<int> sol;

int len;
int max_depth;
clock_t endtime;

Solver::Solver(int rot1, bool inv1) {
  rot = rot1;
  inv = inv1;
}

void Solver::solve(const CubieCube &cube) {
  CubieCube cube1;
  copy(cube, cube1);

  flip[0] = getFlip(cube1);
  twist[0] = getTwist(cube1);

  slicesorted[0] = getSliceSorted(cube1);
  copy(cube, cube1);
  uedges[0] = getUEdges(cube1);
  copy(cube, cube1);
  dedges[0] = getDEdges(cube1);

  corners_depth = 0;
  udedges_depth = 0;

  int dist = getDepthFSSymTwistPrun3(flip[0], slicesorted[0], twist[0]);
  for (int limit = dist; limit < len; limit++)
    phase1(0, dist, limit);
}

void Solver::phase1(int depth, int dist, int limit) {
  if (done)
    return;
  else if (clock() > endtime) {
    done = true;
    return;
  }

  if (depth == limit) {
    for (int i = corners_depth + 1; i <= depth; i++)
      corners[i] = corners_move[corners[i - 1]][moves[i - 1]];
    corners_depth = depth - 1;

    int max_limit = std::min(len - 1 - depth, 11);
    if (cornersslices_prun[CORNERSSLICES(corners[depth], slicesorted[depth])] >= max_limit)
      return;
    max_limit += depth;

    for (int i = udedges_depth + 1; i <= depth; i++) {
      uedges[i] = uedges_move[uedges[i - 1]][moves[i - 1]];
      dedges[i] = dedges_move[dedges[i - 1]][moves[i - 1]];
    }
    udedges_depth = depth - 1;
    udedges[depth] = merge_udedges[uedges[depth]][dedges[depth] % 24];

    int dist1 = getDepthCSymUDEdgesPrun3(corners[depth], udedges[depth]);
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
    slicesorted[depth + 1] = slicesorted_move[slicesorted[depth]][m];
    twist[depth + 1] = twist_move[twist[depth]][m];

    LargeCoord flipslice = FLIPSLICE(
      flip[depth + 1], SS_SLICE(slicesorted[depth + 1])
    );
    LargeCoord fssymtwist = FSSYMTWIST(
      flipslice_sym[flipslice], 
      conj_twist[twist[depth + 1]][flipslice_sym_sym[flipslice]]
    );
    int dist1 = next_dist[dist][getPrun3(fssymtwist_prun3, fssymtwist)];

    if (depth + dist1 < limit) {
      moves[depth] = m;
      phase1(depth + 1, dist1, limit);
      if (done)
        return;
    }
  }

  if (corners_depth == depth)
    corners_depth--;
  if (udedges_depth == depth)
    udedges_depth--;
}

void Solver::phase2(int depth, int dist, int limit) {
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

      if (depth == max_depth)
        done = true;
    }

    mutex.unlock();
    return;
  }

  for (int m = 0; m < N_MOVES_P2; m++) {
    if (depth > 0 && skip_move[moves[depth - 1]][kPhase2Moves[m]])
      continue;

    slicesorted[depth + 1] = slicesorted_move[slicesorted[depth]][kPhase2Moves[m]];
    corners[depth + 1] = corners_move[corners[depth]][kPhase2Moves[m]];
    udedges[depth + 1] = udedges_move[udedges[depth]][m];

    LargeCoord csymudedges = CSYMUDEDGES(
      corners_sym[corners[depth + 1]], 
      conj_udedges[udedges[depth + 1]][corners_sym_sym[corners[depth + 1]]]
    );
    int dist1 = next_dist[dist][getPrun3(csymudedges_prun3, csymudedges)];

    int tmp = cornersslices_prun[CORNERSSLICES(corners[depth + 1], slicesorted[depth + 1])];
    if (depth + std::max(dist1, tmp) < limit) {
      moves[depth] = kPhase2Moves[m];
      phase2(depth + 1, dist1, limit);
      if (done)
        return;
    }
  }
}

std::string solve(const CubieCube &cube, int max_depth1, int timelimit) {
  done = false;
  sol.clear();

  max_depth = max_depth1;
  len = max_depth > 0 ? max_depth + 1 : 23;
  endtime = clock() + clock_t(CLOCKS_PER_SEC / 1000. * timelimit);

  Solver solver(0, false);
  std::thread t(&Solver::solve, solver, cube);
  t.join();

  std::string s;
  for (int i = 0; i < sol.size(); i++) {
    s += kMoveNames[sol[i]];
    if (i != sol.size() - 1)
      s += " ";
  }
  return s;
}

void initSolve() {
  for (int m1 = 0; m1 < N_MOVES; m1++) {
    for (int m2 = 0; m2 < N_MOVES; m2++) {
      int axis_diff = m1 / 3 - m2 / 3;
      skip_move[m1][m2] = axis_diff == 0 || axis_diff == 3;
    }
  }
}
