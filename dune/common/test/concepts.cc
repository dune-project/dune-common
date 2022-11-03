// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

// check whether c++20 concept can be used
#if __has_include(<version>) && __has_include(<concepts>)
  #include <version>
  #if  __cpp_concepts >= 201907L && __cpp_lib_concepts >= 202002L
    #ifndef DUNE_COMMON_HAVE_CONCEPTS
    #define DUNE_COMMON_HAVE_CONCEPTS 1
    #endif
  #endif
#endif

#if DUNE_COMMON_HAVE_CONCEPTS

#include <bitset>
#include <type_traits>
#include <vector>

#include <dune/common/bigunsignedint.hh>
#include <dune/common/concepts.hh>
#include <dune/common/hash.hh>
#include <dune/common/reservedvector.hh>
#include <dune/common/parallel/mpihelper.hh>

int main (int argc, char **argv)
{
  using namespace Dune;
  MPIHelper::instance(argc, argv);

  // test Hashable
  static_assert(Concept::Hashable<int>);
  static_assert(Concept::Hashable<double>);
  static_assert(Concept::Hashable<Dune::bigunsignedint<64>>);
  static_assert(Concept::Hashable<Dune::ReservedVector<double,3>>);

  static_assert(not Concept::Hashable<std::vector<double>>);
}

#else // DUNE_COMMON_HAVE_CONCEPTS

int main () { return 77; }

#endif // DUNE_COMMON_HAVE_CONCEPTS
