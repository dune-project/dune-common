#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iomanip>

#include <dune/common/fmatrix.hh>
#include <dune/common/fmatrixev.hh>
#include <dune/common/fvector.hh>

template <int n, typename T>
void printNative(Dune::FieldMatrix<T, n, n> const &mat)
{
  Dune::FieldVector<T, n> vec;
  Dune::FMatrixHelp::eigenValues(mat, vec);
  std::cout << "native: " << vec << std::endl;
}

template <int n, typename T>
void printLapack(Dune::FieldMatrix<T, n, n> const &mat)
{
  Dune::FieldVector<T, n> vec;
  Dune::FMatrixHelp::eigenValues<n, T>(mat, vec);
  std::cout << "lapack: " << vec << std::endl;
}

template <typename T>
void test0() {
    std::cout << "2x2: mat #0:" << std::endl;
    Dune::FieldMatrix<T, 2, 2> mat = {{1,2},{2,4-1e-4}};
    printNative(mat);
    printLapack(mat);

    std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
    mat *= 1e-5;
    printNative(mat);
    printLapack(mat);
}

template <typename T>
void test1() {
    std::cout << "3x3: mat #1:" << std::endl;
    Dune::FieldMatrix<T, 3, 3> mat = {{1,2,3},{2,4,5},{3,5,6}};
    printNative(mat);
    printLapack(mat);

    std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
    mat *= 1e-5;
    printNative(mat);
    printLapack(mat);
}

template <typename T>
void test2() {
    std::cout << "3x3: mat #2:" << std::endl;
    Dune::FieldMatrix<T, 3, 3> mat = {{6,0,0},{0,6,0},{0,0,6}};
    printNative(mat);
    printLapack(mat);

    std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
    mat *= 1e-5;
    printNative(mat);
    printLapack(mat);
}

int main() {
  std::cout << std::showpos << std::scientific << std::setprecision(16) << std::endl;

  std::cout << "float:\n";
  test0<float>();
  std::cout << "\n";
  test1<float>();
  std::cout << "\n";
  test2<float>();

  std::cout << "\n\ndouble:\n";
  test0<double>();
  std::cout << "\n";
  test1<double>();
  std::cout << "\n";
  test2<double>();
}
