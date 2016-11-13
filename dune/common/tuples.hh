#warning The header dune/common/tuples.hh is deprecated. Use directly "std::tuple" and include dune/common/streamoperators.hh for stream operators.

#ifndef DUNE_TUPLES_HH
#define DUNE_TUPLES_HH

#include <tuple>
#include <dune/common/tupleutility.hh>
#include <dune/common/streamoperators.hh>

namespace Dune {
  using std::tuple;
  using std::tuple_element;
  using std::get;
  using std::tuple_size;
  using std::tie;
  using std::make_tuple;
}
#endif
