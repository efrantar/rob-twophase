#include "misc.h"

#include <random>
#include <stdint.h>

int fac[MISC_N];
int cnk[MISC_N][MISC_N] = {};

int mod(int a, int m) {
  return a > 0 ? a % m : (a % m + m) % m;
}

/* TODO: make the random generation cleaner */

int rand(int max) {
  return std::rand() % max;
}

uint64_t randLong(uint64_t max) {
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<uint64_t> dis;
  return dis(gen);
}

void initMisc() {
  fac[0] = 1;
  for (int i = 1; i < MISC_N; i++)
    fac[i] = fac[i - 1] * i;

  cnk[0][0] = 1;
  for (int n = 1; n < MISC_N; n++) {
    cnk[n][0] = 1;
    for (int k = 1; k <= n; k++)
       cnk[n][k] = cnk[n - 1][k] + cnk[n - 1][k - 1];
  }
}

