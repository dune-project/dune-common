// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <algorithm>

#include <dune/common/densevector.hh>
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/exceptions.hh>

//! @file
/**
 * This test tests for a regression, where `std::is_assignable` would return
 * `true` for certain assignments, but it was not actually possible to
 * instantiate those assignments.  In the fix `std::is_assignable` was fixed
 * to report false, and that is what is checked now.
 */

template<class Component, std::size_t Dim >
class MyVector;

namespace Dune
{
  template<class Component, std::size_t Dim>
  struct DenseMatVecTraits< MyVector<Component, Dim> >
  {
    using derived_type = MyVector<Component, Dim>;
    using value_type = Component;
    using size_type = std::size_t;
  };

  template<class Component, std::size_t Dim, int Size>
  struct IsFieldVectorSizeCorrect<MyVector<Component, Dim>, Size>
    : std::integral_constant<bool, Dim == Size>
  {};
}

template<class Component, std::size_t Dim >
class MyVector : public Dune::DenseVector< MyVector<Component, Dim> >
{
 public:
  static constexpr std::size_t size () { return Dim; }

  Component& operator[] ( std::size_t i ) { return data_; }
  const Component& operator[] ( std::size_t i ) const { return data_; }
 protected:
  Component data_;
};

int main()
{
  try
  {
    // Pure 1d case. Here OuterMV is assignable to MiddleFV as the the
    // 1d FieldVector implements a type-case to the underlying
    // field. This is expected behaviour.
    {
      using InnerFV = Dune::FieldVector<double, 1>;
      using MiddleFV = Dune::FieldVector<InnerFV, 1>;
      using OuterFV = Dune::FieldVector<MiddleFV, 1>;

      using MiddleMV = MyVector<InnerFV, 1>;
      using OuterMV = MyVector<MiddleMV, 1>;

      MiddleFV mfv;
      OuterMV mv;
      OuterFV fv;

      static_assert(std::is_convertible<OuterMV, OuterFV>::value,
                    "DenseVectors should be convertible.");
      fv = mv;

      static_assert(std::is_assignable<MiddleFV&, OuterMV>::value,
                    "Reduced assignability detected.");
      mfv = mv;
    }

    // The following would trigger a problem in the DenseVector
    // operator=() which was cured by first checking whether the
    // value_types are assignable.
    {
      using InnerFV = Dune::FieldVector<double, 2>;
      using MiddleFV = Dune::FieldVector<InnerFV, 1>;
      using OuterFV = Dune::FieldVector<MiddleFV, 1>;

      using MiddleMV = MyVector<InnerFV, 1>;
      using OuterMV = MyVector<MiddleMV, 1>;

      // MiddleFV mfv;
      OuterMV mv;
      OuterFV fv;

      static_assert(std::is_convertible<OuterMV, OuterFV>::value,
                    "DenseVectors should be convertible.");
      fv = mv;

      // before the fix, `is_assignable` returned `true`,
      static_assert(!std::is_assignable<MiddleFV&, OuterMV>::value,
                    "Inconsistent assignability detected.");
      // mfv = mv; // <- but this assignment failed instantiation
    }

    {
      using InnerFV = Dune::FieldMatrix<double, 2, 2>;
      using MiddleFV = Dune::FieldVector<InnerFV, 1>;
      using OuterFV = Dune::FieldVector<MiddleFV, 1>;

      using MiddleMV = MyVector<InnerFV, 1>;
      using OuterMV = MyVector<MiddleMV, 1>;

      // MiddleFV mfv;
      OuterMV mv;
      OuterFV fv;

      static_assert(std::is_assignable<OuterFV, OuterMV>::value,
                    "DenseVectors should be assignable.");
      fv = mv;

      // before the fix, `is_assignable` returned `true`,
      static_assert(!std::is_assignable<MiddleFV&, OuterMV>::value,
                    "Inconsistent assignability detected.");
      // mfv = mv; // <- but this assignment failed instantiation
    }
    return 0;
  } catch (Dune::Exception& e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }
}
