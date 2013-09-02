// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/** \file
 * \brief Test the TypeTraits class
 */

#include <iostream>
#include <cassert>

#include <dune/common/typetraits.hh>

int main() {

  // Test TypeTraits::isPointer
  assert( not Dune::TypeTraits<int>::isPointer );
  assert(     Dune::TypeTraits<int*>::isPointer );
  assert(     Dune::TypeTraits<int**>::isPointer );
  assert(     Dune::TypeTraits<int(*)(int)>::isPointer );

  // Test TypeTraits::isReference
  assert( not Dune::TypeTraits<int>::isReference );
  assert(     Dune::TypeTraits<int&>::isReference );
#if HAVE_RVALUE_REFERENCES
  assert( not Dune::TypeTraits<int&&>::isReference );
#endif

  // Test TypeTraits::PointeeType
  assert( (Dune::is_same<Dune::Empty, Dune::TypeTraits<int>::PointeeType>::value) );
  assert( (Dune::is_same<int,         Dune::TypeTraits<int*>::PointeeType>::value) );
  assert( (Dune::is_same<int*,        Dune::TypeTraits<int**>::PointeeType>::value) );
  assert( (Dune::is_same<const int,   Dune::TypeTraits<const int*>::PointeeType>::value) );
  assert( (Dune::is_same<Dune::Empty, Dune::TypeTraits<int* const>::PointeeType>::value) );

  // Test TypeTraits::ReferredType
  assert( (Dune::is_same<int, Dune::TypeTraits<int>::ReferredType>::value) );
  assert( (Dune::is_same<int, Dune::TypeTraits<int&>::ReferredType>::value) );

  return 0;
}
