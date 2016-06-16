// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tuple>
#include <vector>

#include <dune/common/hybridutilities.hh>
#include <dune/common/test/testsuite.hh>


/** \brief A std::tuple that allows access to its element via operator[]
 *
 * Since there is no heterogeneous container in dune-common that supports
 * operator[std::integralconstant<T,i>] we make our own for the test.
 */
template<class... T>
class TupleVector : public std::tuple<T...>
{
  using Base = std::tuple<T...>;
public:

  using Base::Base;

  /** \brief Array-style access to the tuple elements */
  template<std::size_t i>
  constexpr auto operator[](const Dune::index_constant<i>&)
    ->decltype(std::get<i>(*this))
  {
    return std::get<i>(*this);
  }

  static constexpr auto size()
  {
    return std::tuple_size<Base>::value;
  }
};

template<class... T>
constexpr auto makeTupleVector(T&&... t)
{
  // The std::decay_t<T> is is a slight simplification,
  // because std::reference_wrapper needs special care.
  return TupleVector<std::decay_t<T>...>(std::forward<T>(t)...);
}



template<class C>
auto incrementAll(C&& c)
{
  using namespace Dune::Hybrid;
  forEach(c, [](auto&& ci) {
    ++ci;
  });
}

template<class C>
auto addIndex(C&& c)
{
  using namespace Dune::Hybrid;
  forEach(integralRange(size(c)), [&](auto&& i) {
    c[i] += i;
  });
}

template<class C>
auto incAndAppendToFirst(C&& c)
{
  using namespace Dune::Hybrid;
  using namespace Dune::Indices;
  forEach(integralRange(size(c)), [&](auto&& i) {
    ifElse(equals(i, _0), [&](auto id) {
      id(c[i]).append("+1");
    }, [&](auto id) {
      ++id(c[i]);
    });
  });
}



int main()
{
  auto vector = std::vector<int>{1, 2, 3};
  auto numberTuple = makeTupleVector(0.1, 2, 3);

  Dune::TestSuite test;

  incrementAll(vector);
  test.check(vector == std::vector<int>{2, 3, 4})
    << "Incrementing vector entries with Hybrid::forEach failed.";

  incrementAll(numberTuple);
  test.check(numberTuple == makeTupleVector(1.1, 3, 4))
    << "Incrementing tuple entries with Hybrid::forEach failed.";

  addIndex(vector);
  test.check(vector == std::vector<int>{2, 4, 6})
    << "Adding indices to vector entries with Hybrid::forEach failed.";

  addIndex(numberTuple);
  test.check(numberTuple == makeTupleVector(1.1, 4, 6))
    << "Adding indices to vector entries with Hybrid::forEach failed.";


  auto mixedTuple = makeTupleVector(std::string("1"), 2, 3);
  incAndAppendToFirst(mixedTuple);
  test.check(mixedTuple == makeTupleVector(std::string("1+1"), 3, 4))
    << "Adding indices to vector entries with Hybrid::forEach failed.";

  return test.exit();
}
