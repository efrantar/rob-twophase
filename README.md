# rob-twophase v2.0

This is an extremely efficient Rubik's Cube solving algorithm designed particularly for the use by high-speed robots. At its core, it is a highly optimized C++ version of Herbert Kociemba's two-phase algorithm that combines many of the best tricks from the excellent implementations [`RubiksCube-TwophaseSolver`](https://github.com/hkociemba/RubiksCube-TwophaseSolver), [`min2phase`](https://github.com/cs0x7f/min2phase) and [`cube20src`](https://github.com/rokicki/cube20src) with some further improvements of my own. Additionally, it includes several features that (to the best of my knowledge) cannot be found elsewhere at the moment. First and foremost, `rob-twophase` is able to directly consider the mechanics of axial robots (i.e. that it can it can move opposite faces in parallel or that a 180-degree turn takes about twices as long as a 90-degree one) yielding 15-20% faster to execute solutions on average. Secondly, it supports single-solve multi-threading with an arbitrary number of threads yielding 10+ times speed-ups even on moderate hardware. Finally, it can search for multiple solutions at once allowing post-selection to consider additional execution parameters (like for instance turn transitions). If you are planning to challenge the official Guinness World Record for the fastest robot to solve a Rubik's Cube, `rob-twophase` is most likely the solver you will want to use.

**New in Version 2.0:**

* Considerably faster solving performance by less table decomposition and proper elimination of redundant maneuvers in QT-modes.
* Significantly faster initial table generation (also through better coordinates).
* Much better utilization of high thread-counts via the `-s` parameter.
* Return more than solution with `-n`.
* Automatically compress QT-mode solutions back to HT using the `-c` option.
* Cleaner code by major refactoring and elimination of questionable "optimizations".

## Usage

The easiest way to use `rob-twophase` is to use the small interactive CMD-utility that it compiles to via `make`. Interfacing with this tool via pipes to STDIN/STDOUT should be more than sufficient for most applications (this is also what I do for my own robots). If you want to interface with it directly in C++ best have a look at `src/main.cpp` to see how to use the internal solver eninge. The solving mode needs to be selected during compile time via compiler-flags (both for efficiency but also simplicity reasons). Simply add them to `CPPFLAGS` if you are using the provided `makefile`. `-DQT` solves in the quarter-turn metric (only 90-degree moves), `-DAX` in the axial metric (opposite faces can be manipulated at the same time) and `-DF5` uses only 5-faces (never turning the B-face). All three of those flags can be combined arbitrarily.

The CMD-program provides the following options:

* `-c` (default OFF): Compress solutions to AXHT. This is especially useful when solving in AXQT as properly merging move sequences like `D (U D)` is not entirely trivial without having all the proper move definitions at the ready.

* `-l` (default -1): Maximum solutions length. The search will stop once a solution of at most this length is found. With `-1` the solver will simply search for the full time-limit and eventually return the best solution found.

* `-m` (default 10): Time-limit in milliseconds.

* `-n` (default 1): Number of solutions to return, i.e. it will return the best `-n` solutions found of at most `-m` length.

* `-s` (default 1): Number of splits for every IDA-search task. This is an advanced parallelization parameter most relevant for high thread-count. As a very rough guide, choose it so that `-t / -s` is close to 6 (or close to 4 when using `-DF5`).

* `-t` (default 1): Number of threads. Best set this as the number of processor threads you have (typically number of cores times two), i.e. use hyper-threading.

* `-w` (default 0): Number of random warmup solves to perform on start-up to optimally prepare the cache for the robot solves that matter.

When first starting `rob-twophase`, it will generate fairly big tables which may take several seconds to minutes (see section below). Those are then persisted in files to make further start-ups very quick. After starting it can solve cubes by typing `solve FACECUBE` (see [`src/face.h`](https://github.com/efrantar/rob-twophase/blob/master/src/face.h) for a detailed documentation of the Kociemba's face-cube representation of a cube), generate scrambles with `scramble` or run benchmarks with `bench`. Note that the program is already designed to be directly used by robots (for example via pipe communication) and thereby of course also does things such as always preloading all threads to ensure maximum solving speed.

## Performance

All benchmarks were run on a stock AMD Ryzen 5 3600 (6 cores, 12 threads) processor (hence `-t 12 -s 2`) combined with standard clocked DDR4 memory and use exactly the same set of 10000 uniformly random cubes (file `bench.cubes`).

The first table gives for each solving mode (indicated by the compiler flags) the average solution length (number of moves) when running the solver with a timelimit of 10ms (`-m 10`) per cube in the varios metrics (half-turn HT, quarter-turn QT, axial half-turn AXHT and axial quarter-turn AXQT). The number in bold is the length in the metric that is being solved in (i.e. the number that is relevant), the other values are just given to illustrate the gains from directly solving in the appropriate metric.

| `-DQT` | `-DAX` | `-DF5` | HT        | QT        | AXHT      | AXQT      | Setup Time | Table Size |
| :----: | :----: | :----: | :-:       | :-:       | :--:      | :--:      | :--------: | :--------: |
| -      | -      | -      | **18.56** | *26.46*   | *17.00*   | *24.60*   | 38s        | 676MB      |
| YES    | -      | -      | *19.84*   | **24.22** | *17.97*   | *22.21*   | 26s        | 676MB      |
| -      | YES    | -      | *21.83*   | *31.05*   | **14.71** | *22.95*   | 98s        | 1.2GB      |
| YES    | YES    | -      | *22.76*   | *27.03*   | *16.40*   | **19.91** | 57s        | 1.2GB      |
| -      | -      | YES    | **20.61** | *29.51*   | *18.90*   | *27.50*   | 125s       | 2.7GB      |
| YES    | -      | YES    | *22.07*   | **26.98** | *19.99*   | *24.78*   | 85s        | 2.7GB      |
| -      | YES    | YES    | *23.65*   | *33.34*   | **16.85** | *25.50*   | 287s       | 4.9GB      |
| YES    | YES    | YES    | *25.08*   | *29.94*   | *18.38*   | **22.49** | 162s       | 4.9GB      |

Finally, a speed comparison with Tomas Rokicki's [`cube20src`](https://github.com/rokicki/cube20src) solver which used to prove that God's number is 20 (and is probably the next fastest available solver). `rob-twophase` learned many great tricks from this extremely optimized implementation. Furthermore, `cube20src` clearly remains the best choice for batch-solving a very large number of cubes. However, it does (at least as of right now) not support robot metrics or single-solve multi-threading. In general, `rob-twophase` is slightly faster in single-threaded mode (apart from shorter QT searches it seems) but dramatically faster when multi-threading. The table below gives the average solving time for different move-bounds and metrics (using again `bench.cubes`).

| Metric | Max #Moves | `rob-twophase` w. `-t 1` | `rob-twophase` w. `-t 12` | `cube20src` |
| :----: | :--------: | :----------------------: | :-----------------------: | :---------: |
| HT     | 20         | 00.48ms                  | **00.02ms**               | 00.55ms     |
| HT     | 19         | 41.51ms                  | **04.37ms**               | 53.04ms     |
| QT     | 26         | 11.20ms                  | **00.96ms**               | 10.26ms     |
| QT     | 25         | 71.43ms                  | **07.13ms**               | 77.94ms     |
