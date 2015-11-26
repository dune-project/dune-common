#include <dune/common/diagonalmatrix.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>

int main() try {
  bool passed = true;

  // Write beyond end of singleton vector
  try {
    Dune::FieldVector<double, 1> v = {1};
    v[1] = 10;
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Read beyond end of singleton vector
  try {
    Dune::FieldVector<double, 1> const v = {1};
    double const x = v[1];
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Write beyond end of vector
  try {
    Dune::FieldVector<double, 3> v = {1, 2, 3};
    v[3] = 10;
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Read beyond end of vector
  try {
    Dune::FieldVector<double, 3> const v = {1, 2, 3};
    double const x = v[3];
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Write beyond end of singleton matrix
  try {
    Dune::FieldMatrix<double, 1, 1> m = {{1}};
    m[1][0] = 100;
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Read beyond end of singleton matrix
  try {
    Dune::FieldMatrix<double, 1, 1> const m = {{1}};
    double const x = m[1][0];
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Write beyond end of matrix
  try {
    Dune::FieldMatrix<double, 2, 3> m = {{1, 2, 3}, {10, 20, 30}};
    m[2][0] = 100;
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Read beyond end of matrix
  try {
    Dune::FieldMatrix<double, 2, 3> const m = {{1, 2, 3}, {10, 20, 30}};
    double const x = m[2][0];
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Write beyond end of diagonal matrix (way #1)
  try {
    Dune::DiagonalMatrix<double, 3> d(5);
    d[3][3] = 9;
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // Write beyond end of diagonal matrix (way #2)
  try {
    Dune::DiagonalMatrix<double, 3> d(5);
    d.diagonal(3) = 9;
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Read beyond end of diagonal matrix (way #1)
  try {
    Dune::DiagonalMatrix<double, 3> const d(5);
    double const x = d[3][3];
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }
  // Read beyond end of diagonal matrix (way #2)
  try {
    Dune::DiagonalMatrix<double, 3> const d(5);
    double const x = d.diagonal(3);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Write outside of diagonal matrix pattern
  try {
    Dune::DiagonalMatrix<double, 3> d(5);
    d[1][2] = 9;
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Read outside of diagonal matrix pattern
  try {
    Dune::DiagonalMatrix<double, 3> const d(5);
    double const x = d[1][2];
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Check for entry beyond diagonal matrix size
  try {
    Dune::DiagonalMatrix<double, 3> const d(5);
    d.exists(3, 3);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
    std::cout << "(line " << __LINE__
              << ") All good: Exception thrown as expected." << std::endl;
  }

  // Check for entry beyond matrix size
  try {
    Dune::FieldMatrix<double, 2, 3> const m = {{1, 2, 3}, {10, 20, 30}};
    m.exists(2, 2);
    std::cout << "(line " << __LINE__ << ") Error: No exception thrown."
              << std::endl;
    passed = false;
  } catch (Dune::RangeError) {
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
