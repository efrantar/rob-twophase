# twophase

This repository features a highly optimized C++ version of Herbert Kociemba's two-phase algorithm for finding close to optimal solutions to 3x3 Rubik's Cubes extremely quickly.
It combines many of the best tricks from the excellent implementations [`RubiksCube-TwophaseSolver`](https://github.com/hkociemba/RubiksCube-TwophaseSolver), [`min2phase`](https://github.com/cs0x7f/min2phase) and [`cube20src`](https://github.com/rokicki/cube20src) (fast coordinates, 16-way symmetry reduction, extended phase 1 pruning table with moves, phase 2 prechecking, parallel search in 6 directions and more) with several smaller improvements of my own aiming to push single solve performance on random cubes to the limit. 
Furthermore, it includes several options to search for solutions that are particularly efficient to execute for cube-solving robots (like preferring consecutive moves on opposite faces which can be performed in parallel and preferring quarter- over half-turns).

This is probably one of the "best" (in terms of average solving time for a fixed upper bound on the number of moves and average solution length when searching for a fixed amount of time) solvers you can find online at the moment (I am not aware of any faster implementation than this one in multi-threading mode).
It also seems to be the only one specifically tuned for cube-solving robots (i.e. that supports any combination of the axial, the quarter-turn and the 5-face metric).
However, all of these optimizations make the solver relatively resource intensive (especially for non-standard solving modes), hence if you just want to quickly (for human standards) find a decent solution to a Rubik's Cube, it might be a little overkill. 
On the other hand, if you are interested in the current state of the art techniques for solving a cube as quickly as possible or if you are planning to beat the Guinness World Record for the fastest robot to solve a Rubik's Cube, this solver is most likely what you want to look at.

## Usage

`main.cc` compiles to a little CMD-line utility that allows solving given cubes (modes `twophase` and `interactive`) as well as performing benchmarks (`benchtime` and `benchmoves`). The number of threads to use for solving can be specified with the `-t` option. The solver supports an arbitrary number of threads but note that the performance improvement might not scale linearly as at the memory throughput can bottleneck the parallelization. Simply run the program without any arguments to get the full calling syntax.

For a robot, you will most likely want to use the `interactive` mode. This mode preloads all tables as well as solver threads (to minimize threading overhead) and warms up the cache (by running some solves) before waiting for a cube to be passed over STDIN. Inputs are expected in Herbert Kociemba's face-cube representation (see `face.h` for detailled documentation). See also the example below. When specifying `MAX_MOVES` as `-1` the solver will simply search for the full `TIME` milliseconds and then return the shortest solution. Note that running the program for the first time will generate fairly large lookup tables with may take a few minutes, these are then saved to a file so that consecutive starts are very quick (typically at most a few seconds).

```
>> ./twophase -t 12 interactive
Loading tables ...
Done. 0.25s
Enter >>FACECUBE MAX_MOVES TIME<< to solve.
Ready!
>> UDFUURRLDBFLURRDRUUFLLFRFDBRBRLDBUDLRBBFLBBUDDFFDBUFLL 20 100
R2 F' D2 F' U2 R2 F R2 U2 F' D L F2 D' B' R' B R2 F U'
0.205ms
Ready!
>> UDFUURRLDBFLURRDRUUFLLFRFDBRBRLDBUDLRBBFLBBUDDFFDBUFLL -1 100
D F U2 L' B' U' L U B2 R' B' U R U2 D2 B2 U D2 F2
100.13ms
Ready!
^C
```

One of the key features of this program are the different solving modes. These have to be specified at compile times (both for simplicity but also for efficiency reasons). `-DQUARTER` solves in the quarter-turn metric (i.e. 180 degree turns are twice as expensive as quarter-turns), `-DAXIAL` in the axial-metric (consecutive moves on parallel faces can be performed at the same time; such moves are printed in brackets like `(U D)`) and `-DFACES5` produces solutions using only 5-faces (ignoring the back-face). All options are fully compatible and can be combined arbitrarily. The tables are always persisted in the same file named `twophase.tbl` hence only one version of the program may exist in a directory.

## Performance

All benchmarks were run on a stock *AMD Ryzen 5 3600 (6 cores, 12 threads)* processor (hence `-t 12`) combined with standard clocked *DDR4* memory and use exactly the same set of 10000 uniformly random cubes (file `bench.cubes`).

The first table gives the average solution length (number of moves) when running the solver with a timelimit of 10ms per cube. The average optimal solution length in the standard half-turn metric is approximately 17.7, hence the solutions found by the solver are on average only 1 move longer. While no optimal average is known for the other modes, the solutions can be expected to be slightly further away from the optimum as those metrics are considerably harder (which is also why no reference exists).

| `-DQUARTER` | `-DAXIAL`  | `-DFACES5` | Avg. #Moves | Setup Time | Table Size |
| :---------: | :--------: | :--------: | :---------: | :--------: | :--------: |
| -           | -          | -          | **18.62**   | 53s        | 676MB      |
| YES         | -          | -          | **24.28**   | 40s        | 676MB      |
| -           | YES        | -          | **14.98**   | 149s       | 1.2GB      |
| YES         | YES        | -          | **20.29**   | 81s        | 1.2GB      |
| -           | -          | YES        | **20.71**   | 175s       | 2.7GB      |
| YES         | -          | YES        | **27.19**   | 128s       | 2.7GB      |
| -           | YES        | YES        | **17.10**   | 455s       | 4.9GB      |
| YES         | YES        | YES        | **23.05**  | 242s       | 4.9GB      |

Finally, a speed comparison with Thomas Rokicki's [`cube20src`](https://github.com/rokicki/cube20src) solver which was also used to prove that God's number is 20. This extremely optimized implementation (from which `twophase` learned many many greats tricks) is certainly still the best choice for batch solving a large number of cubes. In single threaded mode it is also still around 25% faster. It does however (as of right now) not support multi-threaded search for individual cubes (or any of the additional solving options for robots). With this enabled `twophase` can perform considerably better (depending on the hardware of course), especially on harder solves where the threading overhead becomes irrelevant. The table below gives the average solving time for different move-bounds and metrics (using again `bench.cubes`).

| Metric       | Max #Moves | `twophase` w. `-t 12` | `cube20src` |
| ------       | :--------: | :-------------------: | :---------: |
| Half-Turn    | 20         | **0.15ms**            | 0.55ms      |
| Half-Turn    | 19         | **5.83ms**            | 53.04ms     |
| Quarter-Turn | 26         | **1.42ms**            | 10.26ms     |
| Quarter-Turn | 25         | **8.78ms**            | 77.94ms     |
