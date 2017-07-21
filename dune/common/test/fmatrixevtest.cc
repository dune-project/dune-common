#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <algorithm>
#include <iomanip>

#include <dune/common/fmatrix.hh>
#include <dune/common/fmatrixev.hh>
#include <dune/common/fvector.hh>

#include <dune/common/gmpfield.hh>

template <int n, typename T>
auto diff(Dune::FieldVector<T, n> const &x, Dune::FieldVector<T, n> const &y) {
  Dune::FieldVector<T, n> tmp = x;
  tmp -= y;
  return tmp.infinity_norm();
}

template <int n, typename T>
Dune::FieldVector<T, n> getNative(Dune::FieldMatrix<T, n, n> const &mat) {
  Dune::FieldVector<T, n> ret;
  Dune::FMatrixHelp::eigenValues(mat, ret);
  return ret;
}

template <typename T, int n>
bool handleNative(Dune::FieldMatrix<T, n, n> const &mat,
                  Dune::FieldVector<T, n> const &sol, T scale = 1) {
  auto eps = std::numeric_limits<T>::epsilon();
  auto native = getNative(mat);
  auto diffNative = diff(native, sol);
  std::cout << "diff(native, exact) = " << diffNative << std::endl;
  return diffNative < 10 * eps * scale;
}

template <int n, typename T>
Dune::FieldVector<T, n>
getLapackRealSymmetric(Dune::FieldMatrix<T, n, n> const &mat) {
  Dune::FieldVector<T, n> ret;
  Dune::FMatrixHelp::eigenValues<n, T>(mat, ret);
  return ret;
}

template <int n, typename T>
bool handleLapackRealSymmetric(Dune::FieldMatrix<T, n, n> const &mat,
                               Dune::FieldVector<T, n> const &sol,
                               T scale = 1) {
  auto eps = std::numeric_limits<T>::epsilon();
  auto lapack = getLapackRealSymmetric(mat);
  auto diffLapack = diff(lapack, sol);
  std::cout << "diff(lapack, exact) = " << diffLapack << std::endl;
  return diffLapack < 10 * eps * scale;
}

template <int n, typename T>
Dune::FieldVector<std::complex<T>, n>
getLapackRealGeneric(Dune::FieldMatrix<T, n, n> const &mat) {
  Dune::FieldVector<std::complex<T>, n> ret;
  Dune::FMatrixHelp::eigenValuesNonSym<n, T>(mat, ret);
  return ret;
}

template <int n, typename T>
bool handleLapackRealGeneric(Dune::FieldMatrix<T, n, n> const &mat,
                             Dune::FieldVector<std::complex<T>, n> const &sol,
                             T scale = 1) {
  auto eps = std::numeric_limits<T>::epsilon();
  auto lapack = getLapackRealGeneric(mat);
  auto diffLapack = diff(lapack, sol);
  std::cout << "diff(lapack, exact) = " << diffLapack << std::endl;
  return diffLapack < 10 * eps * scale;
}

template <int n, typename T>
Dune::FieldVector<T, n>
getLapackComplexHermitian(Dune::FieldMatrix<std::complex<T>, n, n> const &mat) {
  Dune::FieldVector<T, n> ret;
  // FIXME: Currently not exposed
  Dune::Impl::LapackEigenvalueHelper<T, n>::eigenvaluesComplexHermitian(mat,
                                                                        ret);
  return ret;
}

template <int n, typename T>
bool handleLapackComplexHermitian(
    Dune::FieldMatrix<std::complex<T>, n, n> const &mat,
    Dune::FieldVector<T, n> const &sol, T scale = 1) {
  auto eps = std::numeric_limits<T>::epsilon();
  auto lapack = getLapackComplexHermitian(mat);
  auto diffLapack = diff(lapack, sol);
  std::cout << "diff(lapack, exact) = " << diffLapack << std::endl;
  return diffLapack < 10 * eps * scale;
}

