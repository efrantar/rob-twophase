#include "misc.h"

#include <random>
#include <stdint.h>

int fac[MISC_N];
int cnk[MISC_N][MISC_N];

bool initMisc() {
  fac[0] = 1;
  for (int i = 1; i < MISC_N; i++)
    fac[i] = fac[i - 1] * i;

  cnk[0][0] = 1;
  for (int n = 1; n < MISC_N; n++) {
    cnk[n][0] = 1;
    for (int k = 1; k <= n; k++)
      cnk[n][k] = cnk[n - 1][k] + cnk[n - 1][k - 1];
  }

  return true;
}
static bool inited = initMisc();

int mod(int a, int m) {
  return a > 0 ? a % m : (a % m + m) % m;
}

// Best-practice randomization (instead of simple rand())
std::random_device device;
std::mt19937 gen(device());
std::mt19937_64 gen64(device());

int rand(int max) {
  return std::uniform_int_distribution<int>(0, max)(gen);
}

uint64_t rand64(uint64_t max) {
  return std::uniform_int_distribution<uint64_t>(0, max)(gen64);
}
