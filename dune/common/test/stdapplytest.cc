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
  auto args = std::make_tuple(1, 2, 3, "abc");

  Dune::TestSuite test;

  std::stringstream stream;

  Dune::Std::apply([&](auto&&... args) {
            bool first = true;
            nop({(stream << (first ? "":",") << args, first = false)...});
        }, args);

  test.check(stream.str() == "1,2,3,abc") << "Dune::Std::apply failed";

  return test.exit();
}
