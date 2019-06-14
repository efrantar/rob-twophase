#include "solve.h"

#include <algorithm>
#include <condition_variable>
#include <ctime>
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

/* Variables for the optimal solver */
int moves[N];
int prun[3][N];
Coord flip[3][N];
Coord sslice[3][N];
Coord twist[3][N];
Coord uedges[N];
Coord dedges[N];
Coord corners1[N];
int corners1_depth;
int uedges_depth;
int dedges_depth;

TwoPhaseSolver::TwoPhaseSolver(int rot1, bool inv1) {
  rot = rot1;
  inv_ = inv1;
}

void TwoPhaseSolver::solve(const CubieCube &cube) {
  CubieCube cube1;

  if (rot == 1) {
    CubieCube tmp;
    mul(sym_cubes[inv_sym[16]], cube, tmp); // 1. URF symmetry
    mul(tmp, sym_cubes[16], cube1);
  } else if (rot == 2) {
    CubieCube tmp;
    mul(sym_cubes[inv_sym[32]], cube, tmp); // 2. URF symmetry
    mul(tmp, sym_cubes[32], cube1);
  } else
    cube1 = cube;
  if (inv_) {
    CubieCube tmp = cube1;
    inv(tmp, cube1);
  }

  flip[0] = getFlip(cube1);
  twist[0] = getTwist(cube1);
  sslice[0] = getSSlice(cube1);
  uedges[0] = getUEdges(cube1);
  dedges[0] = getDEdges(cube1);
  corners[0] = getCPerm(cube1);

  corners_depth = 0;
  udedges_depth = 0;

  int dist = getFSTwistDist(flip[0], sslice[0], twist[0]);
  for (int togo = dist; togo <= len; togo++)
    phase1(0, dist, togo);
}

int TwoPhaseSolver::phase1(int depth, int dist, int togo) {
  if (done)
    return 0;

  if (togo == 0) {
    for (int i = corners_depth + 1; i <= depth; i++)
      corners[i] = cperm_move[corners[i - 1]][moves[i - 1]];
    if (depth > 0)
      corners_depth = depth - 1;

    // We ignore phase 2 solutions that are longer than 10 moves
    int max_togo = std::min(len - 1 - depth, 10);
    if (cornslice_prun[CORNSLICE(corners[depth], sslice[depth])] > max_togo)
      return cornslice_prun[CORNSLICE(corners[depth], sslice[depth])] > max_togo + 1; // potentially skip to next axis

    for (int i = udedges_depth + 1; i <= depth; i++) {
      uedges[i] = uedges_move[uedges[i - 1]][moves[i - 1]];
      dedges[i] = dedges_move[dedges[i - 1]][moves[i - 1]];
    }
    if (depth > 0)
      udedges_depth = depth - 1;
    udedges[depth] = UDEDGES(uedges[depth], dedges[depth]);

    int dist1 = getCornUDDist(corners[depth], udedges[depth]);

    // As phase 2 solutions often come in pairs, this essentially halves the expensive getCornUDDist() calls
    if (dist1 > max_togo + 1)
      return 1;
    for (int togo1 = dist1; togo1 <= max_togo; togo1++)
      phase2(depth, dist1, togo1);

    return 0;
  }

  // Discard shorter phase 1 solutions -> looses optimality but slight speedup
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

  if (depth > 0 && corners_depth == depth)
    corners_depth--;
  // We need to check this individually as `corners_depth` might be updated much more often
  if (depth > 0 && udedges_depth == depth)
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
          sol[i] = conj_move[sol[i]][16 * rot];
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
    corners[depth + 1] = cperm_move[corners[depth]][kPhase2Moves[m]];
    udedges[depth + 1] = udedges_move2[udedges[depth]][m];

    CCoord cornud = CORNUD(
      COORD(corners_sym[corners[depth + 1]]),
      conj_udedges[udedges[depth + 1]][SYM(corners_sym[corners[depth + 1]])]
    );
    int dist1 = next_dist[dist][getPrun3(cornud_prun3, cornud)];

    int tmp = cornslice_prun[CORNSLICE(corners[depth + 1], sslice[depth + 1])];
    if (std::max(dist1, tmp) < togo) {
      // There are generally very few phase 2 calls, hence axis skipping is not worth it here
      moves[depth] = kPhase2Moves[m];
      phase2(depth + 1, dist1, togo - 1);
      if (done)
        return;
    }
  }
}

