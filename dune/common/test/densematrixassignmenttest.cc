// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include "config.h"

#define DUNE_CHECK_BOUNDS

#include <complex>

#include <dune/common/boundschecking.hh>
#include <dune/common/diagonalmatrix.hh>
#include <dune/common/dynmatrix.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/gmpfield.hh>

template <class M>
void populateMatrix(M &A, int rows, int cols) {
  for (int i = 0; i < rows; ++i)
    for (int j = 0; j < cols; ++j)
      A[i][j] = i + 10 * j;
}


template< class K, int rows, int cols >
struct Foo
{
  constexpr static int M () noexcept { return cols; }
  constexpr static int N () noexcept { return rows; }

  operator Dune::FieldMatrix< K, rows, cols > () const
  {
    Dune::FieldMatrix< K, rows, cols > A;
    populateMatrix( A, rows, cols );
    return A;
  }
};

struct Bar {};

template <class A, class B>
bool identicalContents(A const &a, B const &b) {
  typedef typename A::size_type Size;

  if (a.N() != b.N() or a.M() != b.M())
    return false;

  for (Size i = 0; i < a.N(); ++i)
    for (Size j = 0; j < b.N(); ++j)
      if (a[i][j] != b[i][j])
        return false;
  return true;
}

template <typename ft>
bool run() {
  ft const constant = 47.11;
  std::cout << "Testing with type: " << Dune::className(constant) << std::endl;

  Dune::FieldMatrix<ft, 2, 3> fieldM;
  Dune::FieldMatrix<ft, 1, 1> fieldMWrong11;
  Dune::FieldMatrix<ft, 2, 2> fieldMWrong22;
  Dune::FieldMatrix<ft, 3, 3> fieldMWrong33;
  populateMatrix(fieldM, 2, 3);
  populateMatrix(fieldMWrong11, 1, 1);
  populateMatrix(fieldMWrong22, 2, 2);
  populateMatrix(fieldMWrong33, 3, 3);

  Foo< ft, 2, 3 > fooM;
  fieldM = static_cast< Dune::FieldMatrix< ft, 2, 3 > >( fooM );

  Dune::DynamicMatrix<ft> dynM(2, 3);
  Dune::DynamicMatrix<ft> dynMWrong11(1, 1);
  Dune::DynamicMatrix<ft> dynMWrong22(2, 2);
  Dune::DynamicMatrix<ft> dynMWrong33(3, 3);
  populateMatrix(dynM, 2, 3);
  populateMatrix(dynMWrong11, 1, 1);
  populateMatrix(dynMWrong22, 2, 2);
  populateMatrix(dynMWrong33, 3, 3);

  Dune::DiagonalMatrix<ft, 1> const diagMWrong1 = {1};
  Dune::DiagonalMatrix<ft, 2> const diagMWrong2 = {1, 2};
  Dune::DiagonalMatrix<ft, 3> const diagMWrong3 = {1, 2, 3};

  bool passed = true;

  static_assert(!Dune::HasDenseMatrixAssigner< Dune::FieldMatrix<ft, 2, 3>, std::vector< Dune::FieldMatrix<ft, 2, 3> > >::value,
                "FieldMatrix is not assignable by a std::vector< FieldMatrix >!");
  static_assert(!Dune::HasDenseMatrixAssigner< Dune::FieldMatrix<ft, 2, 3>, Bar >::value,
                "FieldMatrix is not assignable by a Bar!");
  static_assert(Dune::HasDenseMatrixAssigner< Dune::FieldMatrix<ft, 2, 3>, Dune::FieldMatrix<ft, 2, 3> >::value,
                "FieldMatrix is assignable by FieldMatrix!");
  static_assert(Dune::HasDenseMatrixAssigner< Dune::FieldMatrix<ft, 2, 3>, Dune::DynamicMatrix<ft> >::value,
                "FieldMatrix is assignable by a DynamicMatrix!");

  // class: FieldMatrix
  {
    using M = Dune::FieldMatrix<ft, 2, 3>;

    // Assignment
    {
      M fieldT;
      fieldT = fieldM;
      if (!identicalContents(fieldT, fieldM)) {
        std::cout << "FAIL: Content mismatch on line: " << __LINE__
                  << std::endl;
        passed = false;
      }
    }
    {
      M fieldT;
      fieldT = dynM;
      if (!identicalContents(fieldT, dynM)) {
        std::cout << "FAIL: Content mismatch on line: " << __LINE__
                  << std::endl;
        passed = false;
      }
    }
    {
      M fieldT;
      fieldT = constant;
    }

    // Copy construction
    {
      M const fieldT = fieldM;
      if (!identicalContents(fieldT, fieldM)) {
        std::cout << "FAIL: Content mismatch on line: " << __LINE__
                  << std::endl;
        passed = false;
      }
    }
    {
      M const fieldT = dynM;
      if (!identicalContents(fieldT, dynM)) {
        std::cout << "FAIL: Content mismatch on line: " << __LINE__
                  << std::endl;
        passed = false;
      }
    }
    {
      [[maybe_unused]] M const fieldT = constant;
    }
  }

  // class: DynamicMatrix
  {
    using M = Dune::DynamicMatrix<ft>;

    // Assignment
    {
      M dynT;
      dynT = fieldM;
      if (!identicalContents(dynT, fieldM)) {
        std::cout << "FAIL: Content mismatch on line: " << __LINE__
                  << std::endl;
        passed = false;
      }
      dynT = fieldMWrong11;
      if (!identicalContents(dynT, fieldMWrong11)) {
        std::cout << "FAIL: Content mismatch on line: " << __LINE__
                  << std::endl;
        passed = false;
      }
    }
    {
      M dynT;
      dynT = dynM;
      if (!identicalContents(dynT, dynM)) {
        std::cout << "FAIL: Content mismatch on line: " << __LINE__
                  << std::endl;
        passed = false;
      }
    }
    {
      M dynT;
      dynT = constant;
    }

    // Copy construction
    {
      M const dynT = fieldM;
      if (!identicalContents(dynT, fieldM)) {
        std::cout << "FAIL: Content mismatch on line: " << __LINE__
                  << std::endl;
        passed = false;
      }
    }
    {
      M const dynT = dynM;
      if (!identicalContents(dynT, dynM)) {
        std::cout << "FAIL: Content mismatch on line: " << __LINE__
                  << std::endl;
        passed = false;
      }
    }
  }

  // Assignment from other classes
  {
    using M = Dune::FieldMatrix<ft, 3, 3>;
    Dune::DiagonalMatrix<ft, 3> diagM({1, 2, 3});
    { [[maybe_unused]] M const fieldT = diagM; }
    {
      M fieldT;
      fieldT = diagM;
    }
  }
  {
    using M = Dune::DynamicMatrix<ft>;
    Dune::DiagonalMatrix<ft, 3> diagM({1, 2, 3});
    { [[maybe_unused]] M const dynT = diagM; }
    {
      M dynT;
      dynT = diagM;
    }
  }

  // Invalid assignments
  {
    using M = Dune::FieldMatrix<ft, 2, 3>;
#ifdef FAILURE0
    {
      // Should fail at compile-time
      M fieldT;
      fieldT = fieldMWrong11;
    }
#endif
#ifdef FAILURE1
    {
      // Should fail at compile-time
      M fieldT;
      fieldT = fieldMWrong22;
    }
#endif
#ifdef FAILURE2
    {
      // Should fail at compile-time
      M fieldT;
      fieldT = fieldMWrong33;
    }
#endif
    try {
      // Should fail at run-time with RangeError
      M fieldT;
      fieldT = dynMWrong11;
      std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
                << std::endl;
      passed = false;
    } catch (const Dune::RangeError&) {
      std::cout << "(line " << __LINE__
                << ") All good: Exception thrown as expected." << std::endl;
    }
    try {
      // Should fail at run-time with RangeError
      M fieldT;
      fieldT = dynMWrong22;
      std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
                << std::endl;
      passed = false;
    } catch (const Dune::RangeError&) {
      std::cout << "(line " << __LINE__
                << ") All good: Exception thrown as expected." << std::endl;
    }
    try {
      // Should fail at run-time with RangeError
      M fieldT;
      fieldT = dynMWrong33;
      std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
                << std::endl;
      passed = false;
    } catch (const Dune::RangeError&) {
      std::cout << "(line " << __LINE__
                << ") All good: Exception thrown as expected." << std::endl;
    }

    try {
      // Should fail at run-time with RangeError
      // Note: this could be made to fail at compile-time already if
      // we further specialised DenseMatrixAssigner to (FieldMatrix,
      // DiagonalMatrix)
      M fieldT;
      fieldT = diagMWrong1;
      std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
                << std::endl;
      passed = false;
    } catch (const Dune::RangeError&) {
      std::cout << "(line " << __LINE__
                << ") All good: Exception thrown as expected." << std::endl;
    }
    try {
      // Should fail at run-time with RangeError
      // Note: this could be made to fail at compile-time already if
      // we further specialised DenseMatrixAssigner to (FieldMatrix,
      // DiagonalMatrix)
      M fieldT;
      fieldT = diagMWrong2;
      std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
                << std::endl;
      passed = false;
    } catch (const Dune::RangeError&) {
      std::cout << "(line " << __LINE__
                << ") All good: Exception thrown as expected." << std::endl;
    }
    try {
      // Should fail at run-time with RangeError
      // Note: this could be made to fail at compile-time already if
      // we further specialised DenseMatrixAssigner to (FieldMatrix,
      // DiagonalMatrix)
      M fieldT;
      fieldT = diagMWrong3;
      std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
                << std::endl;
      passed = false;
    } catch (const Dune::RangeError&) {
      std::cout << "(line " << __LINE__
                << ") All good: Exception thrown as expected." << std::endl;
    }
  }

  // Invalid copy construction
  {
    using M = Dune::FieldMatrix<ft, 2, 3>;
#ifdef FAILURE3
    {
      // Should fail at compile-time
      [[maybe_unused]] M const fieldT = fieldMWrong11;
    }
#endif
#ifdef FAILURE4
    {
      // Should fail at compile-time
      [[maybe_unused]] M const fieldT = fieldMWrong22;
    }
#endif
#ifdef FAILURE5
    {
      // Should fail at compile-time
      [[maybe_unused]] M const fieldT = fieldMWrong33;
    }
#endif
    try {
      // Should fail at run-time with RangeError
      [[maybe_unused]] M const fieldT = dynMWrong11;
      std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
                << std::endl;
      passed = false;
    } catch (const Dune::RangeError&) {
      std::cout << "(line " << __LINE__
                << ") All good: Exception thrown as expected." << std::endl;
    }
    try {
      // Should fail at run-time with RangeError
      [[maybe_unused]] M const fieldT = dynMWrong22;
      std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
                << std::endl;
      passed = false;
    } catch (const Dune::RangeError&) {
      std::cout << "(line " << __LINE__
                << ") All good: Exception thrown as expected." << std::endl;
    }
    try {
      // Should fail at run-time with RangeError
      [[maybe_unused]] M const fieldT = dynMWrong33;
      std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
                << std::endl;
      passed = false;
    } catch (const Dune::RangeError&) {
      std::cout << "(line " << __LINE__
                << ") All good: Exception thrown as expected." << std::endl;
    }
  }
  {
#ifdef FAILURE6
    using M = Dune::DynamicMatrix<ft>;
    {
      // Should fail at compile-time
      [[maybe_unused]] M const dynT = constant;
    }
#endif
  }
  std::cout << std::endl;
  return passed;
}

int main() {
  bool passed = true;
  passed = passed && run<double>();
  passed = passed && run<std::complex<double>>();
#ifdef HAVE_GMP
  passed = passed && run<Dune::GMPField<128u>>();
#endif
  return passed ? 0 : 1;
}
