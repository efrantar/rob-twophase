#include "solve.h"

#include "coord.h"
#include "cubie.h"
#include "move.h"
#include "sym.h"
#include "prun.h"

std::string

std::string Solver::solve(const CubieCube &cube) {
  flip[0] = getFlip(cube);
  twist[0] = getTwist(cube);

  CubieCube tmp;
  copy(cube, tmp);
  slicesorted[0] = getSliceSorted(tmp);
  copy(cube, tmp);
  uedges[0] = getUEdges(tmp);
  copy(cube, tmp);
  dedges[0] = getDEdges(tmp);

  int dist = getDepthFSSymTwistPrun3(flip[0], slice[0], twist[0]);
  for (int limit = dist; limit < max_depth; limit++)
    phase1(0, dist, limit);

  std::string s;
  for (int i = 0; i < sol.size(); i++) {
    s += kMoveNames[sol[i]];
    if (i != sol.size() - 1)
      s += " ";
  }
}

std::void Solver::phase1(int depth, int dist, int limit) {
  if (found)
    return;

  if (depth == limit) {
    if (depth > 0 && kPairMove[moves[depth - 1]])
      corners[depth] = corners_move[corners[depth]][moves[depth - 1] - 1];
    else {
      for (int i = 1; i <= depth; i++)
        corners[i] = corners_move[corners[i-1]][moves[i-1]];
    }

    int limit1 = depth + min(
      cornersslice_prun[corners][SS_SLICE(slicesorted[depth])], 11
    );
    if (limit1 >= 
  }

  for (int m = 0; m < N_MOVES; m++) {
    if (dist == 0 && limit < 5 && kIsPhase2Move[m])
      continue;
    if (depth > 0 && skip[moves[depth - 1]][m])
      continue;

    flip[depth + 1] = flip_move[flip[depth]][m];
    slicesorted[depth + 1] = slicesorted_move[slicesorted[depth]][m];
    twist[depth + 1] = twist_move[twist[depth]][m];

    LargeCoord flipslice = FLIPSLICE(flip[depth + 1], slice[depth + 1]);
    LargeCoord fssymtwist = FSSYMTWIST(
      flipslice_sym[flipslice], conj_twist[twist[depth + 1]][flipslice_sym_sym[flipslice]]
    );
    int dist1 = next_dist[dist][getFSSymTwistPrun3(fssymtwist)];

    if (depth + dist1 >= limit)
      continue;

    moves[depth] = m;
    phase1(depth + 1, dist1, limit);
  }
}


