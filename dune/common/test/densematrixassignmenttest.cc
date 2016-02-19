#include "config.h"

#define DUNE_CHECK_BOUNDS

#include <dune/common/boundschecking.hh>
#include <dune/common/diagonalmatrix.hh>
#include <dune/common/dynmatrix.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/unused.hh>

template <class M>
void populateMatrix(M &A, int rows, int cols) {
  for (size_t i = 0; i < rows; ++i)
    for (size_t j = 0; j < cols; ++j)
      A[i][j] = i + 10 * j;
}

template <class A, class B>
bool identicalContents(A const &a, B const &b) {
  if (a.N() != b.N() or a.M() != b.M())
    return false;

  for (int i = 0; i < a.N(); ++i)
    for (int j = 0; j < b.N(); ++j)
      if (a[i][j] != b[i][j])
        return false;
  return true;
}

int main() {
  double const constant = 47.11;

  Dune::FieldMatrix<double, 2, 3> fieldM;
  Dune::FieldMatrix<double, 2, 2> fieldMWrong22;
  Dune::FieldMatrix<double, 3, 3> fieldMWrong33;
  populateMatrix(fieldM, 2, 3);
  populateMatrix(fieldMWrong22, 2, 2);
  populateMatrix(fieldMWrong33, 3, 3);

  Dune::DynamicMatrix<double> dynM(2, 3);
  Dune::DynamicMatrix<double> dynMWrong22(2, 2);
  Dune::DynamicMatrix<double> dynMWrong33(3, 3);
  populateMatrix(dynM, 2, 3);
  populateMatrix(dynMWrong22, 2, 2);
  populateMatrix(dynMWrong33, 3, 3);

  Dune::DiagonalMatrix<double, 2> const diagMWrong2({1, 2});
  Dune::DiagonalMatrix<double, 3> const diagMWrong3({1, 2, 3});

  bool passed = true;

  // class: FieldMatrix
  {
    using M = Dune::FieldMatrix<double, 2, 3>;

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
      DUNE_UNUSED M const fieldT = constant; // TODO: do we want this?
    }
  }

  // class: DynamicMatrix
  {
    using M = Dune::DynamicMatrix<double>;

    // Assignment
    {
      M dynT;
      dynT = fieldM;
      if (!identicalContents(dynT, fieldM)) {
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
      // Note: this has the same effect!
      // dynT = Dune::FieldMatrix<double, 1, 1>(constant);
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
    using M = Dune::FieldMatrix<double, 3, 3>;
    Dune::DiagonalMatrix<double, 3> diagM({1,2,3});
    {
      M const fieldT = diagM;
    }
    {
      M fieldT;
      fieldT = diagM;
    }
  }
  {
    using M = Dune::DynamicMatrix<double>;
    Dune::DiagonalMatrix<double, 3> diagM({1,2,3});
    {
      M const dynT = diagM;
    }
    {
      M dynT;
      dynT = diagM;
    }
  }

  // Invalid assignments
  {
    using M = Dune::FieldMatrix<double, 2, 3>;
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
      fieldT = dynMWrong22;
      std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
                << std::endl;
      passed = false;
    } catch (Dune::RangeError) {
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
    } catch (Dune::RangeError) {
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
    } catch (Dune::RangeError) {
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
    } catch (Dune::RangeError) {
      std::cout << "(line " << __LINE__
                << ") All good: Exception thrown as expected." << std::endl;
    }
  }

  // Invalid copy construction
  {
    using M = Dune::FieldMatrix<double, 2, 3>;
#ifdef FAILURE3
    {
      // Should fail at compile-time
      DUNE_UNUSED M const fieldT = fieldMWrong22;
    }
#endif
#ifdef FAILURE4
    {
      // Should fail at compile-time
      DUNE_UNUSED M const fieldT = fieldMWrong33;
    }
#endif
    try {
      // Should fail at run-time with RangeError
      DUNE_UNUSED M const fieldT = dynMWrong22;
      std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
                << std::endl;
      passed = false;
    } catch (Dune::RangeError) {
      std::cout << "(line " << __LINE__
                << ") All good: Exception thrown as expected." << std::endl;
    }
    try {
      // Should fail at run-time with RangeError
      DUNE_UNUSED M const fieldT = dynMWrong33;
      std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
                << std::endl;
      passed = false;
    } catch (Dune::RangeError) {
      std::cout << "(line " << __LINE__
                << ") All good: Exception thrown as expected." << std::endl;
    }
  }
  return passed ? 0 : 1;
}
