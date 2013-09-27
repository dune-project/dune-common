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

  // Test is_pointer
  assert( not Dune::is_pointer<int>::value );
  assert(     Dune::is_pointer<int*>::value );
  assert(     Dune::is_pointer<int**>::value );
  assert(     Dune::is_pointer<int(*)(int)>::value );

  // Test is_reference
  assert( not Dune::is_lvalue_reference<int>::value );
  assert(     Dune::is_lvalue_reference<int&>::value );
#if HAVE_RVALUE_REFERENCES
  assert( not Dune::is_lvalue_reference<int&&>::value );
#endif

  // Test remove_pointer
  // Note: when the argument T is not a pointer, TypeTraits::PointeeType returns Dune::Empty,
  // while Dune::remove_pointer (as std::remove_pointer), returns T itself
  assert( (Dune::is_same<int,       Dune::remove_pointer<int>::type>::value) );
  assert( (Dune::is_same<int,       Dune::remove_pointer<int*>::type>::value) );
  assert( (Dune::is_same<int*,      Dune::remove_pointer<int**>::type>::value) );
  assert( (Dune::is_same<const int, Dune::remove_pointer<const int*>::type>::value) );
  assert( (Dune::is_same<int,       Dune::remove_pointer<int* const>::type>::value) );

  // Test remove_reference
  assert( (Dune::is_same<int, Dune::remove_reference<int>::type>::value) );
  assert( (Dune::is_same<int, Dune::remove_reference<int&>::type>::value) );
#if HAVE_RVALUE_REFERENCES
  assert( (Dune::is_same<int, Dune::remove_reference<int&&>::type>::value) );
#endif

  return 0;
}
