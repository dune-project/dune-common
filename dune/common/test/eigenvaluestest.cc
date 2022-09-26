// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/dynmatrixev.hh>
#include <dune/common/fmatrixev.hh>

#include <algorithm>
#include <limits>
#include <list>
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
      DUNE_THROW(MathError,"error computing eigenvalues of Rosser-matrix");
  }

  std::cout << "Eigenvalues of Rosser matrix: " << eigenComplex << std::endl;
}
#endif // HAVE_LAPACK

template <class field_type, int dim>
void testSymmetricFieldMatrix()
{
  int numberOfTestMatrices = 10;

  for (int i=0; i<numberOfTestMatrices; i++)
  {
    // Construct pseudo-random symmetric test matrix
    FieldMatrix<field_type,dim,dim> testMatrix;
    for (int j=0; j<dim; j++)
      for (int k=j; k<dim; k++)
        testMatrix[j][k] = testMatrix[k][j] = ((int)(M_PI*j*k*i))%100 - 1;

    FieldVector<field_type,dim> eigenValues;
    FieldMatrix<field_type,dim,dim> eigenVectors;
    FMatrixHelp::eigenValuesVectors(testMatrix, eigenValues, eigenVectors);

    // Make sure the compute numbers really are the eigenvalues
    /*for (int j=0; j<dim; j++)
    {
      FieldMatrix<field_type,dim,dim> copy = testMatrix;
      for (int k=0; k<dim; k++)
        copy[k][k] -= eigenValues[j];

      if (std::fabs(copy.determinant()) > 1e-8)
        DUNE_THROW(MathError, "Value computed by FMatrixHelp::eigenValues is not an eigenvalue, Determinant: "+std::to_string(std::fabs(copy.determinant())));
    }*/

    // Make sure eigenvalues and eigenvectors are not NaN (the subsequent tests do not find this!)
    for (int j=0; j<dim; j++)
    {
      using std::isnan;
      if (isnan(eigenValues[j]))
        DUNE_THROW(MathError, j << "-th eigenvalue is NaN!");

      for (std::size_t k=0; k<dim; k++)
        if (isnan(eigenVectors[j][k]))
          DUNE_THROW(MathError, j << "-th eigenvector contains NaN!");
    }

    // Make sure the eigenvalues are in ascending order
    for (int j=0; j<dim-1; j++)
      if (eigenValues[j] > eigenValues[j+1] + 1e-10)
        DUNE_THROW(MathError, "Values computed by FMatrixHelp::eigenValues are not in ascending order");

    // Make sure the vectors really are eigenvectors for the computed eigenvalues
    for (int j=0; j<dim; j++)
    {
      FieldVector<field_type, dim> Av;
      testMatrix.mv(eigenVectors[j], Av);
      if((Av - eigenValues[j]*eigenVectors[j]).two_norm() > dim*std::sqrt(std::numeric_limits<field_type>::epsilon()))
        DUNE_THROW(MathError, "Vector computed by FMatrixHelp::eigenValuesVectors is not an eigenvector");
    }

    // Make sure the eigenvectors have unit length
    for(auto& ev : eigenVectors) {
      constexpr double tol = std::max<double>(std::numeric_limits<field_type>::epsilon(),
                                              std::numeric_limits<double>::epsilon());
      if(std::abs(ev.two_norm())-1 > dim*tol)
        DUNE_THROW(MathError, "Vector computed by FMatrixHelp::eigenValuesVectors does not have unit length");
    }

  }
}

