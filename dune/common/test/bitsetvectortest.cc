// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/bitsetvector.hh>

#if defined(__GNUC__) && ! defined(__clang__)
#include <ext/malloc_allocator.h>
#endif

#include <dune/common/test/iteratortest.hh>

template<class BBF>
struct ConstReferenceOp
{
  typedef typename BBF::value_type bitset;
  typedef typename BBF::const_reference const_reference;

  void operator()(const_reference t){
    [[maybe_unused]] bitset x = t;
  }
};

template <class T>
void testConstBitSetMethods(const T t)
{
  t.size();
  t[0];
  t[t.size()-1];
  t << 2;
  t >> 2;
  ~t;
  t.count();
  t.any();
  t.none();
  t.test(0);
}

template<class BBF>
void testContainer(BBF & bbf)
{
  typedef typename BBF::value_type bitset;
  typedef typename BBF::reference reference;
  typedef typename BBF::const_reference const_reference;

  const BBF & cbbf = bbf;

  bitset x = bbf[3];
  reference y = bbf[4];
  const_reference z = bbf[4];
  const reference v = bbf[4];

  // assignment
  y = false;
  y[2] = true;
  y = x;
  y = z;
  y = v;
  x = y;
  x = z;
  x = v;
  y = cbbf[1];
  x = cbbf[1];
  bbf[4] = x;
  bbf[4] = v;
  bbf[4] = y;
  bbf[4] = true;

  // invoke methods
  testConstBitSetMethods(x);
  testConstBitSetMethods(y);
  testConstBitSetMethods(z);
  testConstBitSetMethods(v);
  testConstBitSetMethods(bbf[1]);
  testConstBitSetMethods(cbbf[2]);

  // equality
  [[maybe_unused]] bool res;
  res = (y == cbbf[2]);
  res = (y == bbf[3]);
  res = (y == x);
  res = (x == y);
  res = (x == z);
  res = (z == x);
  res = (z == y);
  res = (y == z);

  // inequality
  res = (y != cbbf[2]);
  res = (y != bbf[3]);
  res = (y != x);
  res = (x != y);
  res = (x != z);
  res = (z != x);
  res = (z != y);
  res = (y != z);

  // &=
  y &= cbbf[2];
  y &= bbf[3];
  y &= x;
  x &= y;
  x &= z;
  y &= z;

  // |=
  y |= cbbf[2];
  y |= bbf[3];
  y |= x;
  x |= y;
  x |= z;
  y |= z;

  // ^=
  y ^= cbbf[2];
  y ^= bbf[3];
  y ^= x;
  x ^= y;
  x ^= z;
  y ^= z;

  // shift operator
  y <<= 1;
  y >>= 1;

  // flip
  y.flip();
  y.flip(2);
  y[3].flip();
}

template<class BBF>
void testConstContainer(const BBF& bbf){
  typedef typename BBF::value_type bitset;
  typedef typename BBF::iterator iterator;
  typedef typename std::iterator_traits<iterator>::value_type value_type;
  typedef typename BBF::const_reference reference;

  const BBF & cbbf = bbf;

  bitset x = bbf[3];
  [[maybe_unused]] value_type z;
  reference y = bbf[4];

  // assignment
  x = y;
  x = cbbf[1];

  // equality
  [[maybe_unused]] bool res;
  res = (y == cbbf[2]);
  res = (y == bbf[3]);
  res = (y == x);
  res = (x == y);

  // inequality
  res = (y != cbbf[2]);
  res = (y != bbf[3]);
  res = (y != x);
  res = (x != y);
}

template<int block_size, class Alloc>
void doTest() {
  typedef Dune::BitSetVector<block_size, Alloc> BBF;

  BBF bbf(10,true);
  const BBF & cbbf = bbf;

  // test containers and some basic bitset operations
  testContainer(bbf);
  testConstContainer(bbf);
  testConstContainer(cbbf);

  // iterator interface
#ifndef NDEBUG
  ConstReferenceOp<BBF> cop;
  assert(testIterator(bbf, cop) == 0);
  assert(testIterator(cbbf, cop) == 0);
#endif
}

int main()
{
  doTest<4, std::allocator<bool> >();
#if defined(__GNUC__) && ! defined(__clang__)
  doTest<4, __gnu_cxx::malloc_allocator<bool> >();
#endif
  return 0;
}