template <int n, typename T>
Dune::FieldVector<std::complex<T>, n>
getLapackComplexGeneric(Dune::FieldMatrix<std::complex<T>, n, n> const &mat) {
  Dune::FieldVector<std::complex<T>, n> ret;
  // FIXME: Currently not exposed
  Dune::Impl::LapackEigenvalueHelper<T, n>::eigenvaluesComplexGeneric(mat, ret);
  return ret;
}

template <int n, typename T>
bool handleLapackComplexGeneric(
    Dune::FieldMatrix<std::complex<T>, n, n> const &mat,
    Dune::FieldVector<std::complex<T>, n> const &sol, T scale = 1) {
  auto eps = std::numeric_limits<T>::epsilon();
  auto lapack = getLapackComplexGeneric(mat);
  auto diffLapack = diff(lapack, sol);
  std::cout << "diff(lapack, exact) = " << diffLapack << std::endl;
  return diffLapack < 10 * eps * scale;
}

template <typename T> bool test0() {
  std::cout << "\n2x2: mat #0 (symmetric):" << std::endl;
  Dune::FieldMatrix<T, 2, 2> mat = {{1, 2}, {2, 3}};
  Dune::FieldVector<T, 2> sol;
  {
    /*
      computed via maxima:

      A: matrix([1,2],[2,3]);
      eigenvalues(A)
    */
    sol = {2 - sqrt(T(5)), 2 + sqrt(T(5))};
  }
  bool ret = true;
  {
    ret &= handleNative(mat, sol);
    ret &= handleLapackRealSymmetric(mat, sol);
  }
  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  {
    ret &= handleNative(mat, sol, T(1e-5));
    ret &= handleLapackRealSymmetric(mat, sol, T(1e-5));
  }
  return ret;
}

template <typename T> bool test1() {
  std::cout << "\n3x3: mat #1 (symmetric):" << std::endl;
  Dune::FieldMatrix<T, 3, 3> mat = {{1, 2, 3}, {2, 4, 5}, {3, 5, 6}};
  Dune::FieldVector<T, 3> sol;
  {
    /*
      computed via maxima:

      A: matrix([1,2,3],[2,4,5],[3,5,6]);
      eigenvalues(A)
    */
    using std::sqrt;
    using std::pow;
    std::complex<T> i91 = {0, 91};
    std::complex<T> isqrt3 = {0, sqrt(T(3))};
    std::complex<T> tmp1 =
        pow(i91 / (2 * sqrt(T(27))) + 3031 / T(54), 1 / T(3));
    std::complex<T> tmp2 = 133 / T(9) / tmp1;
    T tmp3 = 11 / T(3);
    sol = {std::real(tmp2 * (isqrt3 - T(1)) / T(2) +
                     tmp1 * (-isqrt3 - T(1)) / T(2) + tmp3),
           std::real(tmp2 * (-isqrt3 - T(1)) / T(2) +
                     tmp1 * (isqrt3 - T(1)) / T(2) + tmp3),
           std::real(tmp2 + tmp1 + tmp3)};
    std::sort(sol.begin(), sol.end());
  }
  bool ret = true;
  {
    ret &= handleNative(mat, sol, T(10));
    ret &= handleLapackRealSymmetric(mat, sol, T(10));
  }
  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  {
    ret &= handleNative(mat, sol, T(10e-5));
    ret &= handleLapackRealSymmetric(mat, sol, T(10e-5));
  }
  return ret;
}

template <typename T> bool test2() {
  std::cout << "\n3x3: mat #2 (symmetric):" << std::endl;
  Dune::FieldMatrix<T, 3, 3> mat = {{4, 0, 0}, {0, 5, 0}, {0, 0, 6}};
  Dune::FieldVector<T, 3> sol = {4, 5, 6};
  bool ret = true;
  {
    ret &= handleNative(mat, sol);
    ret &= handleLapackRealSymmetric(mat, sol);
  }
  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  {
    ret &= handleNative(mat, sol, T(1e-5));
    ret &= handleLapackRealSymmetric(mat, sol, T(1e-5));
  }
  return ret;
}

