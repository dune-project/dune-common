// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tuple>
#include <utility>
#include <sstream>

#include <dune/common/std/apply.hh>

#include <dune/common/test/testsuite.hh>


template<class T>
void nop(std::initializer_list<T>&&)
{}


int main()
{
  auto args = std::make_tuple(true, 2, 3, "abc");

  Dune::TestSuite test;

  auto concat = [](auto&&... args) {
            bool first = true;
            std::stringstream stream;
            nop({(stream << (first ? "":",") << args, first = false)...});
            return stream.str();
          };

  test.check(Dune::Std::apply(concat, args) == "1,2,3,abc") << "Dune::Std::apply failed with concat lambda";

  auto makeTuple = [](auto&&... args) {
            return std::make_tuple(args...);
          };

  test.check(Dune::Std::apply(makeTuple, args) == args) << "Dune::Std::apply failed with makeTuple lambda";

  auto intTuple = std::make_tuple(1,2,3);
  auto&& intTuple0 = Dune::Std::apply([](auto&& arg0, auto&&... args) -> decltype(auto) { return arg0; }, intTuple);
  intTuple0 = 42;

  test.check(std::get<0>(intTuple) == intTuple0) << "Dune::Std::apply does not properly return references";


  return test.exit();
}
