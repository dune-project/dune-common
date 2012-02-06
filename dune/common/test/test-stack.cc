// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifdef NDEBUG
#warning "Disabling NDEBUG for this test, otherwise it will fail!"
#undef NDEBUG
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cassert>
#include <iostream>

#include <dune/common/finitestack.hh>

// stack depth to test
static const int MAX = 100;

template <class SType>
void exercise_stack (SType &S) {
  assert(S.empty());

  // fill stack to maximum
  for (int i = 0; i < MAX; ++i) {
    assert(! S.full());
    S.push(i);
    assert(! S.empty());
  };

  for (int i = MAX - 1; i >= 0; --i) {
    int x = S.top();
    int y = S.pop();
    assert(x == i);
    assert(y == i);
  };

  assert(S.empty());
}

int main () {
  // initialize stack, push stuff and check if it comes out again
  Dune::FiniteStack<int, MAX> fixedstack;
  exercise_stack(fixedstack);

  // check error handling of Stack
  try {
    Dune::FiniteStack<int, MAX> stack1;

    assert(stack1.empty());
    stack1.pop();

    // exception has to happen
    // make sure you compile this test without NDEBUG
    std::cerr << "Expected exception Dune::RangeError, but nothing caught\n";
    return 1;
  } catch (Dune::RangeError &e) {
    // exception was correctly reported
    std::cerr << "Caught expected Dune::RangeError: " << e.what() << std::endl;
    return 0;
  } catch (Dune::Exception &e) {
    // exception was correctly reported
    std::cerr << "Dune::Exception: " << e.what() << std::endl;
    return 1;
  } catch (std::exception &e) {
    // exception was correctly reported
    std::cerr << "std::exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    // wrong type of exception
    std::cerr << "unknown exception\n";
    return 1;
  }

}
