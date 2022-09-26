// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/dynmatrix.hh>

int main() try {
  bool passed = true;

  // Free matrix-vector multiplication (mv): Input size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 2> x = {1, 2};
    Dune::FieldVector<double, 2> b(0);
    Dune::DenseMatrixHelp::multAssign(A,x,b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // Free matrix-vector multiplication (mv): Output size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 3> x = {1, 2, 3};
    Dune::FieldVector<double, 3> b(0);
    Dune::DenseMatrixHelp::multAssign(A,x,b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Matrix-vector multiplication (mv): Input size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 2> x = {1, 2};
    Dune::FieldVector<double, 2> b(0);
    A.mv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // Matrix-vector multiplication (mv): Output size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 3> x = {1, 2, 3};
    Dune::FieldVector<double, 3> b(0);
    A.mv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Matrix-vector multiplication (mtv): Input size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 3> x = {1, 2, 3};
    Dune::FieldVector<double, 3> b(0);
    A.mtv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // Matrix-vector multiplication (mtv): Output size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 2> x = {1, 2};
    Dune::FieldVector<double, 2> b(0);
    A.mtv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Matrix-vector multiplication (umv): Input size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 2> x = {1, 2};
    Dune::FieldVector<double, 2> b(0);
    A.umv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // Matrix-vector multiplication (umv): Output size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 3> x = {1, 2, 3};
    Dune::FieldVector<double, 3> b(0);
    A.umv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Matrix-vector multiplication (umtv): Input size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 3> x = {1, 2, 3};
    Dune::FieldVector<double, 3> b(0);
    A.umtv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // Matrix-vector multiplication (umtv): Output size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 2> x = {1, 2};
    Dune::FieldVector<double, 2> b(0);
    A.umtv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Matrix-vector multiplication (umhv): Input size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 3> x = {1, 2, 3};
    Dune::FieldVector<double, 3> b(0);
    A.umhv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // Matrix-vector multiplication (umhv): Output size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 2> x = {1, 2};
    Dune::FieldVector<double, 2> b(0);
    A.umhv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Matrix-vector multiplication (mmv): Input size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 2> x = {1, 2};
    Dune::FieldVector<double, 2> b(0);
    A.mmv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // Matrix-vector multiplication (mmv): Output size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 3> x = {1, 2, 3};
    Dune::FieldVector<double, 3> b(0);
    A.mmv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Matrix-vector multiplication (mmtv): Input size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 3> x = {1, 2, 3};
    Dune::FieldVector<double, 3> b(0);
    A.mmtv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // Matrix-vector multiplication (mmtv): Output size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 2> x = {1, 2};
    Dune::FieldVector<double, 2> b(0);
    A.mmtv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Matrix-vector multiplication (mmhv): Input size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 3> x = {1, 2, 3};
    Dune::FieldVector<double, 3> b(0);
    A.mmhv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // Matrix-vector multiplication (mmhv): Output size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 2> x = {1, 2};
    Dune::FieldVector<double, 2> b(0);
    A.mmhv(x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Matrix-vector multiplication (usmv): Input size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 2> x = {1, 2};
    Dune::FieldVector<double, 2> b(0);
    A.usmv(2, x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // Matrix-vector multiplication (usmv): Output size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 3> x = {1, 2, 3};
    Dune::FieldVector<double, 3> b(0);
    A.usmv(2, x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Matrix-vector multiplication (usmtv): Input size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 3> x = {1, 2, 3};
    Dune::FieldVector<double, 3> b(0);
    A.usmtv(2, x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // Matrix-vector multiplication (usmtv): Output size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 2> x = {1, 2};
    Dune::FieldVector<double, 2> b(0);
    A.usmtv(2, x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Matrix-vector multiplication (usmhv): Input size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 3> x = {1, 2, 3};
    Dune::FieldVector<double, 3> b(0);
    A.usmhv(2, x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // Matrix-vector multiplication (usmhv): Output size incorrect
  try {
    Dune::FieldMatrix<double, 2, 3> A = {{1, 2, 3}, {10, 20, 30}};
    Dune::FieldVector<double, 2> x = {1, 2};
    Dune::FieldVector<double, 2> b(0);
    A.usmhv(2, x, b);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // right multiplication: None-square Matrix argument
  try {
    Dune::DynamicMatrix<double> A(2, 3, 5);
    Dune::DynamicMatrix<double> const B(3, 2, 5);
    A.rightmultiply(B);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // right multiplication: Incorrect number of rows
  try {
    Dune::DynamicMatrix<double> A(2, 2, 5);
    Dune::DynamicMatrix<double> const B(3, 3, 5);
    A.rightmultiply(B);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // left multiplication: None-square Matrix argument
  try {
    Dune::FieldMatrix<double, 3, 2> A = {{1, 2}, {10, 20}, {100, 200}};
    Dune::FieldMatrix<double, 2, 3> const B = {{1, 2, 3}, {10, 20, 30}};
    A.leftmultiply(B);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // left multiplication: Incorrect number of rows
  try {
    Dune::FieldMatrix<double, 3, 3> A = {
        {1, 2, 3}, {10, 20, 30}, {100, 200, 300}};
    Dune::FieldMatrix<double, 3, 2> const B = {{1, 2}, {10, 20}, {100, 200}};
    A.leftmultiply(B);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  return passed ? 0 : 1;
} catch (Dune::Exception &e) {
  std::cerr << e << std::endl;
  return 1;
} catch (std::exception &e) {
  std::cerr << e.what() << std::endl;
  return 1;
}
