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
  }

  std::cout
    << "Average: " << std::accumulate(moves.begin(), moves.end(), 0.) / moves.size()
    << " Min: " << *std::min_element(moves.begin(), moves.end())
    << " Max: " << *std::max_element(moves.begin(), moves.end())
    << " Failed: " << failed << std::endl;
}

int rec(int depth, int i1, int i2, int i3, int i4, int i5) {
  if (depth == 9)
    return 1;
  int tmp = i1 + i2 + i3 + i4 + i5;
  int i11 = i1 + 1;
  int i21 = i2 + 1;
  int i31 = i3 + 1;
  int i41 = i4 + 1;
  int i51 = i5 + 1;
  for (int i = 0; i < 10; i++)
    tmp += rec(depth + 1, i11, i21, i31, i41, i51);
  return tmp;
}

int i1[10];
int i2[10];
int i3[10];
int i4[10];
int i5[10];

int rec1(int depth) {
  if (depth == 9)
    return 1;
  int tmp = i1[depth] + i2[depth] + i3[depth] + i4[depth] + i5[depth];
  i1[depth + 1] = i1[depth] + 1;
  i2[depth + 1] = i2[depth] + 1;
  i3[depth + 1] = i3[depth] + 1;
  i4[depth + 1] = i4[depth] + 1;
  i5[depth + 1] = i5[depth] + 1;
  for (int i = 0; i < 10; i++)
    tmp += rec1(depth + 1);
  return tmp;
}

int main(int argc, char *argv[]) {
  /*
  auto tick = std::chrono::high_resolution_clock::now();
  std::cout << rec1(0) << "\n";
  std::cout << std::chrono::duration_cast<std::chrono::microseconds>(
    std::chrono::high_resolution_clock::now() - tick
  ).count() / 1000. << "ms" << std::endl;
*/
  /*
  int SIZE = 50000;

  int *test = new int[SIZE];
  for (int i = 0; i < SIZE; i++)
    test[i] = rand();

  int indices[1000000];
  for (int i = 0; i < 1000000; i++)
    indices[i] = rand() % SIZE;

  auto tick = std::chrono::high_resolution_clock::now();

  int tmp = 0;
  for (int i = 0; i < 1000000; i++)
    tmp += i;

  if (tmp == 0)
    std::cout << tmp << "\n";

  std::cout << std::chrono::duration_cast<std::chrono::microseconds>(
    std::chrono::high_resolution_clock::now() - tick
  ).count() / 1000. << "ms" << std::endl;

  int sum = 0;
  for (int i : indices)
    sum += test[i];

  if (sum == 0)
    std::cout << sum << "\n";

  std::cout << std::chrono::duration_cast<std::chrono::microseconds>(
    std::chrono::high_resolution_clock::now() - tick
  ).count() / 1000. << "ms" << std::endl;
  exit(0);
  */

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
    std::cout << time1 / 10000 << "\n";
  }

  return 0;
}
