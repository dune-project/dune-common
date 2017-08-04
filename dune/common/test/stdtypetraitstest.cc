// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <dune/common/std/type_traits.hh>

#include <dune/common/test/testsuite.hh>


int main()
{
  Dune::TestSuite test;

  {
    auto f = [](int i) { return 0; };
    using F = decltype(f);

    test.check(Dune::Std::is_callable<F(int)>() == true)
      << "Dune::Std::is_callable does not accept copy from r-value";
    test.check(Dune::Std::is_callable<F(int&)>() == true)
      << "Dune::Std::is_callable does not accept copy from l-value reference";
    test.check(Dune::Std::is_callable<F(int&&)>() == true)
      << "Dune::Std::is_callable does not accept copy from r-value reference";

    test.check(Dune::Std::is_callable<F(std::string)>() == false)
      << "Dune::Std::is_callable accepts invalid argument type";
    test.check(Dune::Std::is_callable<F(int, int)>() == false)
      << "Dune::Std::is_callable accepts invalid argument count";
    test.check(Dune::Std::is_callable<F(int), int>() == true)
      << "Dune::Std::is_callable does not accept valid return type";
    test.check(Dune::Std::is_callable<F(int), std::string>() == false)
      << "Dune::Std::is_callable accepts invalid return type";
  }

  {
    auto f = [](const int& i) {};
    using F = decltype(f);

    test.check(Dune::Std::is_callable<F(int)>() == true)
      << "Dune::Std::is_callable does not accept const& temporary from r-value";
    test.check(Dune::Std::is_callable<F(int&)>() == true)
      << "Dune::Std::is_callable does not accept const& temporary from l-value reference";
    test.check(Dune::Std::is_callable<F(int&&)>() == true)
      << "Dune::Std::is_callable does not accept const& temporary from r-value reference";
  }

  {
    auto f = [](int& i) {};
    using F = decltype(f);

    test.check(Dune::Std::is_callable<F(int)>() == false)
      << "Dune::Std::is_callable accepts l-value reference from r-value";
    test.check(Dune::Std::is_callable<F(int&)>() == true)
      << "Dune::Std::is_callable does not accept l-value reference from l-value reference";
    test.check(Dune::Std::is_callable<F(int&&)>() == false)
      << "Dune::Std::is_callable accepts l-value reference from r-value reference";
  }

  {
    auto f = [](int&& i) {};
    using F = decltype(f);

    test.check(Dune::Std::is_callable<F(int)>() == true)
      << "Dune::Std::is_callable does not accept r-value reference from r-value";
    test.check(Dune::Std::is_callable<F(int&)>() == false)
      << "Dune::Std::is_callable accepts r-value reference from l-value reference";
    test.check(Dune::Std::is_callable<F(int&&)>() == true)
      << "Dune::Std::is_callable does not accept r-value reference from r-value reference";
  }

  return test.exit();
}