template<typename field_type, int dim>
void compareEigenvectorSets(FieldMatrix<field_type,dim,dim> evec,
                              FieldVector<field_type,dim> refEval,
                              FieldMatrix<field_type,dim,dim> refEvec)
{
  field_type th = dim*std::sqrt(std::numeric_limits<field_type>::epsilon());

  std::size_t i=0;
  std::size_t shift;
  std::list<FieldVector<field_type,dim>> refEvecList;
  field_type currentEval;

  while(i<dim) {
    shift=i;
    currentEval = refEval[i];
    while(i<dim && refEval[i]==currentEval) {
      refEvecList.push_back(refEvec[i]);
      ++i;
    }
    for(std::size_t j=0; j<refEvecList.size(); ++j) {
      bool found = false;
      auto it = refEvecList.begin();
      while(!found && it != refEvecList.end()) {
        if((evec[shift+j]-*it).two_norm() < th || (-1.0*evec[shift+j]-*it).two_norm() < th)
          found = true;
        else
          ++it;
      }
      if(!found)
        DUNE_THROW(MathError, "Eigenvector [" << evec[j] << "] for eigenvalue "
                   << currentEval << " not found within the reference solutions [" << refEvec << "]");
    }
    refEvecList.clear();
  }
}

template<typename field_type, int dim>
void checkMatrixWithReference(FieldMatrix<field_type, dim, dim> matrix,
                              FieldMatrix<field_type, dim, dim> refEvec,
                              FieldVector<field_type, dim> refEval)
{
  //normalize reference
  for(auto& ev : refEvec)
    ev /= ev.two_norm();

  field_type th = dim*std::sqrt(std::numeric_limits<field_type>::epsilon());

  FieldMatrix<field_type,dim,dim> eigenvectors;
  FieldVector<field_type,dim> eigenvalues;

  FMatrixHelp::eigenValuesVectors(matrix, eigenvalues, eigenvectors);

  if((eigenvalues-refEval).two_norm() > th)
    DUNE_THROW(MathError, "Eigenvalues [" << eigenvalues << "] computed by FMatrixHelp::eigenValuesVectors do not match the reference solution [" << refEval << "]");
  try {
    compareEigenvectorSets(eigenvectors, refEval, refEvec);
  }
  catch(Dune::MathError& e) {
    std::cerr << "Computations by `FMatrixHelp::eigenValuesVectors`: " << e.what() << std::endl;
  }
}

template<typename field_type, int dim>
void checkMatrixWithLAPACK(FieldMatrix<field_type, dim, dim> matrix)
{
  field_type th = dim*std::sqrt(std::numeric_limits<field_type>::epsilon());

  FieldMatrix<field_type,dim,dim> eigenvectors, refEvec;
  FieldVector<field_type,dim> eigenvalues, refEval;

  FMatrixHelp::eigenValuesVectors(matrix, eigenvalues, eigenvectors);
  FMatrixHelp::eigenValuesVectorsLapack(matrix, refEval, refEvec);

  if((eigenvalues-refEval).two_norm() > th)
    DUNE_THROW(MathError, "Eigenvalues [" << eigenvalues << "] computed by FMatrixHelp::eigenValuesVectorsLapack do not match the reference solution [" << refEval << "]");
  try {
    compareEigenvectorSets(eigenvectors, refEval, refEvec);
  }
  catch(Dune::MathError& e) {
    std::cerr << "Computations by `FMatrixHelp::eigenValuesVectorsLapack`: " << e.what() << std::endl;
  }
}

