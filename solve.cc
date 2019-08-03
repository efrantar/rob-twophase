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

double time1 = 0;

CubieCube cube1;
std::mutex ready_mutex;
std::condition_variable ready_var;
bool ready;

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

  int flip = getFlip(cube1);
  int twist = getTwist(cube1);
  Edges4 sslice(getSSlice(cube1));
  Edges4 uedges(getUEdges(cube1));
  Edges4 dedges(getDEdges(cube1));
  CPerm cperm(getCPerm(cube1));

  MoveMask mm;
  for (int togo = getFSTwistPrun(flip, sslice, twist, 0, mm); togo <= len; togo++) {
    phase1(0, togo, flip, twist, sslice, uedges, dedges, cperm, phase1_moves);
    /*
    if (!done) {
      mutex.lock();
      std::cout << inv_ << " " << rot << " " << togo << " " << probes << "\n";
      mutex.unlock();
    }
     */
    // std::this_thread::yield();
  }
  // mutex.lock();
  // std::cout << "probes: " << inv_ << " " << rot << " " << probes << "\n";
  // mutex.unlock();
}

void TwoPhaseSolver::phase1(
  int depth, int togo,
  int flip, int twist, const Edges4 &sslice, const Edges4 &uedges, const Edges4 &dedges, const CPerm &cperm,
  MoveMask movemask
) {
  if (done)
    return;

  if (togo == 0) {
    probes++;
    for (int togo1 = getCornEdPrun(cperm, uedges, dedges); togo1 < len - depth; togo1++) {
      if (phase2(depth, togo1, sslice, uedges, dedges, cperm, movemask))
        return;
    }
    return;
  }

  MoveMask mm;
  int dist = getFSTwistPrun(flip, sslice, twist, togo, mm);
  if (dist != togo && dist + togo < 5)
    return;
  mm &= movemask;

  depth++;
  togo--;

  Edges4 sslice1;
  Edges4 uedges1;
  Edges4 dedges1;
  CPerm cperm1;

  while (mm) {
    int m = ffsll(mm) - 1;
    mm &= mm - 1;

    int flip1 = move_flip[flip][m];
    int twist1 = move_twist[twist][m];
    moveSSlice(sslice, m, sslice1);
    moveEdges4(uedges, m, uedges1);
    moveEdges4(dedges, m, dedges1);
    moveCPerm(cperm, m, cperm1);
    moves[depth - 1] = m;

    phase1(depth, togo, flip1, twist1, sslice1, uedges1, dedges1, cperm1, next_moves[m]);
  }
}

bool TwoPhaseSolver::phase2(
  int depth, int togo,
  const Edges4 &sslice, const Edges4 &uedges, const Edges4 &dedges, const CPerm &cperm,
  MoveMask movemask
) {
  if (done)
    return true;

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

      if (depth <= max_depth) // keep searching if current solution exceeds max-depth
        done = true;
    }

    mutex.unlock();
    return true;
  }

  Edges4 sslice1;
  Edges4 uedges1;
  Edges4 dedges1;
  CPerm cperm1;

  MoveMask mm = phase2_moves & movemask;
  while (mm) {
    int m = ffsll(mm) - 1;
    mm &= mm - 1;

    moveSSlice(sslice, m, sslice1);
    moveEdges4(uedges, m, uedges1);
    moveEdges4(dedges, m, dedges1);
    moveCPerm(cperm, m, cperm1);

    if (getCornEdPrun(cperm1, uedges1, dedges1) < togo) {
      moves[depth] = m;
      if (phase2(depth + 1, togo - 1, sslice1, uedges1, dedges1, cperm1, next_moves[m]))
        return true;
    }
  }

  return false;
}

int twophase(const CubieCube &cube, int max_depth1, int timelimit, std::vector<int> &sol1) {
  ready = false;

  std::vector<std::thread> threads;
  for (int rot = 0; rot < 3; rot++) {
    #ifdef FACES5
      if (rot > 1)
        break;
    #endif
    for (int inv = 0; inv < 2; inv++) {
      threads.push_back(std::thread([rot, inv]() {
        {
          std::unique_lock<std::mutex> lock(ready_mutex);
          ready_var.wait(lock, []{ return ready; });
          ready_var.notify_one();
        }
        TwoPhaseSolver solver(rot, (bool) inv);
        solver.solve(cube1);
      }));
    }
  }

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


  cube1 = cube;
  {
    std::lock_guard<std::mutex> lock(ready_mutex);
    ready = true;
    ready_var.notify_one();
  }

  auto tick = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < threads.size(); i++)
    threads[i].join();
  time1 += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - tick).count() / 1000.;
  // std::cout << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - tick).count() / 1000. << "\n";

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
    return;
  }

  FILE *f = fopen(FILE_TWOPHASE, "rb");

  if (f == NULL) {
    initFSTwistPrun();
    initCornEdPrun();

    f = fopen(FILE_TWOPHASE, "wb");
    // Use `tmp` to avoid nasty warnings; not clean but we don't want to make this part too complicated
    int tmp = fwrite(fstwist_prun, sizeof(Prun), N_FSTWIST, f);
    tmp = fwrite(corned_prun, sizeof(uint8_t), N_CORNED, f);
  } else {
    fstwist_prun = new Prun[N_FSTWIST];
    corned_prun = new uint8_t[N_CORNED];
    int tmp = fread(fstwist_prun, sizeof(Prun), N_FSTWIST, f);
    tmp = fread(corned_prun, sizeof(uint8_t), N_CORNED, f);
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
