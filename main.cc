#include <algorithm>
#include <ctime>
#include <iostream>
#include <numeric>
#include <stdlib.h>
#include <string>
#include <vector>

#include "cubie.h"
#include "coord.h"
#include "face.h"
#include "misc.h"
#include "moves.h"
#include "prun.h"
#include "solve.h"
#include "sym.h"

#define MAX_BENCHTIME 10000
#define PRINT_EVERY 1000

double tock(clock_t tick) {
  return double(clock() - tick) / CLOCKS_PER_SEC * 1000;
}

void printSol(const std::vector<int> &sol) {
  for (int i = 0; i < sol.size(); i++)
    std::cout << kMoveNames[sol[i]] << " ";
  std::cout << "(" << sol.size() << ")\n";
}

bool checkSol(const CubieCube &cube, const std::vector<int> &sol) {
  CubieCube cube1;
  CubieCube cube2;

  copy(cube, cube1);
  for (int m : sol) {
    mul(cube1, move_cubes[m], cube2);
    std::swap(cube1, cube2);
  }

  return cube1 == kSolvedCube;
}

void benchTime(int count, int max_moves) {
  std::vector<double> times(count);
  int failed = 0;

  for (int i = 0; i < count; i++) {
    if (i % PRINT_EVERY == 0)
      std::cout << "Benchmarking ...\n";

    clock_t tick = clock();
    CubieCube cube = randomCube();
    std::vector<int> sol = twophase(cube, max_moves, MAX_BENCHTIME);
    tick = tock(tick);

    if (!checkSol(cube, sol) || sol.size() > max_moves)
      failed++;
    else
      times[i] = tick;
  }

  std::cout
    << "Average: " << std::accumulate(times.begin(), times.end(), 0.) / times.size()
    << "ms Min: " << *std::min_element(times.begin(), times.end())
    << "ms Max: " << *std::max_element(times.begin(), times.end())
    << "ms Failed: " << failed << "\n";
}

void benchMoves(int count, int time) {
  std::vector<int> moves(count);
  int failed = 0;

  for (int i = 0; i < count; i++) {
    if (i % PRINT_EVERY == 0)
      std::cout << "Benchmarking ...\n";
    CubieCube cube = randomCube();
    std::vector<int> sol = twophase(cube, -1, time);
    if (!checkSol(cube, sol))
      failed++;
    else
      moves[i] = sol.size();
  }

  std::cout
    << "Average: " << std::accumulate(moves.begin(), moves.end(), 0.) / moves.size()
    << " Min: " << *std::min_element(moves.begin(), moves.end())
    << " Max: " << *std::max_element(moves.begin(), moves.end())
    << " Failed: " << failed << "\n";
}

int main(int argc, char *argv[]) {
  /*
  initTwistMove();
  initFlipMove();
  initSSliceMove();
  initUEdgesMove();
  initDEdgesMove();
  initUDEdgesMove();
  initCornersMove();
  initMergeUDEdges();

  clock_t tick = clock();
  initConjTwist();
  initConjUDEdges();
  initFlipSliceSym();
  initCornersSym();
  std::cout << tock(tick) << "\n";

  initFSTwistPrun3();

  std::cout << tock(tick) / 1000. << "\n";
  */

  if (argc != 4) {
    std::cout << "Call:\n"
      << "./twophase FACECUBE MAX_MOVES TIME\n"
      << "./twophase benchtime COUNT MAX_MOVES\n"
      << "./twophase benchmoves COUNT TIME\n";
    return 0;
  }

  std::cout << "Loading tables ...\n";
  initTwophase();
  std::cout << "Done.\n";

  std::string s = argv[1];
  if (s == "benchtime")
    benchTime(std::stoi(argv[2]), std::stoi(argv[3]));
  else if (s == "benchmoves")
    benchMoves(std::stoi(argv[2]), std::stoi(argv[3]));
  else {
    CubieCube cube;
    int tmp = faceToCubie(std::string(argv[1]), cube);
    if (tmp) {
      std::cout << "Facecube Error: " << tmp << "\n";
      return 0;
    }
    tmp = check(cube);
    if (tmp) {
      std::cout << "Cubiecube Error: " << tmp << "\n";
      return 0;
    }
    printSol(twophase(cube, std::stoi(argv[2]), std::stoi(argv[3])));
  }

  return 0;
}