template <typename T> bool test2b() {
  std::cout << "\n3x3: mat #2b (symmetric):" << std::endl;
  Dune::FieldMatrix<T, 3, 3> mat = {{6, 0, 0}, {0, 6, 0}, {0, 0, 6}};
  Dune::FieldVector<T, 3> sol = {6, 6, 6};
  bool ret = true;
  {
    ret &= handleNative(mat, sol);
    ret &= handleLapackRealSymmetric(mat, sol);
  }
  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  {
    ret &= handleNative(mat, sol, T(1e-5));
    ret &= handleLapackRealSymmetric(mat, sol, T(1e-5));
  }
  return ret;
}

template <typename T> bool test3() {
  std::cout << "\n3x3: mat #3 (symmetric):" << std::endl;
  Dune::FieldMatrix<T, 3, 3> mat = {{1.6165, 0.5444, 0.5444},
                                    {0.5444, 1.6165, 0.5444},
                                    {0.5444, 0.5444, 1.7815}};
  Dune::FieldVector<T, 3> sol;
  {
    /*
      computed via maxima:

      A: matrix([16165,5444,5444],
                [5444,16165,5444],
                [5444,5444,17815]) / 10000;
      eigenvalues(A);
    */
    using std::sqrt;
    sol = {(19712 - sqrt(T(62872881))) / 10000,
           (19712 + sqrt(T(62872881))) / 10000, T(10721) / 10000};
    std::sort(sol.begin(), sol.end());
  }
  bool ret = true;
  {
    ret &= handleNative(mat, sol);
    ret &= handleLapackRealSymmetric(mat, sol);
  }
  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  {
    ret &= handleNative(mat, sol, T(1e-5));
    ret &= handleLapackRealSymmetric(mat, sol, T(1e-5));
  }
  return ret;
}

template <typename T> bool test3b() {
  std::cout << "\n3x3: mat #3b (symmetric):" << std::endl;
  Dune::FieldMatrix<T, 3, 3> mat = {{1, 1, 2}, {1, 1, 2}, {2, 2, 2}};
  Dune::FieldVector<T, 3> sol;
  {
    /*
      computed via maxima:

      A: matrix([1,1,2],
                [1,1,2],
                [2,2,2]);
      eigenvalues(A);
    */
    using std::sqrt;
    sol = {2 - sqrt(T(8)), 2 + sqrt(T(8)), 0};
    std::sort(sol.begin(), sol.end());
  }
  bool ret = true;
  {
    ret &= handleNative(mat, sol, T(5));
    ret &= handleLapackRealSymmetric(mat, sol, T(5));
  }
  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  {
    ret &= handleNative(mat, sol, T(5e-5));
    ret &= handleLapackRealSymmetric(mat, sol, T(5e-5));
  }
  return ret;
}

template <typename T> bool test4() {
  std::cout << "\n3x3: mat #4 (symmetric):" << std::endl;
  Dune::FieldMatrix<T, 3, 3> mat = {{4, 1, 0}, {1, 4, 1}, {0, 1, 4}};
  Dune::FieldVector<T, 3> sol;
  {
    /*
      computed via maxima:

      A: matrix([4,1,0],
                [1,4,1],
                [0,1,4]);
      eigenvalues(A);
    */
    using std::sqrt;
    sol = {4 - sqrt(T(2)), 4 + sqrt(T(2)), 4};
    std::sort(sol.begin(), sol.end());
  }
  bool ret = true;
  {
    ret &= handleNative(mat, sol);
    ret &= handleLapackRealSymmetric(mat, sol);
  }
  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  {
    ret &= handleNative(mat, sol, T(1e-5));
    ret &= handleLapackRealSymmetric(mat, sol, T(1e-5));
  }
  return ret;
}

