// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

#include <dune/common/std/optional.hh>
#include <dune/common/test/testsuite.hh>
#include <dune/common/unused.hh>

class T {
public:

  T() : valid_(true) {}

  T(const T& other) : valid_(true) {}

  T(T&& other) : valid_(true) { other.valid_ = false; }

  T& operator=(const T& other) { valid_ = true; }

  T& operator=(T&& other) { valid_ = true; other.valid_ = false; }

  bool valid() const { return valid_; }

protected:
  bool valid_;
};

Dune::Std::optional< std::string > create ( bool b )
{
  if( b )
    return "void";
  else
    return {};
}

int main()
{
  Dune::TestSuite test;
  auto optFalse = create( false );
  auto optTrue = create( true );

  // construction/value check
  test.check( !optFalse );
  test.check( bool(optTrue) );
  test.check( optTrue.value() == "void" );
  test.check( *optTrue == "void" );

  // value_or check (Not implemented atm)
  test.check( optFalse.value_or( "empty" ) == "empty" );
  test.check( optTrue.value_or( "notEmpty" ) == "void" );

  // emplace check
  optFalse.emplace( "foo" );
  test.check( bool(optFalse) );
  test.check( optFalse.value() == "foo" );

  // reset test (Not implemented atm)
  optTrue.reset();
  test.check( !optTrue );

  // swap check
  optFalse.swap( optTrue );
  test.check( !optFalse );
  test.check( *optTrue == "foo" );

  // Check construction of optional for const types
  Dune::Std::optional<const int> o1(42);
  Dune::Std::optional<const int> DUNE_UNUSED o2(o1);
  Dune::Std::optional<const int> DUNE_UNUSED o3(std::move(o1));

  optFalse = create( true );
  test.check( *optFalse == "void" );

  {
    T t1;
    Dune::Std::optional<T> o(t1);
    test.check(t1.valid());

    T t2;
    o = t2;
    test.check(t2.valid());
  }

  return test.exit();
}
