// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstddef>

#include <dune/common/tuples.hh>
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
static_assert((Dune::is_same<MyTupleTupleFlat1, MyTupleTupleFlat2>::value),
              "FlattenTuples failed!");



//////////////////////////////////////////////////////////////////////
//
// check nested ReduceTuple with a litte TMP

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
  typedef Dune::tuple<> type;
};

// An accumulator to build up a list of divisors of an integer using reduce
template<class Data, class PotentialDivisor>
struct DivisorAccumulator
{
  enum {value = Data::first_type::value};
  enum {isDivisor = (PotentialDivisor::value*(value / PotentialDivisor::value)==value)};

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

  enum {value = std::tuple_size<type>::value};
};

// An accumulator to build up a list of primes up to a fixed integer
template<class Data, class N>
struct PrimeAccumulator
{
  enum {isPrime = (Divisors<N::value>::value==2)};

  typedef typename std::conditional<isPrime, typename Dune::PushBackTuple<Data, N>::type, Data>::type type;
};

// Construct list primes
template<int X>
struct Primes
{
  typedef typename Dune::ReduceTuple<
      PrimeAccumulator,
      typename Range<1,X+1>::type,
      typename Dune::tuple<>
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


int main() {}
