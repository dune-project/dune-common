#include "config.h"

#include <iostream>
#include <type_traits>
#include <cassert>

#include <dune/common/typetree/treepath.hh>

int main(int argc, char** argv)
{

  using namespace Dune::Indices;

  {

    constexpr auto path = Dune::TypeTree::hybridTreePath(_1,3,_2,5);

    static_assert(std::is_same<std::decay_t<decltype(path.element(_0))>,std::decay_t<decltype(_1)>>{},"wrong entry value");
    static_assert(path.element(_0) == 1,"wrong entry value");

    static_assert(std::is_same<std::decay_t<decltype(path[_2])>,std::decay_t<decltype(_2)>>{},"wrong entry value");
    static_assert(path[_0] == 1,"wrong entry value");

    assert(path.element(_0) == 1);
    assert(path.element(3) == 5);

    assert(path[_0] == 1);
    assert(path[3] == 5);

  }

  return 0;
}
