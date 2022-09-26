// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>

int main() try {
  bool passed = true;

  // Add vectors of different sizes
  try {
    Dune::FieldVector<double, 3> v1 = {1, 2, 3};
    Dune::FieldVector<double, 2> const v2 = {1, 2};
    v1 += v2;
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Subtract vectors of different sizes
  try {
    Dune::FieldVector<double, 3> v1 = {1, 2, 3};
    Dune::FieldVector<double, 2> const v2 = {1, 2};
    v1 -= v2;
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Check vectors of different sizes for equality
  try {
    Dune::FieldVector<double, 3> const v1 = {1, 2, 3};
    Dune::FieldVector<double, 2> const v2 = {1, 2};
    [[maybe_unused]] bool res = (v1 == v2);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Check vectors of different sizes for inequality
  try {
    Dune::FieldVector<double, 3> const v1 = {1, 2, 3};
    Dune::FieldVector<double, 2> const v2 = {1, 2};
    [[maybe_unused]] bool res = (v1 != v2);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Apply axpy to vectors of different sizes
  try {
    Dune::FieldVector<double, 3> v1 = {1, 2, 3};
    Dune::FieldVector<double, 2> const v2 = {1, 2};
    v1.axpy(2, v2);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  try {
    Dune::FieldMatrix<double, 1, 3> m1 = {{1, 2, 3}};
    Dune::FieldMatrix<double, 2, 3> const m2 = {{1, 2, 3}, {10, 20, 30}};
    m1 += m2;
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  try {
    Dune::FieldMatrix<double, 1, 3> m1 = {{1, 2, 3}};
    Dune::FieldMatrix<double, 2, 3> const m2 = {{1, 2, 3}, {10, 20, 30}};
    m1 -= m2;
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  try {
    Dune::FieldMatrix<double, 1, 3> m1 = {{1, 2, 3}};
    Dune::FieldMatrix<double, 2, 3> const m2 = {{1, 2, 3}, {10, 20, 30}};
    [[maybe_unused]] bool res = (m1 == m2);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  try {
    Dune::FieldMatrix<double, 1, 3> m1 = {{1, 2, 3}};
    Dune::FieldMatrix<double, 2, 3> const m2 = {{1, 2, 3}, {10, 20, 30}};
    [[maybe_unused]] bool res = (m1 != m2);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (const Dune::RangeError&) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  try {
    Dune::FieldMatrix<double, 1, 3> m1 = {{1, 2, 3}};
    Dune::FieldMatrix<double, 2, 3> const m2 = {{1, 2, 3}, {10, 20, 30}};
    m1.axpy(2, m2);
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
