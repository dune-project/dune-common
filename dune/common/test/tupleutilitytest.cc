// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <cmath>
#include <cstddef>
#include <iostream>
#include <tuple>

#include <dune/common/tupleutility.hh>

//////////////////////////////////////////////////////////////////////
//
// check FirstTypeIndex
//
typedef std::tuple<int, unsigned, double> MyTuple;
static_assert((Dune::FirstTypeIndex<MyTuple, int>::value == 0),
              "FirstTypeIndex finds the wrong index for double in MyTuple!");
static_assert((Dune::FirstTypeIndex<MyTuple, unsigned>::value == 1),
              "FirstTypeIndex finds the wrong index for double in MyTuple!");
static_assert((Dune::FirstTypeIndex<MyTuple, double>::value == 2),
              "FirstTypeIndex finds the wrong index for double in MyTuple!");



//////////////////////////////////////////////////////////////////////
//
// check PushBackTuple
typedef Dune::PushBackTuple<MyTuple, char>::type MyTupleAppended1;
typedef std::tuple<int, unsigned, double, char> MyTupleAppended2;
static_assert((std::is_same<MyTupleAppended1, MyTupleAppended2>::value),
              "PushBackTuple failed!");



//////////////////////////////////////////////////////////////////////
//
// check PushFrontTuple
typedef Dune::PushFrontTuple<MyTuple, char>::type MyTuplePrepended1;
typedef std::tuple<char, int, unsigned, double> MyTuplePrepended2;
static_assert((std::is_same<MyTuplePrepended1, MyTuplePrepended2>::value),
              "PushFrontTuple failed!");



//////////////////////////////////////////////////////////////////////
//
// check JoinTuples
typedef Dune::JoinTuples<MyTuple, MyTuple>::type MyTupleMyTuple1;
typedef std::tuple<int, unsigned, double, int, unsigned, double> MyTupleMyTuple2;
static_assert((std::is_same<MyTupleMyTuple1, MyTupleMyTuple2>::value),
              "JoinTuples failed!");



//////////////////////////////////////////////////////////////////////
//
// check FlattenTuple
typedef std::tuple<char, float> MyTuple2;
typedef std::tuple<MyTuple, MyTuple2> MyTupleTuple;
typedef Dune::FlattenTuple<MyTupleTuple>::type MyTupleTupleFlat1;
typedef std::tuple<int, unsigned, double, char, float> MyTupleTupleFlat2;
static_assert((std::is_same<MyTupleTupleFlat1, MyTupleTupleFlat2>::value),
              "FlattenTuples failed!");



//////////////////////////////////////////////////////////////////////
//
// check nested ReduceTuple with a little TMP

// A tuple of a range of integers wrapped in integral_constant types
template<int start, int end>
struct Range
{
  typedef typename Dune::PushBackTuple<
      typename Range<start, end-1>::type,
      typename std::integral_constant<int,end-1>
      >::type type;
};

template<int start>
struct Range<start, start>
{
  typedef std::tuple<> type;
};

// An accumulator to build up a list of divisors of an integer using reduce
template<class Data, class PotentialDivisor>
struct DivisorAccumulator
{
  constexpr static int value = Data::first_type::value;
  constexpr static bool isDivisor = (PotentialDivisor::value*(value / PotentialDivisor::value)==value);

  typedef typename Data::second_type OldTuple;
  typedef typename Dune::PushBackTuple<OldTuple, PotentialDivisor>::type ExtendedTuple;
  typedef typename std::conditional<isDivisor, ExtendedTuple, OldTuple>::type NewTuple;

  typedef typename std::pair<typename Data::first_type, NewTuple> type;
};

// Construct list of divisors using reduce
template<int X>
struct Divisors
{
  typedef typename Dune::ReduceTuple<
      DivisorAccumulator,
      typename Range<1,X+1>::type,
      typename std::pair<typename std::integral_constant<int, X>, typename std::tuple<> >
      >::type::second_type type;

  constexpr static int value = std::tuple_size<type>::value;
};

// An accumulator to build up a list of primes up to a fixed integer
template<class Data, class N>
struct PrimeAccumulator
{
  constexpr static bool isPrime = (Divisors<N::value>::value==2);

  typedef typename std::conditional<isPrime, typename Dune::PushBackTuple<Data, N>::type, Data>::type type;
};

// Construct list primes
template<int X>
struct Primes
{
  typedef typename Dune::ReduceTuple<
      PrimeAccumulator,
      typename Range<1,X+1>::type,
      typename std::tuple<>
      >::type type;
};

typedef Primes<9>::type Primes1;
typedef std::tuple<
    std::integral_constant<int, 2>,
    std::integral_constant<int, 3>,
    std::integral_constant<int, 5>,
    std::integral_constant<int, 7> > Primes2;
static_assert((std::is_same<Primes1, Primes2>::value),
              "ReduceTuple failed in primes-tmp!");

struct Reciprocal
{
  template<class>
  struct TypeEvaluator
  {
    typedef double Type;
  };
  template<class T>
  typename TypeEvaluator<T>::Type operator()(const T& val) const {
    return 1./val;
  }
};

int main()
{
  const std::tuple<int, double> t1(1, 2.);
  auto t2 = Dune::genericTransformTuple(t1, Reciprocal());
  static_assert(std::is_same<decltype(t2), std::tuple<double, double>>::value,
                "Type after genericTransformTuple does not match!");
  if(fabs(std::get<0>(t2)-1.) > 1e-8 ||
     fabs(std::get<1>(t2)-.5) > 1e-8)
  {
    std::cout << "genericTransformTuple gives wrong result!\n";
    std::abort();
  }

  auto t3 = Dune::applyPartial([&] (auto&&... x) {
      return std::make_tuple((1./x)...);
    }, t1, std::make_index_sequence<2>());

  if(t2 != t3)
  {
    std::cout << "genericTransformTuple gives wrong result!\n";
    std::abort();
  }

}
