// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/dynmatrixev.hh>
#include <dune/common/fmatrixev.hh>

#include <algorithm>
#include <complex>

using namespace Dune;

#if HAVE_LAPACK
/** \brief Test the eigenvalue code with the Rosser test matrix

   This matrix was a challenge for many matrix eigenvalue
   algorithms. But the Francis QR algorithm, as perfected by
   Wilkinson and implemented in EISPACK, has no trouble with it. The
   matrix is 8-by-8 with integer elements. It has:

 * A double eigenvalue
 * Three nearly equal eigenvalues
 * Dominant eigenvalues of opposite sign
 * A zero eigenvalue
 * A small, nonzero eigenvalue

*/
template<typename ft>
void testRosserMatrix()
{
  DynamicMatrix<ft> A = {
    { 611, 196, -192, 407, -8, -52, -49, 29 },
    { 196, 899, 113, -192, -71, -43, -8, -44 },
    { -192, 113, 899, 196, 61, 49, 8, 52 },
    { 407, -192, 196, 611, 8, 44, 59, -23 },
    { -8, -71, 61, 8, 411, -599, 208, 208 },
    { -52, -43, 49, 44, -599, 411, 208, 208 },
    { -49, -8, 8, 59, 208, 208, 99, -911 },
    { 29, -44, 52, -23, 208, 208, -911, 99}
  };

  // compute eigenvalues
  DynamicVector<std::complex<ft> > eigenComplex;
  DynamicMatrixHelp::eigenValuesNonSym(A, eigenComplex);

  // test results
  /*
     reference solution computed with octave 3.2

     > format long e
     > eig(rosser())

   */
  std::vector<ft> reference = {
          -1.02004901843000e+03,
          -4.14362871168386e-14,
           9.80486407214362e-02,
           1.00000000000000e+03,
           1.00000000000000e+03,
           1.01990195135928e+03,
           1.02000000000000e+03,
           1.02004901843000e+03
  };

  std::vector<ft> eigenRealParts(8);
  for (int i=0; i<8; i++)
    eigenRealParts[i] = std::real(eigenComplex[i]);

  std::sort(eigenRealParts.begin(), eigenRealParts.end());

  for (int i=0; i<8; i++)
  {
    if (std::fabs(std::imag(eigenComplex[i])) > 1e-10)
      DUNE_THROW(MathError, "Symmetric matrix has complex eigenvalue");

    if( std::fabs(reference[i] - eigenRealParts[i]) > 1e-10 )
      DUNE_THROW(MathError,"error computing eigenvalues");
  }

  std::cout << "Eigenvalues of Rosser matrix: " << eigenComplex << std::endl;
}

template <class field_type>
void testSymmetricFieldMatrix()
{
  int numberOfTestMatrices = 10;

  for (int i=0; i<numberOfTestMatrices; i++)
  {
    // Construct pseudo-random symmetric test matrix
    FieldMatrix<field_type,3,3> testMatrix;
    for (int j=0; j<3; j++)
      for (int k=j; k<3; k++)
        testMatrix[j][k] = testMatrix[k][j] = ((int)(M_PI*j*k*i))%100;

    FieldVector<field_type,3> eigenValues;
    FMatrixHelp::eigenValues(testMatrix, eigenValues);

    // Make sure the compute numbers really are the eigenvalues
    for (int j=0; j<3; j++)
    {
      FieldMatrix<field_type,3,3> copy = testMatrix;
      for (int k=0; k<3; k++)
        copy[k][k] -= eigenValues[j];

      if (std::fabs(copy.determinant()) > 1e-8)
        DUNE_THROW(MathError, "Value computed by FMatrixHelp::eigenValues is not an eigenvalue");
    }

    // Make sure the eigenvalues are in ascending order
    for (int j=0; j<3-1; j++)
      if (eigenValues[j] > eigenValues[j+1] + 1e-10)
        DUNE_THROW(MathError, "Values computed by FMatrixHelp::eigenValues are not in ascending order");
  }
}
#endif // HAVE_LAPACK

int main() try
{
#if HAVE_LAPACK
  testRosserMatrix<double>();

  testSymmetricFieldMatrix<double>();

  return 0;
#else
  std::cout << "WARNING: eigenvaluetest needs LAPACK, test disabled" << std::endl;
  return 77;
#endif // HAVE_LAPACK
} catch (Exception exception)
{
  std::cerr << exception << std::endl;
  return 1;
}
