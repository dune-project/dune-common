// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tuple>

#include <dune/common/indices.hh>

using namespace Dune;

/** \brief A std::tuple that allows access to its element via operator[]
 *
 * Helper class to test the static indices with
 */
template<class... T>
struct TupleVector : public std::tuple<T...>
{
  /** \brief Array-style access to the tuple elements */
  template<std::size_t i>
  auto operator[](const index_constant<i>&)
    ->decltype(std::get<i>(*this))
  {
    return std::get<i>(*this);
  }
};



int main()
{
  using namespace Dune::Indices;

  // Test whether indices can be used to index a data structure
  TupleVector<int,double,float> v;
  v[_0] = 42;
  v[_1] = 3.14;
  v[_2] = 2.7;

  // Test whether the indices can be used as numbers
  std::get<_0>(v) = 43;
  std::get<_1>(v) = 4.14;
  std::get<_2>(v) = 3.7;

  return 0;
}