template <typename T> bool test4b() {
  std::cout << "\n5x5: mat #4b (symmetric):" << std::endl;
  Dune::FieldMatrix<T, 5, 5> mat = {{4, 1, 0, 0, 0},
                                    {1, 4, 1, 0, 0},
                                    {0, 1, 4, 1, 0},
                                    {0, 0, 1, 4, 1},
                                    {0, 0, 0, 1, 4}};
  Dune::FieldVector<T, 5> sol;
  {
    /*
      computed via maxima:

      A: matrix([4,1,0,0,0],
                [1,4,1,0,0],
                [0,1,4,1,0],
                [0,0,1,4,1],
                [0,0,0,1,4]);
      eigenvalues(A);
    */
    using std::sqrt;
    sol = {3, 4, (4 - sqrt(T(3))), (4 + sqrt(T(3))), 5};
    std::sort(sol.begin(), sol.end());
  }
  bool ret = true;
  { ret &= handleLapackRealSymmetric(mat, sol); }
  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  { ret &= handleLapackRealSymmetric(mat, sol, T(1e-5)); }
  return ret;
}

template <typename T> bool test4c() {
  std::cout << "\n9x9: mat #4c (symmetric):" << std::endl;
  Dune::FieldMatrix<T, 9, 9> mat = {
      {4, 1, 0, 0, 0, 0, 0, 0, 0}, {1, 4, 1, 0, 0, 0, 0, 0, 0},
      {0, 1, 4, 1, 0, 0, 0, 0, 0}, {0, 0, 1, 4, 1, 0, 0, 0, 0},
      {0, 0, 0, 1, 4, 1, 0, 0, 0}, {0, 0, 0, 0, 1, 4, 1, 0, 0},
      {0, 0, 0, 0, 0, 1, 4, 1, 0}, {0, 0, 0, 0, 0, 0, 1, 4, 1},
      {0, 0, 0, 0, 0, 0, 0, 1, 4}};
  Dune::FieldVector<T, 9> sol;
  {
    /*
      computed via maxima:

      A: matrix([4,1,0,0,0,0,0,0,0],
                [1,4,1,0,0,0,0,0,0],
                [0,1,4,1,0,0,0,0,0],
                [0,0,1,4,1,0,0,0,0],
                [0,0,0,1,4,1,0,0,0],
                [0,0,0,0,1,4,1,0,0],
                [0,0,0,0,0,1,4,1,0],
                [0,0,0,0,0,0,1,4,1],
                [0,0,0,0,0,0,0,1,4]);
      eigenvalues(A);
    */
    using std::sqrt;
    sol = {(9 - sqrt(T(5))) / 2,
           (9 + sqrt(T(5))) / 2,
           4,
           (7 - sqrt(T(5))) / 2,
           (7 + sqrt(T(5))) / 2,
           4 - sqrt(5 - sqrt(T(5))) / sqrt(T(2)),
           4 + sqrt(5 - sqrt(T(5))) / sqrt(T(2)),
           4 - sqrt(5 + sqrt(T(5))) / sqrt(T(2)),
           4 + sqrt(5 + sqrt(T(5))) / sqrt(T(2))};
    std::sort(sol.begin(), sol.end());
  }
  bool ret = true;
  { ret &= handleLapackRealSymmetric(mat, sol, T(5)); }
  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  { ret &= handleLapackRealSymmetric(mat, sol, T(5e-5)); }
  return ret;
}

template <typename T> bool test5() {
  std::cout << "\n2x2: mat #5 (real; real eigenvalues):" << std::endl;
  Dune::FieldMatrix<T, 2, 2> mat = {{-19., 22.}, {11., -8.}};
  Dune::FieldVector<std::complex<T>, 2> sol;
  {
    /*
      computed via maxima:

      A: matrix([-19,22],
                [11,-8]);
      eigenvalues(A);
    */
    sol = {{-30, 0}, {3, 0}};
  }
  bool ret = true;
  { ret &= handleLapackRealGeneric(mat, sol, T(30)); }
  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  { ret &= handleLapackRealGeneric(mat, sol, T(30e-5)); }
  return ret;
}

template <typename T> bool test6() {
  std::cout << "\n2x2: mat #6 (real; complex eigenvalues):" << std::endl;
  Dune::FieldMatrix<T, 2, 2> mat = {{1, -2}, {2, 1}};
  Dune::FieldVector<std::complex<T>, 2> sol;
  {
    /*
      computed via maxima:

      A: matrix([1,-2],
                [2,1]);
      eigenvalues(A);
    */
    sol = {{1, 2}, {1, -2}}; // FIXME: order?
  }
  bool ret = true;
  { ret &= handleLapackRealGeneric(mat, sol); }
  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  { ret &= handleLapackRealGeneric(mat, sol, T(1e-5)); }
  return ret;
}

