# twophase

This repository contains an efficient C++ implementation of Herbert Kociemba's two-phase algorithm for solving Rubik's cubes. 
The solver is based on the most current algorithmic ideas (see [`RubiksCube-TwophaseSolver`](https://github.com/hkociemba/RubiksCube-TwophaseSolver)) and furthermore includes various smaller optimizations and programming tricks (partly inspired by [`min2phase`](https://github.com/cs0x7f/min2phase)).
It features the full phase 1 and almost full phase 2 pruning tables with 16-way symmetry reduction, multithreaded parallel search in 6 directions as well as an optimal solving mode.

Even on moderate hardware, the solver usually manages to find 20 or less move solutions to random cubes in less than 1 millisecond on average.
This probably makes it one of the fastest versions to date.

## Usage

Usually you will probably want to use this as an API. 
The file `cube.h` provides 3 simple function for solving cubes with the twophase algorithm, optimally and for generating scrambles to uniform cubes.
Their interfaces are completely decoupled from any internal structures and work solely with `std::string`s.
A short example is shown below, refer to respective function documentation for details.

```C++
#include <string>
#include "solve.h"

int main() {
  initTwophase(); // Initialize twophase solver, may take a few seconds
  // Input cubes in face-cube representation, see `face.h` for details
  std::string cube = "UDFUURRLDBFLURRDRUUFLLFRFDBRBRLDBUDLRBBFLBBUDDFFDBUFLL";
  // Look for a solution with 20 or less moves and use at most 10ms computation time
  std::string sol = twophaseStr(cube, 20, 10);
  
  initOptim(); // Initialize optimal solver, may take a few minutes
  sol = optimStr(cube);
  
  // Generate a scrambling sequence leading to a uniformly random cube; take at most 10ms
  sol = scrambleStr(10);
}
```

A small CMD-application is also included. It provides functionality to run benchmarks, solve individual cubes and generate scrambles.
Run `./twophase` for more information.

**Note:** The solver does generally not use any platform specific features and should therefore in theory be portable. 
The makefile however links the library `pthread` and has only been tested on Linux.

## Performance

All benchmarks were performed with a fixed set of 10000 uniformly random cubes sampled cubes (`bench.cubes`) on an Intel(R) Core(TM) i7-4710HQ CPU @ 2.50GHz quad-core processor.

**Note:** The solver likely performs even better on stronger hardware as it is considerably bottlenecked by only 4 cores (it fully supports up to 6) and relatively slow RAM access speed (the lookup tables are too large to fit into the cache).

### Solving Time:

| Moves | Avg [ms] | Min [ms] | Max [ms] |
| ----- | -------- | -------- | -------- |
| 20    | 0.739    | 0.075    | 144.913  |
| 21    | 0.207    | 0.070    | 2.219    |
| 30    | 0.218    | 0.076    | 1.798    |

### Number of Moves:

| Time [ms] | Avg Moves | Solved [%] |
| --------- | --------- | ---------- |
| 1         | 19.564    | 99.98      |
| 5         | 19.261    | 100        |
| 10        | 19.154    | 100        |
| 50        | 18.835    | 100        |
| 100       | 18.745    | 100        |

### Optimal Solver:

The performance of the optimal solver highly depends on the particular cube to be solved. Typically, cubes with solutions up to depth 15 take a few milliseconds, with depth 16 - 18 a few seconds to minutes, and 19 - 20 several minutes to a few hours.
