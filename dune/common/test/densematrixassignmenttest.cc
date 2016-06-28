#include "config.h"

#define DUNE_CHECK_BOUNDS

#include <complex>

#include <dune/common/boundschecking.hh>
#include <dune/common/diagonalmatrix.hh>
#include <dune/common/dynmatrix.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/gmpfield.hh>
#include <dune/common/identitymatrix.hh>
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

  Dune::IdentityMatrix<ft, 1> const identMWrong1{};
  Dune::IdentityMatrix<ft, 2> const identMWrong2{};
  Dune::IdentityMatrix<ft, 3> const identMWrong3{};

  bool passed = true;

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
      // Intentionally disabled: ( compile-time size mismatch )
      // fieldT = Dune::FieldMatrix<ft, 1, 1>(constant);
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
      DUNE_UNUSED M const fieldT(constant);
      // Intentionally disabled:
      // M const fieldT2 = constant;
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
    Dune::IdentityMatrix<ft, 3> identM;
    { DUNE_UNUSED M const fieldT = diagM; }
    {
      M fieldT;
      fieldT = diagM;
    }
    { DUNE_UNUSED M const fieldT = identM; }
    {
      M fieldT;
      fieldT = identM;
    }
  }
  {
    using M = Dune::DynamicMatrix<ft>;
    Dune::DiagonalMatrix<ft, 3> diagM({1, 2, 3});
    Dune::IdentityMatrix<ft, 3> identM;
    { DUNE_UNUSED M const dynT = diagM; }
    {
      M dynT;
      dynT = diagM;
    }
    { DUNE_UNUSED M const dynT = identM; }
    {
      M dynT;
      dynT = identM;
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
    } catch (Dune::RangeError) {
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
      fieldT = diagMWrong1;
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

    try {
      // Should fail at run-time with RangeError
      // Note: this could be made to fail at compile-time already if
      // we further specialised DenseMatrixAssigner to (FieldMatrix,
      // IdentityMatrix)
      M fieldT;
      fieldT = identMWrong1;
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
      // IdentityMatrix)
      M fieldT;
      fieldT = identMWrong2;
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
      // IdentityMatrix)
      M fieldT;
      fieldT = identMWrong3;
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
    using M = Dune::FieldMatrix<ft, 2, 3>;
#ifdef FAILURE3
    {
      // Should fail at compile-time
      DUNE_UNUSED M const fieldT = fieldMWrong11;
    }
#endif
#ifdef FAILURE4
    {
      // Should fail at compile-time
      DUNE_UNUSED M const fieldT = fieldMWrong22;
    }
#endif
#ifdef FAILURE5
    {
      // Should fail at compile-time
      DUNE_UNUSED M const fieldT = fieldMWrong33;
    }
#endif
    try {
      // Should fail at run-time with RangeError
      DUNE_UNUSED M const fieldT = dynMWrong11;
      std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
                << std::endl;
      passed = false;
    } catch (Dune::RangeError) {
      std::cout << "(line " << __LINE__
                << ") All good: Exception thrown as expected." << std::endl;
    }
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
