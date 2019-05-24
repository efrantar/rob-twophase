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

bool skip_move[N_MOVES][N_MOVES];
const int axis_end[] = {
  U3, U3, U3,
  R3, R3, R3,
  F3, F3, F3,
  D3, D3, D3,
  L3, L3, L3,
  B3, B3, B3
};

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

int moves[N];
int prun[3][N];
Coord flip[3][N];
Coord sslice[3][N];
Coord twist[3][N];
Coord uedges[N];
Coord dedges[N];
Coord corners1[N];
int cud_depth;

TwoPhaseSolver::TwoPhaseSolver(int rot1, bool inv1) {
  rot = rot1;
  inv = inv1;
}

void TwoPhaseSolver::solve(const CubieCube &cube) {
  count1 = 0;
  count2 = 0;

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
  for (int togo = dist; togo < len; togo++)
    phase1(0, dist, togo);
}

int TwoPhaseSolver::phase1(int depth, int dist, int togo) {
  count1++;

  if (done)
    return 0;
  else if (clock() > endtime) {
    done = true;
    return 0;
  }

  if (togo == 0) {
    for (int i = corners_depth + 1; i <= depth; i++)
      corners[i] = corners_move[corners[i - 1]][moves[i - 1]];
    if (depth > 0)
      corners_depth = depth - 1;

    int max_togo = std::min(len - 1 - depth, 10);
    if (cornslice_prun[CORNSLICE(corners[depth], sslice[depth])] > max_togo)
      return cornslice_prun[CORNSLICE(corners[depth], sslice[depth])] > max_togo + 1;

    for (int i = udedges_depth + 1; i <= depth; i++) {
      uedges[i] = uedges_move[uedges[i - 1]][moves[i - 1]];
      dedges[i] = dedges_move[dedges[i - 1]][moves[i - 1]];
    }
    if (depth > 0)
      udedges_depth = depth - 1;
    udedges[depth] = merge_udedges[uedges[depth]][dedges[depth] % 24];

    int dist1 = getCornUDDist(corners[depth], udedges[depth]);

    if (dist1 > max_togo + 1)
      return 1;
    for (int togo1 = dist1; togo1 <= max_togo; togo1++)
      phase2(depth, dist1, togo1);

    return 0;
  }

  for (int m = 0; m < N_MOVES; m++) {
    if (depth > 0 && skip_move[moves[depth - 1]][m])
      continue;
    if (dist == 0 && kIsPhase2Move[m])
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

    if (dist1 < togo) {
      moves[depth] = m;
      if (phase1(depth + 1, dist1, togo - 1) == 1)
        m = axis_end[m];
      if (done)
        return 0;
    }
  }

  if (depth > 0 && corners_depth == depth)
    corners_depth--;
  if (depth > 0 && udedges_depth == depth)
    udedges_depth--;
}

void TwoPhaseSolver::phase2(int depth, int dist, int togo) {
  count2++;

  if (done)
    return;
  else if (clock() > endtime) {
    done = true;
    return;
  }

  if (togo == 0) {
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
    if (std::max(dist1, tmp) < togo) {
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

  if (dist == 0) {
    for (int i = cud_depth + 1; i <= depth; i++) {
      corners1[i] = corners_move[corners1[i - 1]][moves[i - 1]];
      uedges[i] = uedges_move[uedges[i - 1]][moves[i - 1]];
      dedges[i] = dedges_move[dedges[i - 1]][moves[i - 1]];
    }
    if (depth > 0)
      cud_depth = depth - 1;

    if (corners1[depth] == 0 && uedges[depth] == 0 && dedges[depth] == DEDGES_SOLVED) {
      sol.resize(depth);
      for (int i = 0; i < depth; i++)
        sol[i] = moves[i];
      done = true;
    }

    return;
  }

  for (int m = 0; m < N_MOVES; m++) {
    if (depth > 0 && skip_move[moves[depth - 1]][m])
      continue;

    bool next = false;
    for (int rot = 0; rot < 3; rot++) {
      flip[rot][depth + 1] = flip_move[flip[rot][depth]][conj_move[m][16 * rot]];
      sslice[rot][depth + 1] = sslice_move[sslice[rot][depth]][conj_move[m][16 * rot]];
      twist[rot][depth + 1] = twist_move[twist[rot][depth]][conj_move[m][16 * rot]];

      int tmp = sslice[rot][depth + 1];
      CoordL fsstwist = FSSTWIST(
        conj_flip[flip[rot][depth + 1]][SYM(sslice_sym[tmp])][COORD(sslice_sym[tmp])],
        COORD(sslice_sym[tmp]),
        conj_twist[twist[rot][depth + 1]][SYM(sslice_sym[tmp])]
      );

      prun[rot][depth + 1] = next_dist[prun[rot][depth]][getPrun3(fsstwist_prun3, fsstwist)];
      if (prun[rot][depth + 1] >= togo) {
        if (prun[rot][depth + 1] > togo)
          m = axis_end[m];
        next = true;
        break;
      }
    }
    if (next)
      continue;

    int dist1;
    if (
      prun[0][depth + 1] != 0 &&
      prun[0][depth + 1] == prun[1][depth + 1] && prun[1][depth + 1] == prun[2][depth + 1]
    ) {
      dist1 = prun[0][depth + 1] + 1;
    }
    else
      dist1 = std::max(prun[0][depth + 1], std::max(prun[1][depth + 1], prun[2][depth + 1]));

    if (dist1 < togo) {
      moves[depth] = m;
      optim(depth + 1, dist1, togo - 1);
      if (done)
        return;
    } else if (dist1 > togo)
      m = axis_end[m];
  }

  if (depth > 0 && cud_depth == depth)
    cud_depth--;
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

std::vector<int> optim(const CubieCube &cube) {
  done = false;

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
  corners1[0] = getCorners(cube);
  cud_depth = 0;

  int dist;
  if (prun[0][0] != 0 && prun[0][0] == prun[1][0] && prun[1][0] == prun[2][0])
    dist = prun[0][0] + 1;
  else
    dist = std::max(prun[0][0], std::max(prun[1][0], prun[2][0]));

  for (int togo = dist; togo <= 20; togo++) {
    std::cout << togo << "\n";
    optim(0, dist, togo);
  }

  return sol;
}

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

  FILE *f = fopen(FILE_TWOPHASE, "rb");

  if (f == NULL) {
    initFSTwistPrun3();
    initCornUDPrun3();
    initCornSlicePrun();

    f = fopen(FILE_TWOPHASE, "wb");
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
  initCornersMove();

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