void optim(int depth, int dist, int togo) {
  if (done)
    return;

  // As this will usually be called many many times, reconstructing one coord after the other seems worth it
  if (dist == 0) {
    for (int i = corners1_depth + 1; i <= depth; i++)
      corners1[i] = cperm_move[corners1[i - 1]][moves[i - 1]];
    if (depth > 0)
      corners1_depth = depth - 1;
    if (corners1[depth] != 0)
      return;

    for (int i = uedges_depth + 1; i <= depth; i++)
      uedges[i] = uedges_move[uedges[i - 1]][moves[i - 1]];
    if (depth > 0)
      uedges_depth = depth - 1;
    if (uedges[depth] != 0)
      return;

    for (int i = dedges_depth + 1; i <= depth; i++)
      dedges[i] = dedges_move[dedges[i - 1]][moves[i - 1]];
    if (depth > 0)
      dedges_depth = depth - 1;
    if (dedges[depth] != DEDGES_SOLVED) // DEDDGES is non-0 in solved state
      return;

    sol.resize(depth);
    for (int i = 0; i < depth; i++)
      sol[i] = moves[i];
    len = depth;
    done = true;

    return;
  }

  for (int m = 0; m < N_MOVES; m++) {
    if (depth > 0 && skip_move[moves[depth - 1]][m])
      continue;

    bool next = false; // stop as soon as one of the cubes is not solvable anymore in the given number of moves
    for (int rot = 0; rot < 3; rot++) {
      flip[rot][depth + 1] = flip_move[flip[rot][depth]][conj_move[m][16 * rot]];
      sslice[rot][depth + 1] = sslice_move[sslice[rot][depth]][conj_move[m][16 * rot]];
      twist[rot][depth + 1] = twist_move[twist[rot][depth]][conj_move[m][16 * rot]];

      int tmp = sslice[rot][depth + 1];
      CCoord fsstwist = FSSTWIST(
        conj_flip[flip[rot][depth + 1]][SYM(sslice_sym[tmp])][COORD(sslice_sym[tmp])],
        COORD(sslice_sym[tmp]),
        conj_twist[twist[rot][depth + 1]][SYM(sslice_sym[tmp])]
      );

      prun[rot][depth + 1] = next_dist[prun[rot][depth]][getPrun3(fsstwist_prun3, fsstwist)];
      if (prun[rot][depth + 1] >= togo) {
        if (prun[rot][depth + 1] > togo)
          m = kAxisEnd[m];
        next = true;
        break;
      }
    }
    if (next)
      continue;

    int dist1;
    if (
      prun[0][depth + 1] != 0 && // the cube is already solved
      prun[0][depth + 1] == prun[1][depth + 1] && prun[1][depth + 1] == prun[2][depth + 1]
    ) {
      dist1 = prun[0][depth + 1] + 1; // optimization by Michiel de Bondt
    }
    else
      dist1 = std::max(prun[0][depth + 1], std::max(prun[1][depth + 1], prun[2][depth + 1]));

    if (dist1 < togo) {
      moves[depth] = m;
      optim(depth + 1, dist1, togo - 1);
      if (done)
        return;
    } else if (dist1 > togo)
      m = kAxisEnd[m]; // axis-skipping crucial in the optimal solver
  }

  if (depth > 0 && corners1_depth == depth)
    corners1_depth--;
  if (depth > 0 && uedges_depth == depth)
    uedges_depth--;
  if (depth > 0 && dedges_depth == depth)
    dedges_depth--;
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
  len = max_depth + 1; // inital reference value for pruning

  std::vector<std::thread> threads;
  for (int rot = 0; rot < 3; rot++) {
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
    return ret;
  }
  return 2;
}

