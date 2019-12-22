#include <iostream>

#include "coord.h"
#include "cubie.h"
#include "move.h"

inline void ok() { std::cout << "Ok." << std::endl; }
inline void error() { std::cout << "Error." << std::endl; }

void test_cubie() {
  std::cout << "Testing cubie level ..." << std::endl;
  cubie::cube c = cubie::SOLVED_CUBE;

  cubie::cube tmp1, tmp2;
  cubie::inv(c, tmp1);
  if (c != tmp1)
    error();
  cubie::mul(c, tmp1, tmp2);
  if (c != tmp2)
    error();

  cubie::shuffle(c);
  cubie::inv(c, tmp1);
  cubie::mul(c, tmp1, tmp2);
  if (tmp2 != cubie::SOLVED_CUBE)
    error();

  for (int i = 0; i < 100; i++) {
    cubie::shuffle(c);
    if (cubie::check(c) != 0)
      error();
  }
  cubie::shuffle(c);
  std::swap(c.cperm[0], c.cperm[1]);
  if (check(c) == 0)
    error();

  ok();
}

void test_getset(int (*get_coord)(const cubie::cube&), void (*set_coord)(cubie::cube&, int), int count) {
  cubie::cube c;
  for (int i = 0; i < count; i++) {
    set_coord(c, i);
    if (get_coord(c) != i)
      error();
  }
  ok();
}

void test_coord() {
  std::cout << "Testing coord level ..." << std::endl;
  test_getset(coord::get_twist, coord::set_twist, coord::N_TWIST);
  test_getset(coord::get_flip, coord::set_flip, coord::N_FLIP);
  test_getset(coord::get_slice, coord::set_slice, coord::N_SLICE);
  test_getset(coord::get_uedges, coord::set_uedges, coord::N_UEDGES);
  test_getset(coord::get_dedges, coord::set_dedges, coord::N_DEDGES);
  test_getset(coord::get_corners, coord::set_corners, coord::N_CORNERS);

  test_getset(coord::get_slice1, coord::set_slice1, coord::N_SLICE1);
  test_getset(coord::get_udedges2, coord::set_udedges2, coord::N_UDEDGES2);
}

void test_move() {
  std::cout << "Testing move level ..." << std::endl;

  cubie::cube c;
  for (int m = 0; m < move::COUNT; m++) {
    if (move::inv[move::inv[m]] != m)
      error();
    cubie::mul(move::cubes[m], move::cubes[move::inv[m]], c);
    if (c != cubie::SOLVED_CUBE)
      error();
    cubie::mul(move::cubes[move::inv[m]], move::cubes[m], c);
    if (c != cubie::SOLVED_CUBE)
      error();
  }
  ok();


}

int main() {
  test_cubie();
  test_coord();
  test_move();
  return 0;
}
