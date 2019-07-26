/**
 * Main program; Simple CMD interface for benchmarks, solving and scrambling
 */

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
#include "face.h"
#include "moves.h"
#include "solve.h"

#define BENCHFILE "bench.cubes"
#define MAX_BENCHTIME 10000 // even the worst cubes should not take much more than a few 100ms to solve in 20 moves
#define PRINT_EVERY 1000

bool checkSol(const CubieCube &cube, const std::vector<int> &sol) {
  CubieCube cube1;
  CubieCube cube2;

  cube1 = cube;
  for (int m : sol) {
    if (m >= N_MOVES)
      return false;
    mul(cube1, move_cubes[m], cube2);
    std::swap(cube1, cube2);
  }

  return cube1 == kSolvedCube;
}

/*
int cost(const std::vector<int> &sol) {
  int cost = 0;

  int i;
  for (i = 0; i < sol.size() - 1; i++) {
    if (abs(sol[i] / 3 - sol[i + 1] / 3) == 3) {
      cost += std::max(sol[i] % 3 == 1 ? 2 : 1, sol[i + 1] % 3 == 1 ? 2 : 1);
      i++;
    } else
      cost += sol[i] % 3 == 1 ? 2 : 1;
  }
  if (i < sol.size())
    cost += sol[i] % 3 == 1 ? 2 : 1;

  return cost;
}
 */

void benchTime(const std::vector<CubieCube> &cubes, int moves) {
  std::vector<double> times;
  int failed = 0;

  for (int i = 0; i < cubes.size(); i++) {
    if (i % PRINT_EVERY == 0)
      std::cout << "Benchmarking ..." << std::endl;

    auto tick = std::chrono::high_resolution_clock::now();
    std::vector<int> sol;
    twophase(cubes[i], moves, MAX_BENCHTIME, sol);
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
    << "ms Failed: " << failed << std::endl;
}

void benchMoves(const std::vector<CubieCube> &cubes, int time) {
  std::vector<int> moves;
  int failed = 0;

  for (int i = 0; i < cubes.size(); i++) {
    if (i % PRINT_EVERY == 0)
      std::cout << "Benchmarking ..." << std::endl;
    std::vector<int> sol;
    twophase(cubes[i], -1, time, sol);
    if (!checkSol(cubes[i], sol))
      failed++;
    else
      moves.push_back(sol.size());
      // moves.push_back(cost(sol));
  }

  std::cout
    << "Average: " << std::accumulate(moves.begin(), moves.end(), 0.) / moves.size()
    << " Min: " << *std::min_element(moves.begin(), moves.end())
    << " Max: " << *std::max_element(moves.begin(), moves.end())
    << " Failed: " << failed << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    std::cout
      << "Call:" << std::endl
      << "./twophase twophase FACECUBE MAX_MOVES TIME" << std::endl
      << "./twophase benchtime MAX_MOVES" << std::endl
      << "./twophase benchmoves TIME" << std::endl
    ;
    return 0;
  }
  std::string mode(argv[1]);

  std::cout << "Loading tables ..." << std::endl;
  auto tick = std::chrono::high_resolution_clock::now();
  initTwophase(true);
  auto tock = std::chrono::high_resolution_clock::now() - tick;
  std::cout
    << "Done. " << std::chrono::duration_cast<std::chrono::milliseconds>(tock).count() / 1000. << "s" << std::endl;

  if (mode == "twophase") {
    auto tick = std::chrono::high_resolution_clock::now();
    if (mode == "twophase") {
      std::cout << twophaseStr(std::string(argv[2]), std::stoi(argv[3]), std::stoi(argv[4])) << std::endl;
      std::cout << std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - tick
      ).count() / 1000. << "ms" << std::endl;
    }
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
