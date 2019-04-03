#include <iostream>
#include <stdlib.h>

#include "cubie.h"
#include "misc.h"
#include "moves.h"
#include "coord.h"

int main() {
  initMisc();
  initMoveCubes();

  initTwistMove();
  std::cout << "twist\n";
  initFlipMove();
  std::cout << "flip\n";
  initSliceMove();
  std::cout << "slice\n";
  initUEdgesMove();
  std::cout << "uedges\n";
  initDEdgesMove();
  std::cout << "dedges\n";
  initUDEdgesMove();
  std::cout << "udedges\n";
  initCornersMove();
  std::cout << "corners\n";

  initMergeUEdgesDEdges();
  std::cout << "merge\n";

  return 0;
}

