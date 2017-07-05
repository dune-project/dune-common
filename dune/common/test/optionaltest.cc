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

  optFalse = create( true );
  test.check( *optFalse == "void" );

  return test.exit();
}
