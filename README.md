# twophase

This repository contains an efficient C++ implementation of Herbert Kociemba's two-phase algorithm for solving Rubik's cubes. 
The solver is based on the most current algorithmic ideas (see [`RubiksCube-TwophaseSolver`](https://github.com/hkociemba/RubiksCube-TwophaseSolver)) and furthermore includes various smaller optimizations and programming tricks (partly inspired by [`min2phase`](https://github.com/cs0x7f/min2phase)).
It features the full phase 1 and almost full phase 2 pruning tables with 16-way symmetry reduction, multithreaded parallel search in 6 directions as well as an optimal solving mode.

Even on moderate hardware, the solver usually manages to find 20 or less move solutions to random cubes in less than 1 millisecond on average.
This probably makes it one of the fastest versions available at the moment.

## Usage

Usually you will probably want to use this as an API. 
The file `cube.h` provides 3 simple function for solving cubes with the twophase algorithm, optimally and for generating scrambles to unfiorm cubes.
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

A small CMD-application is also includes. It provides functionality to run benchmarks, solve individual cubes and generate scrambles.
Run `./twophase` for more information.

**Note:** The solver does generally not use any platform specific features and should therefore in theory be portable. 
The makefile however links the library `pthread` and has only been tested on Linux.

## Benchmarks

