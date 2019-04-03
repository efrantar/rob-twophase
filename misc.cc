#include "misc.h"

int fac[MISC_N];
int cnk[MISC_N][MISC_N] = {};

void initMisc() {
  fac[0] = 1;
  for (int i = 1; i < MISC_N; i++)
    fac[i] = fac[i - 1] * i;

  for (int n = 1; n < MISC_N; n++) {
    cnk[n][0] = 1;
    for (int k = 1; k <= n; k++) {
       cnk[n][k] = cnk[n - 1][k] + cnk[n - 1][k - 1];
    }
  }
}

