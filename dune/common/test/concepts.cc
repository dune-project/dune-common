// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <array>
#include <bitset>
#include <complex>
#include <list>
#include <type_traits>
#include <vector>

#include <dune/common/bigunsignedint.hh>
#include <dune/common/fvector.hh>
#include <dune/common/gmpfield.hh>
#include <dune/common/hash.hh>
#include <dune/common/quadmath.hh>
#include <dune/common/reservedvector.hh>
#include <dune/common/concepts/container.hh>
#include <dune/common/concepts/hashable.hh>
#include <dune/common/concepts/number.hh>

int main ()
{
  using namespace Dune;

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

  // test Number concept for arithmetic types
  static_assert(Concept::Number<short>);
  static_assert(Concept::Number<unsigned short>);
  static_assert(Concept::Number<int>);
  static_assert(Concept::Number<unsigned int>);
  static_assert(Concept::Number<long>);
  static_assert(Concept::Number<unsigned long>);

  static_assert(Concept::Number<float>);
  static_assert(Concept::Number<double>);
  static_assert(Concept::Number<long double>);

  static_assert(Concept::Number<std::complex<float>>);
  static_assert(Concept::Number<std::complex<double>>);
  static_assert(Concept::Number<std::complex<long double>>);

  // test Number concept for user-defined types
  static_assert(Concept::Number<Dune::bigunsignedint<64>>);
#if HAVE_GMP
  static_assert(Concept::Number<Dune::GMPField<64>>);
#endif
#if HAVE_QUADMATH
  static_assert(Concept::Number<Dune::Float128>);
#endif

}
