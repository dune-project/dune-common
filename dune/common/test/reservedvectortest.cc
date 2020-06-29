// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <unordered_map>

#include <dune/common/test/testsuite.hh>
#include <dune/common/classname.hh>
#include <dune/common/reservedvector.hh>

int main() {
  Dune::TestSuite test;
  // check that make_array works
  Dune::ReservedVector<unsigned int, 8> rv = {3,2,1};
  test.check(rv.size() == 3);
  test.check(rv.back() == 1);
  test.check(rv.front() == 3);

  // check assignment from an initializer list
  rv = {1,2,3,4};
  test.check(rv.size() == 4);
  test.check(rv.back() == 4);
  test.check(rv.front() == 1);

  // check push_back
  rv.push_back(5);
  test.check(rv.size() == 5);
  test.check(rv.back() == 5);

  // check copy constructor
  Dune::ReservedVector<unsigned int, 8> rv2 = rv;
  test.check(rv2[0] == 1 &&
    rv2[1] == 2 &&
    rv2[2] == 3 &&
    rv2[3] == 4 &&
    rv2[4] == 5);

  // check pop_back
  rv2.pop_back();
  test.check(rv2.size() == 4);
  test.check(rv2.back() == 4);

  // make sure we can hash a reserved vector
  std::hash< Dune::ReservedVector<unsigned int, 8> > rv_hash;
  auto hash_value = rv_hash(rv);
  auto hash_value2 = rv_hash(rv2);
  test.check( hash_value != hash_value2 );

  // try using an unordered map
  std::unordered_map< Dune::ReservedVector<unsigned int, 8>, double > rv_map;
  rv_map[rv] = 1.0;
  rv_map[rv2] = 2.0;

  // try and try again with a const ReservedVector
  std::unordered_map< const Dune::ReservedVector<unsigned int, 8>, double> const_rv_map;

  return 0;
}
