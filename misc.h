#ifndef MISC_H_
#define MISC_H_

#include <stdint.h>

#define MISC_N 13

extern int fac[MISC_N];
extern int cnk[MISC_N][MISC_N];

int mod(int a, int m);
int rand(int max);
uint64_t randLong(uint64_t max);

void initMisc();

#endif

