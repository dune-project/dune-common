// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <unordered_map>

#include <dune/common/test/testsuite.hh>
#include <dune/common/classname.hh>
#include <dune/common/reservedvector.hh>

struct A
{
  explicit A(int s = 42)
    : data_(s > 0 ? new double[s] : nullptr)
    , size_(s) {};
  A(const A& other)
    : A(other.size_) {}
  A(A&& other) {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
  }
  A& operator=(A other) {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    return *this;
  }
  ~A() { delete[] data_; }

  double* data_ = nullptr;
  int size_ = 0;
};

struct NoCopy
{
  NoCopy() = default;
  NoCopy(const NoCopy&) = delete;
  NoCopy(NoCopy&&) = default;
  NoCopy& operator= (const NoCopy&) = delete;
  NoCopy& operator= (NoCopy&&) = default;
};

struct NoMove
{
  NoMove() = default;
  NoMove(const NoMove&) = default;
  NoMove(NoMove&&) = delete;
  NoMove& operator= (const NoMove&) = default;
  NoMove& operator= (NoMove&&) = delete;
};

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

  // check emplace_back
  test.check(rv.emplace_back(6) == 6);
  test.check(rv.size() == 6);
  test.check(rv.back() == 6);
  rv.pop_back();

  // check copy constructor
  Dune::ReservedVector<unsigned int, 8> rv2 = rv;
  test.check(rv2[0] == 1 &&
    rv2[1] == 2 &&
    rv2[2] == 3 &&
    rv2[3] == 4 &&
    rv2[4] == 5);

  // check size constructor
  Dune::ReservedVector<unsigned int, 8> rv3(7);
  test.check(rv3.size() == 7);
  test.check(rv3[6] == 0);

  // check size and value constructor
  Dune::ReservedVector<unsigned int, 8> rv4(5, 42);
  test.check(rv4.size() == 5);
  test.check(rv4[3] == 42);

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

  rv = {1,2,3,4};
  { // check forward iterators
    unsigned int i = 1;
    for (auto it = rv.begin(); it != rv.end(); ++it)
      test.check( *it == i++ );

    // check backward iterators
    i = 4;
    for (auto it = rv.rbegin(); it != rv.rend(); ++it)
      test.check( *it == i-- );

    // check raw data
    i = 1;
    for (auto* it = rv.data(); it != rv.data()+rv.size(); ++it)
      test.check( *it == i++ );
  }

  { // check non-trivial types
    Dune::ReservedVector<A, 8> rvA;
    rvA.push_back(A(5));
    rvA.emplace_back(A(5));
    rvA.emplace_back(5);
    test.check( rvA.size() == 3 );
  }

  { // check non-copyable types
    Dune::ReservedVector<NoCopy, 8> rv;
    rv.push_back(NoCopy{});
    rv.emplace_back();
    test.check( rv.size() == 2 );
  }

  { // check non-movable types
    Dune::ReservedVector<NoMove, 8> rv;
    NoMove x;
    rv.push_back(x);
    rv.emplace_back();
    test.check( rv.size() == 2 );
  }

  { // check constexpr
    constexpr Dune::ReservedVector<unsigned int, 8> crv{3,2,1};
    static_assert(crv.size() == 3);
    static_assert(crv.at(2) == 1);
  }
  return 0;
}
