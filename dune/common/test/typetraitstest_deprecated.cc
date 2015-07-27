// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/** \file
 * \brief Test the deprecated functionality of the TypeTraits class.
 */

#include <cassert>

// Suppress deprecation warnings
// do this before including any other dune-headers
#include <dune/common/deprecated.hh>
#undef DUNE_DEPRECATED
#define DUNE_DEPRECATED
#undef DUNE_DEPRECATED_MSG
#define DUNE_DEPRECATED_MSG(text)

#include <dune/common/typetraits.hh>

int main() {

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
