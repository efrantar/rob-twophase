/**
 * Main program; Simple CMD interface for solving and scrambling
 */

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "cubie.h"
#include "face.h"
#include "moves.h"
#include "solve.h"

#define BENCHFILE "bench.cubes"
#define MAX_BENCHTIME 10000 // should be sufficient to solve even the worst cubes in `benchtime` mode
#define PRINT_EVERY 1000

// In interactive mode, we want to preinitalize the cache by solving some random cube
#define WARMUP_CUBE "UDFUURRLDBFLURRDRUUFLLFRFDBRBRLDBUDLRBBFLBBUDDFFDBUFLL"

int n_threads = 1;

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

void benchTime(const std::vector<CubieCube> &cubes, int moves) {
  std::vector<double> times;
  int failed = 0;

  for (int i = 0; i < cubes.size(); i++) {
    if (i % PRINT_EVERY == 0)
      std::cout << "Benchmarking ..." << std::endl;

    prepareSolve(n_threads);
    std::vector<int> sol;
    auto tick = std::chrono::high_resolution_clock::now();
    solve(cubes[i], moves, MAX_BENCHTIME, sol);
    auto tock = std::chrono::high_resolution_clock::now() - tick;

    if (!checkSol(cubes[i], sol) || sol.size() > moves)
      failed++;
    else
      times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(tock).count() / 1000.);
  }
  waitForFinish();

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
    prepareSolve(n_threads);
    solve(cubes[i], -1, time, sol);
    if (!checkSol(cubes[i], sol))
      failed++;
    else
      moves.push_back(sol.size());
  }
  waitForFinish();

  std::cout
    << "Average: " << std::accumulate(moves.begin(), moves.end(), 0.) / moves.size()
    << " Min: " << *std::min_element(moves.begin(), moves.end())
    << " Max: " << *std::max_element(moves.begin(), moves.end())
    << " Failed: " << failed << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    std::cout
      << "Usage:" << std::endl
      << "./twophase [-t N_THREADS] solve FACECUBE MAX_MOVES TIME" << std::endl
      << "./twophase [-t N_THREADS] scramble TIME" << std::endl
      << "./twophase [-t N_THREADS] interactive" << std::endl
      << "./twophase [-t N_THREADS] benchtime MAX_MOVES" << std::endl
      << "./twophase [-t N_THREADS] benchmoves TIME" << std::endl
    ;
    return 0;
  }

  int i = 1;
  if (std::string(argv[i]) == "-t") {
    n_threads = std::stoi(argv[i + 1]);
    i += 2;
  }
  std::string mode(argv[i]);

  std::cout << "Loading tables ..." << std::endl;
  auto tick = std::chrono::high_resolution_clock::now();
  initTwophase(true);
  auto tock = std::chrono::high_resolution_clock::now() - tick;
  std::cout
    << "Done. " << std::chrono::duration_cast<std::chrono::milliseconds>(tock).count() / 1000. << "s" << std::endl;

  if (mode == "solve") {
    auto tick = std::chrono::high_resolution_clock::now();
    std::cout << twophase(std::string(argv[i + 1]), std::stoi(argv[i + 2]), std::stoi(argv[i + 3])) << std::endl;
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::high_resolution_clock::now() - tick
    ).count() / 1000. << "ms" << std::endl;
  } else if (mode == "scramble")
    std::cout << scramble(std::stoi(argv[i + 1]), n_threads) << std::endl;
  else if (mode == "interactive") {
    std::ios_base::sync_with_stdio(false);
    twophase(WARMUP_CUBE, -1, 100, true, true, n_threads);

    std::string mode;
    std::string cube;
    int len;
    int timelimit;

    std::cout << "Enter >>solve FACECUBE MAX_MOVES TIME<< to solve or >>scramble TIME<< to scramble.\n";
    while (std::cin) {
      prepareSolve(n_threads);
      std::cout << "Ready!" << std::endl;
      std::cin >> mode;
      if (mode == "solve") {
        std::cin >> cube >> len >> timelimit;
        auto tick = std::chrono::high_resolution_clock::now();
        std::cout << twophase(cube, len, timelimit, false, false, n_threads) << std::endl;
        std::cout << std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now() - tick
        ).count() / 1000. << "ms" << std::endl;
      } else if (mode == "scramble") {
        std::cin >> timelimit;
        std::cout << scramble(timelimit, n_threads) << std::endl;
      } else
        std::cout << "Error." << std::endl;
    }
    waitForFinish();
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
        benchTime(cubes, std::stoi(argv[i + 1]));
      else if (mode == "benchmoves")
        benchMoves(cubes, std::stoi(argv[i + 1]));
  }

  return 0;
}