template<class FT>
void checkMultiplicity()
{
  //--2d--
  //repeated eigenvalue (x2)
  checkMatrixWithReference<FT,2>({{1, 0},{0, 1}}, {{1,0}, {0,1}}, {1, 1});

  //eigenvalues with same magnitude (x2)
  checkMatrixWithReference<FT,2>({{0, 1}, {1, 0}}, {{1,-1}, {1,1}}, {-1, 1});

  // singular matrix
  checkMatrixWithReference<FT,2>({{1, 0},{0, 0}}, {{0,1}, {1,0}}, {0, 1});

  // another singular matrix (triggers a different code path)
  checkMatrixWithReference<FT,2>({{0, 0},{0, 1}}, {{1,0}, {0,1}}, {0, 1});

  // Seemingly simple diagonal matrix -- triggers unstable detection of zero columns
  checkMatrixWithReference<FT,2>({{1.01, 0},{0, 1}}, {{0,1}, {1,0}}, {1, 1.01});

  // check 2x2 zero matrix
  checkMatrixWithReference<FT,2>({{ 0, 0},
                                  { 0, 0}},
    {{1,0}, {0,1}},
    {0, 0});

  //--3d--
  //repeated eigenvalue (x3)
  checkMatrixWithReference<FT,3>({{  1,   0,   0},
                                      {  0,   1,   0},
                                      {  0,   0,   1}},
    {{1,0,0}, {0,1,0}, {0,0,1}},
    {1, 1, 1});

  //eigenvalues with same magnitude (x2)
  checkMatrixWithReference<FT,3>({{  0,   1,   0},
                                      {  1,   0,   0},
                                      {  0,   0,   5}},
    {{-1,1,0}, {1,1,0}, {0,0,1}},
    {-1, 1, 5});

  //repeated eigenvalue (x2)
  checkMatrixWithReference<FT,3>({{  3,  -2,   0},
                                      { -2,   3,   0},
                                      {  0,   0,   5}},
    {{1,1,0}, {0,0,1}, {1,-1,0}},
    {1, 5, 5});

  // singular non-diagonal matrix
  checkMatrixWithReference<FT,3>({{  0,   0,   0},
                                  {  0,   1,   1},
                                  {  0,   1,   1}},
    {{1,0,0}, {0,FT(-1.0/std::sqrt(2.0)),FT(1.0/std::sqrt(2.0))}, {0,FT(1.0/std::sqrt(2.0)),FT(1.0/std::sqrt(2.0))}},
    {0, 0, 2});

  // singular diagonal matrix (that's a different code path again)
  checkMatrixWithReference<FT,3>({{  0,   0,   0},
                                  {  0,   1,   0},
                                  {  0,   0,   0}},
    {{1,0,0}, {0,0,1}, {0,1,0}},
    {0, 0, 1});

  // diagonal matrix whose largest eigenvalue is not 1
  // this tests the matrix scaling employed by the eigenvector code.
  checkMatrixWithReference<FT,3>({{  3,   0,   0},
                                  {  0,   2,   0},
                                  {  0,   0,   4}},
    {{0,1,0}, {1,0,0}, {0,0,1}},
    {2, 3, 4});

  // check 3x3 zero matrix
  checkMatrixWithReference<FT,3>({{  0,   0,   0},
                                  {  0,   0,   0},
                                  {  0,   0,   0}},
    {{1,0,0}, {0,1,0}, {0,0,1}},
    {0, 0, 0});

  //repeat tests with LAPACK (if found)
#if HAVE_LAPACK
  checkMatrixWithLAPACK<FT,2>({{1, 0}, {0, 1}});
  checkMatrixWithLAPACK<FT,2>({{0, 1}, {1, 0}});
  checkMatrixWithLAPACK<FT,3>({{1,0,0}, {0,1,0}, {0,0,1}});
  checkMatrixWithLAPACK<FT,3>({{0,1,0}, {1,0,0}, {0,0,5}});
  checkMatrixWithLAPACK<FT,3>({{3,-2,0}, {-2,3,0}, {0,0,5}});
#endif

}

int main()
{
#if HAVE_LAPACK
  testRosserMatrix<double>();
  testRosserMatrix<float>();
  testRosserMatrix<long double>();
#else
  std::cout << "WARNING: eigenvaluetest needs LAPACK, test disabled" << std::endl;
#endif // HAVE_LAPACK

  //we basically just test LAPACK here, so maybe discard those tests
#if HAVE_LAPACK
  testSymmetricFieldMatrix<double,4>();
  testSymmetricFieldMatrix<double,200>();
  testSymmetricFieldMatrix<float,4>();
  testSymmetricFieldMatrix<float,200>();
  testSymmetricFieldMatrix<long double,4>();
  testSymmetricFieldMatrix<long double,200>();
#endif // HAVE_LAPACK

  testSymmetricFieldMatrix<double,2>();
  testSymmetricFieldMatrix<double,3>();
  testSymmetricFieldMatrix<float,2>();
  testSymmetricFieldMatrix<float,3>();
  testSymmetricFieldMatrix<long double,2>();
  testSymmetricFieldMatrix<long double,3>();

  checkMultiplicity<double>();
  checkMultiplicity<float>();
  checkMultiplicity<long double>();

  return 0;
}
