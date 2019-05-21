#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <stdlib.h>
#include <string>
#include <vector>
#include <thread>

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

    auto tick = std::chrono::high_resolution_clock::now();
    CubieCube cube = randomCube();
    std::vector<int> sol = twophase(cube, max_moves, MAX_BENCHTIME);
    auto tock = std::chrono::high_resolution_clock::now() - tick;

    if (!checkSol(cube, sol) || sol.size() > max_moves)
      failed++;
    else
      times[i] = std::chrono::duration_cast<std::chrono::microseconds>(tock).count() / 1000.;
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
  initOptim(true);

  auto tick = std::chrono::high_resolution_clock::now();
  CubieCube cube = randomCube();
  std::cout << "Started solving ...\n";
  std::vector<int> sol = optim(cube);
  auto tock = std::chrono::high_resolution_clock::now() - tick;
  std::cout << std::chrono::duration_cast<std::chrono::microseconds>(tock).count() / 1000. << "\n";

  if (checkSol(cube, sol))
    std::cout << sol.size() << "\n";
  else
    std::cout << "error\n";
    */

  if (argc == 1) {
    std::cout << "Call:\n"
      << "./twophase FACECUBE MAX_MOVES TIME\n"
      << "./twophase benchtime COUNT MAX_MOVES\n"
      << "./twophase benchmoves COUNT TIME\n";
    return 0;
  }

  std::cout << "Loading tables ...\n";
  initTwophase(true);
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
