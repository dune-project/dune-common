// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

#include <dune/common/concepts.hh>

#if DUNE_ENABLE_CONCEPTS

#include <array>
#include <bitset>
#include <list>
#include <type_traits>
#include <vector>

#include <dune/common/bigunsignedint.hh>
#include <dune/common/fvector.hh>
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

  // test Container
  static_assert(Concept::Container<std::list<double>>);
  static_assert(Concept::RandomAccessContainer<std::array<double,3>>);
  static_assert(Concept::RandomAccessContainer<std::vector<double>>);
  static_assert(Concept::RandomAccessContainer<Dune::ReservedVector<double,3>>);

  static_assert(not Concept::Container<double*>);
  static_assert(not Concept::Container<Dune::FieldVector<double,3>>);
}

#else // DUNE_ENABLE_CONCEPTS

int main () { return 77; }

#endif // DUNE_ENABLE_CONCEPTS
