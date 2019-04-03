#include <iostream>
#include <stdlib.h>

#include "cubie.h"
#include "moves.h"
#include "coord.h"

int main() {
  initMoveCubes();

  initTwistMove();
  std::cout << "twist\n";
  initFlipMove();
  std::cout << "flip\n";
  initUDEdgesMove();
  std::cout << "udedges\n";
  initCornersMove();
  std::cout << "corners\n";

  return 0;
}