template <typename T> bool test7() {
  std::cout << "\n3x3: mat #7 (real; complex eigenvalues):" << std::endl;
  Dune::FieldMatrix<T, 3, 3> mat = {{1, -2, 1}, {2, 1, 1}, {1, 1, 1}};
  Dune::FieldVector<std::complex<T>, 3> sol;
  {
    /*
      computed via maxima:

      A: matrix([1,-2,1],
                [2,1,1],
                [1,1,1]);
      eigenvalues(A);
    */
    using std::sqrt;
    sol = {{1, sqrt(T(2))}, {1, -sqrt(T(2))}, {1, 0}}; // FIXME: order?
  }
  bool ret = true;
  { ret &= handleLapackRealGeneric(mat, sol); }
  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  { ret &= handleLapackRealGeneric(mat, sol, T(1e-5)); }
  std::cout << "RET = " << ret << std::endl; // FIXME FIXME FIXME FIXME
  return ret;
}

template <typename T> bool test8() {
  std::cout << "\n2x2: mat #8 (complex; complex eigenvalues):" << std::endl;
  Dune::FieldMatrix<std::complex<T>, 2, 2> mat = {{{1, 0}, {0, 2}},
                                                  {{0, 2}, {3, 0}}};
  Dune::FieldVector<std::complex<T>, 2> sol;
  {
    /*
      computed via maxima:

      A: matrix([1,2*%i],
                [2*%i, 3]);
      eigenvalues(A);
    */
    using std::sqrt;
    sol = {{{2, sqrt(T(3))}, {2, -sqrt(T(3))}}}; // FIXME: order?
  }
  bool ret = true;
  { ret &= handleLapackComplexGeneric(mat, sol); }
  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  { ret &= handleLapackComplexGeneric(mat, sol, T(1e-5)); }
  return ret;
}

template <typename T> bool test9() {
  std::cout << "\n2x2: mat #9 (hermitian):" << std::endl;
  Dune::FieldMatrix<std::complex<T>, 2, 2> mat = {{{1, 0}, {0, 2}},
                                                  {{0, -2}, {3, 0}}};
  Dune::FieldVector<T, 2> sol;
  {
    /*
      computed via maxima:

      A: matrix([1,2*%i],
                [-2*%i, 3]);
      eigenvalues(A);
    */
    using std::sqrt;
    sol = {2 - sqrt(T(5)), 2 + sqrt(T(5))};
  }
  bool ret = true;
  { ret &= handleLapackComplexHermitian(mat, sol); }
  std::cout << "Rescaling matrix: mat -> 1e-5*mat" << std::endl;
  mat *= 1e-5;
  sol *= 1e-5;
  { ret &= handleLapackComplexHermitian(mat, sol, T(1e-5)); }
  return ret;
}

template <typename T> bool run() {
  bool ret = true;
  std::cout << std::showpos << std::scientific
            << std::setprecision(std::numeric_limits<T>::digits10);
  ret &= test0<T>();
  ret &= test1<T>();
  ret &= test2<T>();
  ret &= test2b<T>();
  ret &= test3<T>();
  ret &= test3b<T>();
  ret &= test4<T>();
  ret &= test4b<T>();
  ret &= test4c<T>();
  ret &= test5<T>();
  ret &= test6<T>();
  ret &= test7<T>();
  ret &= test8<T>();
  ret &= test9<T>();

  return ret;
}

int main() {
  bool ret = true;

  std::cout << "float:" << std::endl;
  ret &= run<float>();
  std::cout << "\n\ndouble:" << std::endl;
  ret &= run<double>();

#if 0 // HAVE_GMP
  std::cout << std::setprecision(25) << "\n\nGMPField<80u>:";
  // ...
#endif

  return ret ? 0 : 1;
}