int optim(const CubieCube &cube, int max_depth, int timelimit, std::vector<int> &sol1) {
  done = false;
  len = 21; // to check if a solution was found

  cont = false;
  ret = 0;
  std::thread timeout([timelimit]() {
    std::unique_lock<std::mutex> lock(wait);
    notify.wait_for(lock, std::chrono::seconds(timelimit), []{ return cont; });
    done = true;
    ret++;
  });

  for (int rot = 0; rot < 3; rot++) {
    CubieCube cube1;
    CubieCube tmp;
    mul(sym_cubes[16 * rot], cube, tmp);
    mul(tmp, sym_cubes[inv_sym[16 * rot]], cube1);

    flip[rot][0] = getFlip(cube1);
    sslice[rot][0] = getSSlice(cube1);
    twist[rot][0] = getTwist(cube1);

    prun[rot][0] = getFSSTwistDist(flip[rot][0], sslice[rot][0], twist[rot][0]);
  }

  uedges[0] = getUEdges(cube);
  dedges[0] = getDEdges(cube);
  corners1[0] = getCPerm(cube);
  corners1_depth = 0;
  uedges_depth = 0;
  dedges_depth = 0;

  int dist;
  if (prun[0][0] != 0 && prun[0][0] == prun[1][0] && prun[1][0] == prun[2][0])
    dist = prun[0][0] + 1;
  else
    dist = std::max(prun[0][0], std::max(prun[1][0], prun[2][0]));

  for (int togo = dist; togo <= max_depth; togo++)
    optim(0, dist, togo);

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
    return 0;
  }
  return 2 - ret;
}

std::vector<int> scramble(int timelimit) {
  std::vector<int> scramble;
  CubieCube cube;
  randomize(cube);
  twophase(cube, -1, timelimit, scramble);
  std::reverse(scramble.begin(), scramble.end());
  for (int i = 0; i < scramble.size(); i++)
    scramble[i] = kInvMove[scramble[i]];
  return scramble;
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
  initCornersSym();

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
    tmp = fwrite(cornud_prun3, sizeof(uint64_t), N_CORNUD / 32 + 1, f);
    tmp = fwrite(cornslice_prun, sizeof(uint8_t), N_CORNSLICE, f);
  } else {
    fstwist_prun3 = new uint64_t[N_FSTWIST / 32 + 1];
    cornud_prun3 = new uint64_t[N_CORNUD / 32 + 1];
    cornslice_prun = new uint8_t[N_CORNSLICE];
    int tmp = fread(fstwist_prun3, sizeof(uint64_t), N_FSTWIST / 32 + 1, f);
    tmp = fread(cornud_prun3, sizeof(uint64_t), N_CORNUD / 32 + 1, f);
    tmp = fread(cornslice_prun, sizeof(uint8_t), N_CORNSLICE, f);
  }

  fclose(f);
}

void initOptim(bool file) {
  initTwistMove();
  initFlipMove();
  initSSliceMove();
  initUEdgesMove();
  initDEdgesMove();
  initCPermMove();

  initConjTwist();
  initSSliceSym();
  initConjFlip();

  if (!file) {
    initFSSTwistPrun3();
    return;
  }

  FILE *f = fopen(FILE_OPTIM, "rb");

  if (f == NULL) {
    initFSSTwistPrun3();
    f = fopen(FILE_OPTIM, "wb");
    int tmp = fwrite(fsstwist_prun3, sizeof(uint64_t), N_FSSTWIST / 32 + 1, f);
  } else {
    fsstwist_prun3 = new uint64_t[N_FSSTWIST / 32 + 1];
    int tmp = fread(fsstwist_prun3, sizeof(uint64_t), N_FSSTWIST / 32 + 1, f);
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

std::string optimStr(std::string s, int max_depth, int timelimit) {
  CubieCube cube;
  int err = faceToCubie(s, cube);
  if (err)
    return "FaceError " + std::to_string(err);
  if ((err = check(cube)))
    return "CubieError " + std::to_string(err);

  std::vector<int> sol;
  int ret = optim(cube, max_depth, timelimit, sol);
  return ret == 0 ? solToStr(sol) : "SolveError " + std::to_string(ret);
}

std::string scrambleStr(int timelimit) {
  return solToStr(scramble(timelimit));
}
