#include <algorithm>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <vector>
#include <numeric>

#include "cubie.h"
#include "coord.h"
#include "face.h"
#include "move.h"
#include "prun.h"
#include "solve.h"
#include "sym.h"

const std::string BENCH_FILE = "bench.cubes";

void usage() {
  std::cout << "Usage: ./twophase "
    << "[-c] [-l MAX_LEN = 1] [-m MILLIS = 10] [-n N_SOLS = 1] [-s N_SPLITS = 1] [-t N_THREADS = 1] [-w N_WARMUPS = 0]"
  << std::endl;
  exit(1);
}

void init() {
  auto tick = std::chrono::high_resolution_clock::now();
  std::cout << "Loading tables ..." << std::endl;

  face::init();
  move::init();
  coord::init();
  sym::init();
  if (prun::init(true)) {
    std::cout << "Error." << std::endl;
    exit(1);
  }

  std::cout << "Done. " << std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::high_resolution_clock::now() - tick
  ).count() / 1000. << "s" << std::endl << std::endl;
}

void warmup(solve::Engine& solver, int count) {
  if (count == 0)
    return;

  std::cout << "Warming up ..." << std::endl;
  cubie::cube c;
  std::vector<std::vector<int>> sols;
  for (int i = 0; i < count; i++) {
    cubie::shuffle(c);
    solver.prepare();
    solver.solve(c, sols);
    solver.finish();
    std::cout << i << std::endl;
  }
  std::cout << "Done." << std::endl << std::endl;
}

bool check(const cubie::cube &c, const std::vector<int>& sol) {
  cubie::cube c1;
  cubie::cube c2;

  c1 = c;
  for (int m : sol) {
    cubie::mul(c1, move::cubes[m], c2);
    std::swap(c1, c2);
  }

  return c1 == cubie::SOLVED_CUBE;
}

double mean(const std::vector<std::vector<int>>& sols, int (*len)(const std::vector<int>&)) {
  double total = 0;
  for (auto& sol : sols)
    total += len(sol);
  return total / sols.size();
}

int main(int argc, char *argv[]) {
  int n_threads = 1;
  int tlim = 10;
  int n_sols = 1;
  int max_len = -1;
  int n_splits = 1;
  bool compress = false;
  int n_warmups = 0;

  try {
    int opt;
    while ((opt = getopt(argc, argv, "cl:m:n:s:t:w:")) != -1) {
      switch (opt) {
        case 'c':
          compress = true;
          break;
        case 'l':
          max_len = std::stoi(optarg);
          break;
        case 'm':
          tlim = std::stoi(optarg);
          break;
        case 'n':
          if ((n_sols = std::stoi(optarg)) <= 0) {
            std::cout << "Error: Number of solutions (-n) must be >= 1." << std::endl;
            return 1;
          }
          break;
        case 's':
          if ((n_splits = std::stoi(optarg)) <= 0) {
            std::cout << "Error: Number of job splits (-s) must be >= 1." << std::endl;
            return 1;
          }
          break;
        case 't':
          if ((n_threads = std::stoi(optarg)) <= 0) {
            std::cout << "Error: Number of solver threads (-t) must be >= 1." << std::endl;
            return 1;
          }
          break;
        case 'w':
          if ((n_warmups = std::stoi(optarg)) <= 0) {
            std::cout << "Error: Number of warmup solves (-w) must be >= 0." << std::endl;
            return 1;
          }
          break;
        default:
          usage();
      }
    }
  } catch (...) { // catch any integer conversion errors
    usage();
  }

  std::cout << "This is rob-twophase v2.0; copyright Elias Frantar 2020." << std::endl << std::endl;
  init();
  solve::Engine solver(n_threads, tlim, n_sols, max_len, n_splits);
  warmup(solver, n_warmups);

  std::cout << "Enter >>solve FACECUBE<< to solve, >>scramble<< to scramble or >>bench<< to benchmark." << std::endl << std::endl;

  std::string mode;
  while (std::cin) {
    solver.prepare();
    std::cout << "Ready!" << std::endl;

    std::cin >> mode;
    if (mode == "bench") {
      try {
        std::ifstream fstream;
        fstream.open(BENCH_FILE);

        std::string s;
        std::vector<cubie::cube> cubes;
        while (std::getline(fstream, s)) {
          cubie::cube c;
          face::to_cubie(s, c);
          cubes.push_back(c);
        }
        if (cubes.size() == 0) {
          std::cout << "Error." << std::endl;
          continue;
        }

        std::vector<std::vector<int>> sols;
        std::vector<int> times(cubes.size());
        int failed = 0;

        std::cout << "Benchmarking ..." << std::endl;
        for (int i = 0; i < cubes.size(); i++) {
          std::cout << i << std::endl;

          solver.prepare();
          auto tick = std::chrono::high_resolution_clock::now();
          std::vector<std::vector<int>> tmp;
          solver.solve(cubes[i], tmp);
          times[i] = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - tick
          ).count() / 1000.;
          solver.finish();

          if (tmp.size() == 0 || !check(cubes[i], tmp[0])) {
            std::cout << face::from_cubie(cubes[i]) << std::endl;
            failed++;
          }
          else
            sols.push_back(tmp[0]);
        }

        std::cout << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        std::cout << "Avg. Time: " << std::accumulate(times.begin(), times.end(), 0.) / times.size() << " ms" << std::endl;
        std::cout << "Avg. Moves: "
          << mean(sols, move::len_ht) << " (HT), "
          << mean(sols, move::len_qt) << " (QT), "
          << mean(sols, move::len_axht) << " (AXHT), "
          << mean(sols, move::len_axqt) << " (AXQT)"
        << std::endl;

        int freq[100];
        int min = 100;
        int max = 0;
        for (auto& sol : sols) {
          freq[sol.size()]++;
          min = std::min(min, (int) sol.size()); // errors without casting ...
          max = std::max(max, (int) sol.size());
        }

        std::cout << std::endl;
        std::cout << "Distribution:" << std::endl;
        for (int len = min; len <= max; len++)
          std::cout << len << ": " << freq[len] << std::endl;
        std::cout << std::endl;
      } catch (...) { // any file reading errors
        std::cout << "Error." << std::endl;
        continue;
      }
    } else {
      cubie::cube c;
      std::vector<std::vector<int>> sols;

      if (mode == "solve") {
        std::string fcube;
        std::cin >> fcube;
        int err = face::to_cubie(fcube, c);
        if (err != 0) {
          std::cout << "Face-error " << err << "." << std::endl;
          continue;
        }
        err = cubie::check(c);
        if (err != 0) {
          std::cout << "Cubie-error " << err << "." << std::endl;
          continue;
        }
      } else if (mode == "scramble") {
        cubie::shuffle(c);
        std::cout << face::from_cubie(c) << std::endl;
      } else {
        std::cout << "Error." << std::endl;
        continue;
      }

      auto tick = std::chrono::high_resolution_clock::now();
      solver.solve(c, sols);
      std::cout << std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - tick
      ).count() / 1000. << "ms" << std::endl;

      for (std::vector<int>& sol : sols) {
        int len = sol.size(); // always print uncompressed length
        if (compress)
          std::cout << move::compress(sol) << " ";
        else {
          for (int m : sol)
            std::cout << move::names[m] << " ";
        }
        std::cout << "(" << len << ")" << std::endl;
      }
    }
  }
  solver.finish(); // clean exit

  return 0;
}
