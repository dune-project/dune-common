// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include "config.h"

#include <type_traits>
#include <string>
#include <cstddef>

#include <dune/common/transpose.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/diagonalmatrix.hh>
#include <dune/common/classname.hh>
#include <dune/common/test/testsuite.hh>

template<class A>
void checkTranspose(Dune::TestSuite& suite, A a)
{
  // Check with value capture
  {
    [[maybe_unused]] auto at = transpose(a);
    auto at2 = transpose(a);
    at = at2;
    at = transpose(a);
  }

  // Check with reference capture by std::ref
  {
    [[maybe_unused]] auto at = transpose(std::ref(a));
    [[maybe_unused]] auto at_dense = at.asDense();
    auto at2 = transpose(std::ref(a));
    at = at2;
    at = transpose(std::ref(a));
  }

  // Check with reference capture by std::cref
  {
    [[maybe_unused]] auto at = transpose(std::cref(a));
    [[maybe_unused]] auto at_dense = at.asDense();
    auto at2 = transpose(std::cref(a));
    at = at2;
    at = transpose(std::cref(a));
  }

  // Check transposedView
  {
    [[maybe_unused]] auto at = transposedView(a);
    [[maybe_unused]] auto at_dense = at.asDense();
    auto at2 = transposedView(a);
    at = at2;
    at = transposedView(a);
  }
}


// check A*transpose(B)
template<class A, class B, class BT>
auto checkAxBT(const A& a, const B&b, const BT&bt)
{
  Dune::TestSuite suite(std::string{"Check transpose with A="} + Dune::className<A>() + " and B=" + Dune::className<B>() + " and BT=" + Dune::className<BT>());

  // compute abt
  auto abt = a * bt;

  // check result type
  using FieldA = typename Dune::FieldTraits<A>::field_type;
  using FieldB = typename Dune::FieldTraits<B>::field_type;
  using Field = typename Dune::PromotionTraits<FieldA, FieldB>::PromotedType;

  // manually compute result value
  auto abt_check = Dune::DynamicMatrix<Field>{a.N(), b.N()};
  abt_check = 0;
  for(std::size_t i=0; i<a.N(); ++i)
    for(std::size_t j=0; j<b.N(); ++j)
      for(auto&& [a_ik, k] : Dune::sparseRange(a[i]))
        for(auto&& [b_jl, l] : Dune::sparseRange(b[j]))
          if (std::size_t(k) == std::size_t(l))
            abt_check[i][j] += a_ik*b_jl;

  // check result value
  bool equal = true;
  for(std::size_t i=0; i<a.N(); ++i)
    for(std::size_t j=0; j<b.N(); ++j)
      equal = equal and (abt_check[i][j] == abt[i][j]);
  suite.check(equal)
    << "Result of A*transpose(B) should be \n" << abt_check << " but is \n" << abt;

  return suite;
}

template<class A, class B>
void checkTransposeProduct(Dune::TestSuite& suite, A a, B b_original)
{
  // Check with value capture
  {
    auto b = b_original;
    auto bt = transpose(b);
    suite.subTest(checkAxBT(a,b_original,bt));
    // Check if bt was captured by value
    b = b_original;
    b *= 2;
    suite.subTest(checkAxBT(a,b_original,bt));
  }

  // Check with reference capture by std::ref
  {
    auto b = b_original;
    auto bt = transpose(std::ref(b));
    suite.subTest(checkAxBT(a,b,bt));
    // Check if bt was captured by reference
    b *= 2;
    suite.subTest(checkAxBT(a,b,bt));
  }

  // Check conversion to dense matrix of wrapper
  {
    auto b = b_original;
    auto bt = transpose(std::ref(b)).asDense();
    suite.subTest(checkAxBT(a,b,bt));
  }

  // Check with reference capture by std::ref
  {
    auto b = b_original;
    auto bt = transposedView(b);
    suite.subTest(checkAxBT(a,b,bt));
    // Check if bt was captured by reference
    b *= 2;
    suite.subTest(checkAxBT(a,b,bt));
  }

  // Check conversion to dense matrix of wrapper
  {
    auto b = b_original;
    auto bt = transposedView(b).asDense();
    suite.subTest(checkAxBT(a,b,bt));
  }

}


