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

## Benchmarks

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
