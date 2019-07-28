# twophase

This repository features a highly optimized C++ implementation of Herbert Kociemba's two-phase algorithm for finding close to optimal solutions to 3x3 Rubik's Cubes extremely quickly.
It combines essentially all of the best tricks
from [`RubiksCube-TwophaseSolver`](https://github.com/hkociemba/RubiksCube-TwophaseSolver), [`min2phase`](https://github.com/cs0x7f/min2phase) and [`cube20src`](https://github.com/rokicki/cube20src) (fast coordinates, 16-way symmetry reduction, huge phase 1 pruning table with moves, phase 2 prechecking, 6-thread parallel search and more) with several smaller improvements of my own aiming to push single solve performance on random cubes to the limit. 
Furthermore, it includes several options to search for solutions that are particularly efficient to execute for cube-solving robots (like preferring consecutive moves on opposite faces which can be performed in parallel and preferring quarter- over half-turns).

As far as I know, this is the "best" (in terms of average solving time for a fixed upper bound on the number of moves and average solution length when searching for a fixed amount of time) solver you can find online at the moment. 
It also seems to be the only one specifically tuned for cube-solving robots. 
However, all of these optimizations make the solver relatively resource intensive (especially for non-standard solving modes), hence if you just want to quickly (for human standards) find a decent solution to a Rubik's Cube, it might be a little overkill. 
On the other hand, if you are interested in the current state of the art techniques for solving a cube as quickly as possible or if you are planning to beat the Guinness World Record for the fastest robot to solve a Rubik's Cube, this solver is most likely what you want to look at.

## Performance

## Usage
