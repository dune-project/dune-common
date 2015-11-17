// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/** \file
 * \brief Test the type traits classes
 */

#include <iostream>
#include <cassert>

#include <dune/common/typetraits.hh>

int main() {

  // Test is_pointer
  assert( not Dune::is_pointer<int>::value );
  assert(     Dune::is_pointer<int*>::value );
  assert(     Dune::is_pointer<int**>::value );
  assert(     Dune::is_pointer<int(*)(int)>::value );

  // Test is_reference
  assert( not Dune::is_lvalue_reference<int>::value );
  assert(     Dune::is_lvalue_reference<int&>::value );
  assert( not Dune::is_lvalue_reference<int&&>::value );

  return 0;
}
