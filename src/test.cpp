/**
 * Tests & sanity checks for individual parts of the solver; very useful during development.
 */

#include <bitset>
#include <chrono>
#include <iostream>
#include <strings.h>

#include "coord.h"
#include "cubie.h"
#include "move.h"
#include "prun.h"
#include "sym.h"

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

void test_movecoord(uint16_t move_coord[][move::COUNT], int n_coord, move::mask moves = move::p1mask | move::p2mask) {
  for (int coord = 0; coord < n_coord; coord++) {
    for (; moves; moves &= moves - 1) {
      int m = ffsll(moves) - 1;
      if (move_coord[move_coord[coord][m]][move::inv[m]] != coord)
        error();
      if (move_coord[move_coord[coord][move::inv[m]]][m] != coord)
        error();
    }
  }
  ok();
}

void test_coord() {
  std::cout << "Testing coord level ..." << std::endl;
  test_getset(coord::get_flip, coord::set_flip, coord::N_FLIP);
  test_getset(coord::get_twist, coord::set_twist, coord::N_TWIST);
  test_getset(coord::get_slice, coord::set_slice, coord::N_SLICE);
  test_getset(coord::get_uedges, coord::set_uedges, coord::N_UEDGES);
  test_getset(coord::get_dedges, coord::set_dedges, coord::N_DEDGES);
  test_getset(coord::get_corners, coord::set_corners, coord::N_CORNERS);

  test_getset(coord::get_slice1, coord::set_slice1, coord::N_SLICE1);
  test_getset(coord::get_udedges2, coord::set_udedges2, coord::N_UDEDGES2);

  test_movecoord(coord::move_flip, coord::N_FLIP);
  test_movecoord(coord::move_twist, coord::N_TWIST);
  test_movecoord(coord::move_edges4, coord::N_SLICE);
  test_movecoord(coord::move_corners, coord::N_CORNERS);
  test_movecoord(coord::move_udedges2, coord::N_UDEDGES2, move::p2mask);
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

  std::cout << "Phase 1: ";
  for (int m = 0; m < move::COUNT; m++) {
    if (move::in(m, move::p1mask))
      std::cout << move::names[m] << " ";
  }
  std::cout << std::endl;
  std::cout << "Phase 2: ";
  for (int m = 0; m < move::COUNT; m++) {
    if (move::in(m, move::p2mask))
      std::cout << move::names[m] << " ";
  }
  std::cout << std::endl;

  std::cout << "Forbidden:" << std::endl;
  for (int m = 0; m < move::COUNT; m++) {
    std::cout << move::names[m] << ": ";
    for (int m1 = 0; m1 < move::COUNT; m1++) {
      if (!move::in(m1, move::next[m]))
        std::cout << move::names[m1] << " ";
    }
    if (move::qt_skip[m] != 0) {
      std::cout << "| ";
      for (int m1 = 0; m1 < move::COUNT; m1++) {
        if (move::in(m1, move::qt_skip[m]))
          std::cout << move::names[m1] << " ";
      }
    }
    std::cout << std::endl;
  }

}

void test_conj(uint16_t conj_coord[][sym::COUNT_SUB], int n_coord) {
  for (int coord = 0; coord < n_coord; coord++) {
    for (int s = 0; s < sym::COUNT_SUB; s++) {
      if (conj_coord[conj_coord[coord][s]][sym::inv[s]] != coord)
        error();
      if (conj_coord[conj_coord[coord][sym::inv[s]]][s] != coord)
        error();
    }
  }
  ok();
}

void test_sym() {
  std::cout << "Testing sym level ..." << std::endl;
  test_conj(sym::conj_twist, coord::N_TWIST);
  test_conj(sym::conj_udedges2, coord::N_UDEDGES2);
}

void test_prun() {
  std::cout << "Testing pruning ..." << std::endl;

  srand(0);
  int n_moves = std::bitset<64>(move::p1mask).count(); // make sure not to consider B-moves in F5-mode

  for (int i = 0; i < 1000; i++) {
    int flip = rand() % coord::N_FLIP;
    int slice = rand() % coord::N_SLICE;
    int twist = rand() % coord::N_TWIST;

    move::mask next;
    move::mask next1;
    move::mask tmp;
    int togo = prun::get_phase1(flip, slice, twist, 100, tmp);

    int dist = prun::get_phase1(flip, slice, twist, togo, next);
    next &= move::p1mask;

    next1 = 0;
    for (int m = 0; m < n_moves; m++) {
      int flip1 = coord::move_flip[flip][m];
      int slice1 = coord::move_edges4[slice][m];
      int twist1 = coord::move_twist[twist][m];
      if (prun::get_phase1(flip1, slice1, twist1, 100, tmp) < dist)
        next1 |= move::bit(m);
    }
    if (next1 != next)
      error();

    dist = prun::get_phase1(flip, slice, twist, togo + 1, next);
    next &= move::p1mask;

    next1 = 0;
    for (int m = 0; m < n_moves; m++) {
      int flip1 = coord::move_flip[flip][m];
      int slice1 = coord::move_edges4[slice][m];
      int twist1 = coord::move_twist[twist][m];
      if (prun::get_phase1(flip1, slice1, twist1, 100, tmp) <= dist)
        next1 |= move::bit(m);
    }
    if (next1 != next)
      error();
  }

  ok();
}

bool check(const cubie::cube &c, const std::vector<int>& sol) {
  cubie::cube c1;
  cubie::cube c2;

  c1 = c;
  for (int m : sol) {
    cubie::mul(c1, move::cubes[m], c2);
    std::swap(c1, c2);
  }

  return c1 == cubie::SOLVED_CUBE;
}

int main() {
  auto tick = std::chrono::high_resolution_clock::now();
  move::init();
  coord::init();
  sym::init();
  prun::init();
  std::cout << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - tick).count() / 1000. << "ms" << std::endl;

  test_cubie();
  test_coord();
  test_move();
  test_sym();
  test_prun();

  return 0;
}