int main()
{
  Dune::TestSuite suite;
  std::size_t seed=0;

  // fill dense matrix with test data
  auto testFillDense = [k=seed](auto& matrix) mutable {
    for(std::size_t i=0; i<matrix.N(); ++i)
      for(std::size_t j=0; j<matrix.M(); ++j)
        matrix[i][j] = k++;
  };

  {
    auto a = Dune::FieldMatrix<double,1,1>{};
    auto b = Dune::FieldMatrix<double,1,1>{};
    a = 2;
    b = 3;
    checkTranspose(suite,a);
    checkTranspose(suite,b);
    checkTransposeProduct(suite,a,b);
  }

  {
    auto a = Dune::DiagonalMatrix<double,1>{};
    auto b = Dune::FieldMatrix<double,1,1>{};
    a = 2;
    b = 3;
    checkTranspose(suite,a);
    checkTranspose(suite,b);
    checkTransposeProduct(suite,a,b);
  }

  {
    auto a = Dune::DiagonalMatrix<double,1>{};
    auto b = Dune::DiagonalMatrix<double,1>{};
    a = 2;
    b = 3;
    checkTranspose(suite,a);
    checkTranspose(suite,b);
    checkTransposeProduct(suite,a,b);
  }

  {
    auto a = Dune::DiagonalMatrix<double,1>{};
    auto b = Dune::FieldMatrix<double,4,1>{};
    a = 2;
    testFillDense(b);
    checkTranspose(suite,a);
    checkTranspose(suite,b);
    checkTransposeProduct(suite,a,b);
  }

  {
    auto a = Dune::FieldMatrix<double,1,1>{};
    auto b = Dune::FieldMatrix<double,4,1>{};
    a = 2;
    testFillDense(b);
    checkTranspose(suite,a);
    checkTranspose(suite,b);
    checkTransposeProduct(suite,a,b);
  }

  {
    auto a = Dune::DiagonalMatrix<double,2>{};
    auto b = Dune::FieldMatrix<double,2,2>{};
    a = {0, 1};
    testFillDense(b);
    checkTranspose(suite,a);
    checkTranspose(suite,b);
    checkTransposeProduct(suite,a,b);
  }

  {
    auto a = Dune::FieldMatrix<double,3,4>{};
    auto b = Dune::FieldMatrix<double,7,4>{};
    testFillDense(a);
    testFillDense(b);
    checkTranspose(suite,a);
    checkTranspose(suite,b);
    checkTransposeProduct(suite,a,b);
  }

  {
    auto a = Dune::FieldMatrix<double,1,2>{};
    auto b = Dune::FieldMatrix<double,3,2>{};
    testFillDense(a);
    testFillDense(b);
    checkTranspose(suite,a);
    checkTranspose(suite,b);
    checkTransposeProduct(suite,a,b);
  }

  {
    auto a = Dune::FieldMatrix<double,1,2>{};
    auto b = Dune::FieldMatrix<double,1,2>{};
    testFillDense(a);
    testFillDense(b);
    checkTranspose(suite,a);
    checkTranspose(suite,b);
    checkTransposeProduct(suite,a,b);
  }

  {
    auto a = Dune::FieldMatrix<double,7,4>{};
    auto b = Dune::DiagonalMatrix<double,4>{};
    testFillDense(a);
    b = {0, 1, 2, 3};
    checkTranspose(suite,a);
    checkTranspose(suite,b);
    checkTransposeProduct(suite,a,b);
  }

  {
    auto a = Dune::DiagonalMatrix<double,2>{};
    auto b = Dune::FieldMatrix<double,2,2>{};
    a = {0, 1};
    testFillDense(b);
    checkTranspose(suite,a);
    checkTranspose(suite,b);
    checkTransposeProduct(suite,a,b);
  }

  {
    auto a = Dune::DiagonalMatrix<double,4>{};
    auto b = Dune::FieldMatrix<double,7,4>{};
    a = {0, 1, 2, 3};
    testFillDense(b);
    checkTranspose(suite,a);
    checkTranspose(suite,b);
    checkTransposeProduct(suite,a,b);
  }

  {
    auto a = Dune::FieldMatrix<double,3,4>{};
    auto b = Dune::DynamicMatrix<double>(4,4);
    testFillDense(a);
    testFillDense(b);
    checkTranspose(suite,a);
    checkTranspose(suite,b);
    suite.subTest(checkAxBT(a,b,transposedView(b)));
    [[maybe_unused]] auto abt = a * transposedView(b);
  }
  return suite.exit();
}
