#include <algorithm>
#include <chrono>
#include <fstream>
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

#define BENCHFILE "bench.cubes"
#define MAX_BENCHTIME 10000
#define MAX_SCRAMBLETIME 100
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

void benchTime(const std::vector<CubieCube> &cubes, int moves) {
  std::vector<double> times;
  int failed = 0;

  for (int i = 0; i < cubes.size(); i++) {
    if (i % PRINT_EVERY == 0)
      std::cout << "Benchmarking ...\n";

    auto tick = std::chrono::high_resolution_clock::now();
    std::vector<int> sol = twophase(cubes[i], moves, MAX_BENCHTIME);
    auto tock = std::chrono::high_resolution_clock::now() - tick;

    if (!checkSol(cubes[i], sol) || sol.size() > moves)
      failed++;
    else
      times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(tock).count() / 1000.);
  }

  std::cout
    << "Average: " << std::accumulate(times.begin(), times.end(), 0.) / times.size()
    << "ms Min: " << *std::min_element(times.begin(), times.end())
    << "ms Max: " << *std::max_element(times.begin(), times.end())
    << "ms Failed: " << failed << "\n";
}

void benchMoves(const std::vector<CubieCube> &cubes, int time) {
  std::vector<int> moves;
  int failed = 0;

  for (int i = 0; i < cubes.size(); i++) {
    if (i % PRINT_EVERY == 0)
      std::cout << "Benchmarking ...\n";
    std::vector<int> sol = twophase(cubes[i], -1, time);
    if (!checkSol(cubes[i], sol))
      failed++;
    else
      moves.push_back(sol.size());
  }

  std::cout
    << "Average: " << std::accumulate(moves.begin(), moves.end(), 0.) / moves.size()
    << " Min: " << *std::min_element(moves.begin(), moves.end())
    << " Max: " << *std::max_element(moves.begin(), moves.end())
    << " Failed: " << failed << "\n";
}

void err() {
  std::cout << "Call:\n"
    << "./twophase twophase FACECUBE MAX_MOVES TIME\n"
    << "./twophase optim FACECUBE\n"
    << "./twophase benchtime COUNT MAX_MOVES\n"
    << "./twophase benchmoves COUNT TIME\n";
  exit(0);
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    std::cout << "Call:\n"
      << "./twophase twophase FACECUBE MAX_MOVES TIME\n"
      << "./twophase optim FACECUBE\n"
      << "./twophase benchtime MAX_MOVES\n"
      << "./twophase benchmoves TIME\n"
      << "./twophase scramble COUNT\n";
    return 0;
  }
  std::string mode(argv[1]);

  std::cout << "Loading tables ...\n";
  auto tick = std::chrono::high_resolution_clock::now();
  if (mode == "optim")
    initOptim(true);
  else
    initTwophase(true);
  auto tock = std::chrono::high_resolution_clock::now() - tick;
  std::cout << "Done. " << std::chrono::duration_cast<std::chrono::milliseconds>(tock).count() / 1000. << "s\n";

  if (mode == "scramble") {
    int count = std::stoi(argv[2]);
    while (count-- > 0)
      printSol(scramble(MAX_SCRAMBLETIME));
  } else if (mode == "twophase") {
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
  } else if (mode == "optim") {
    // TODO
  } else {
    std::vector<CubieCube> cubes;

    std::ifstream file;
    file.open(BENCHFILE);
    std::string s;

    while (std::getline(file, s)) {
      CubieCube c;
      faceToCubie(s, c);
      cubes.push_back(c);
    }

    if (mode == "benchtime")
      benchTime(cubes, std::stoi(argv[2]));
    else if (mode == "benchmoves")
      benchMoves(cubes, std::stoi(argv[2]));
  }

  return 0;
}
