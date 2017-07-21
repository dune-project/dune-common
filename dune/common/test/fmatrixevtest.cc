#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iomanip>

#include <dune/common/fmatrix.hh>
#include <dune/common/fmatrixev.hh>
#include <dune/common/fvector.hh>

template <int n, typename T>
void printNative(Dune::FieldMatrix<T, n, n> const &mat) {
  Dune::FieldVector<T, n> vec;
  Dune::FMatrixHelp::eigenValues(mat, vec);
  std::cout << "native: " << vec << std::endl;
}

template <int n, typename T>
void printLapack(Dune::FieldMatrix<T, n, n> const &mat) {
  Dune::FieldVector<T, n> vec;
  Dune::FMatrixHelp::eigenValues<n, T>(mat, vec);
  std::cout << "lapack: " << vec << std::endl;
}

template <int n, typename T>
void printLapackGeneric(Dune::FieldMatrix<T, n, n> const &mat) {
  Dune::FieldVector<std::complex<T>, n> vec;
  Dune::FMatrixHelp::eigenValuesNonSym<n, T>(mat, vec);
  std::cout << "lapack: " << vec << std::endl;
}

template <int n, typename T>
void printSolution(Dune::FieldVector<T, n> const &sol) {
  std::cout << "exact:  " << sol << std::endl;
}

template <typename T> void test0() {
  std::cout << "\n2x2: mat #0 (symmetric):" << std::endl;
  Dune::FieldMatrix<T, 2, 2> mat = {{1, 2}, {2, 3}};
  // computed via maxima
  Dune::FieldVector<double, 2> sol = {2 - sqrt(5), 2 + sqrt(5)};
  printNative(mat);
  printLapack(mat);
  printSolution(sol);

  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  printNative(mat);
  printLapack(mat);
  printSolution(sol);
}

template <typename T> void test1() {
  std::cout << "\n3x3: mat #1 (symmetric):" << std::endl;
  Dune::FieldMatrix<T, 3, 3> mat = {{1, 2, 3}, {2, 4, 5}, {3, 5, 6}};
  printNative(mat);
  printLapack(mat);

  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  printNative(mat);
  printLapack(mat);
}

template <typename T> void test2() {
  std::cout << "\n3x3: mat #2 (symmetric):" << std::endl;
  Dune::FieldMatrix<T, 3, 3> mat = {{4, 0, 0}, {0, 5, 0}, {0, 0, 6}};
  printNative(mat);
  printLapack(mat);

  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  printNative(mat);
  printLapack(mat);
}

template <typename T> void test2b() {
  std::cout << "\n3x3: mat #2b (symmetric):" << std::endl;
  Dune::FieldMatrix<T, 3, 3> mat = {{6, 0, 0}, {0, 6, 0}, {0, 0, 6}};
  printNative(mat);
  printLapack(mat);

  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  printNative(mat);
  printLapack(mat);
}

template <typename T> void test3() {
  std::cout << "\n3x3: mat #3 (symmetric):" << std::endl;
  Dune::FieldMatrix<T, 3, 3> mat = {
      {16165, 5444, 5444}, {5444, 16165, 5444}, {5444, 5444, 17815}};
  // computed via maxima
  Dune::FieldVector<double, 3> sol = {10721, 19712 - sqrt(62872881.),
                                      19712 + sqrt(62872881.)};
  printNative(mat);
  printLapack(mat);
  printSolution(sol);

  std::cout << "Rescaling matrix: mat -> 1e-10*mat" << std::endl;
  mat *= 1e-10;
  sol *= 1e-10;
  printNative(mat);
  printLapack(mat);
  printSolution(sol);
}

template <typename T> void test4() {
  std::cout << "\n3x3: mat #4 (symmetric):" << std::endl;
  Dune::FieldMatrix<T, 3, 3> mat = {{1, 1, 2}, {1, 1, 2}, {2, 2, 2}};
  // computed via maxima
  Dune::FieldVector<double, 3> sol = {2 * (1 - sqrt(2)), 0, 2 * (1 + sqrt(2))};
  printNative(mat);
  printLapack(mat);
  printSolution(sol);

  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  printNative(mat);
  printLapack(mat);
  printSolution(sol);
}

template <typename T> void test5() {
  std::cout << "\n2x2: mat #5 (real eigenvalues):" << std::endl;
  Dune::FieldMatrix<T, 2, 2> mat = {{-19., 22.}, {11., -8.}};
  // computed via maxima
  Dune::FieldVector<std::complex<double>, 2> sol = {{-30, 0}, {3, 0}};
  printLapackGeneric(mat);
  printSolution(sol);
}

template <typename T> void test6() {
  std::cout << "\n2x2: mat #6 (complex eigenvalues):" << std::endl;
  Dune::FieldMatrix<T, 2, 2> mat = {{1, -2}, {2, 1}};
  // computed via maxima
  Dune::FieldVector<std::complex<double>, 2> sol = {{1, 2},
                                                    {1, -2}}; // FIXME: order?
  printLapackGeneric(mat);
  printSolution(sol);
}

template <typename T> void test7() {
  std::cout << "\n3x3: mat #7 (complex eigenvalues):" << std::endl;
  Dune::FieldMatrix<T, 3, 3> mat = {{1, -2, 1}, {2, 1, 1}, {1, 1, 1}};
  // computed via maxima
  Dune::FieldVector<std::complex<double>, 3> sol = {
      {1, +sqrt(2)}, {1, -sqrt(2)}, {1, 0}}; // FIXME: order?
  printLapackGeneric(mat);
  printSolution(sol);
}

int main() {
  std::cout << std::showpos << std::scientific
            << std::setprecision(std::numeric_limits<float>::digits10)
            << "float:";
  test0<float>();
  test1<float>();
  test2<float>();
  test2b<float>();
  test3<float>();
  test4<float>();
  test5<float>();
  test6<float>();
  test7<float>();

  std::cout << std::setprecision(std::numeric_limits<double>::digits10)
            << "\n\ndouble:";
  test0<double>();
  test1<double>();
  test2<double>();
  test2b<double>();
  test3<double>();
  test4<double>();
  test5<double>();
  test6<double>();
  test7<double>();
}
