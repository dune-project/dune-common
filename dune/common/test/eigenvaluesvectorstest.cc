// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/fmatrixev.hh>

using namespace Dune;

#if HAVE_LAPACK
template <class field_type, int dim>
void testSymmetricFieldMatrixValues()
{
  int numberOfTestMatrices = 10;
  std::cout << "Testing eigenvalues of " << numberOfTestMatrices << " random " << dim << "D symmetric matrices ..." << std::endl;
  for (int i=0; i<numberOfTestMatrices; i++)
  {
    // Construct pseudo-random symmetric test matrix
    FieldMatrix<field_type,dim,dim> testMatrix;
    for (int j=0; j<dim; j++)
      for (int k=j; k<dim; k++)
        testMatrix[j][k] = testMatrix[k][j] = ((int)(M_PI*j*k*i))%100 - 1;

    FieldVector<field_type,dim> eigenValues;
    FMatrixHelp::eigenValues(testMatrix, eigenValues);

    // Make sure the compute numbers really are the eigenvalues
    for (int j=0; j<dim; j++)
    {
      FieldMatrix<field_type,dim,dim> copy = testMatrix;
      for (int k=0; k<dim; k++)
        copy[k][k] -= eigenValues[j];

      if (std::fabs(copy.determinant()) > 1e-8)
        DUNE_THROW(MathError, "Value computed by FMatrixHelp::eigenValues is not an eigenvalue");
    }

    // Make sure the eigenvalues are in ascending order
    for (int j=0; j<dim-1; j++)
      if (eigenValues[j] > eigenValues[j+1] + 1e-10)
        DUNE_THROW(MathError, "Values computed by FMatrixHelp::eigenValues are not in ascending order");
  }
}

template <class field_type, int dim>
void testSymmetricFieldMatrixValuesVectors()
{
  int numberOfTestMatrices = 10;

  std::cout << "Testing eigenvalues and vectors of " << numberOfTestMatrices << " random " << dim << "D symmetric matrices ..." << std::endl;
  for (int i=0; i<numberOfTestMatrices; i++)
  {
    // Construct pseudo-random symmetric test matrix
    FieldMatrix<field_type,dim,dim> testMatrix;
    for (int j=0; j<dim; j++)
      for (int k=j; k<dim; k++)
        testMatrix[j][k] = testMatrix[k][j] = ((int)(M_PI*j*k*i))%100 - 1;

    FieldVector<field_type,dim> eigenValues;
    FieldMatrix<field_type,dim,dim> eigenVectors;
    FMatrixHelp::eigenValuesVectors<dim,field_type>(testMatrix, eigenValues, eigenVectors);

    FieldVector<field_type,dim> test;
    for (int i=0; i<dim; ++i){
      auto copy = eigenVectors[i];
      testMatrix.mv(copy,test);
      copy *= eigenValues[i];
      for (int j=0; j<dim; ++j)
        if (fabs(copy[j]-test[j]) > 1e-10)
          DUNE_THROW(MathError, "Value and Vector computed by FMatrixHelp::eigenValuesVectors do not fit (A v!=lambda v)");
     }
    // Make sure the compute numbers really are the eigenvalues
  }
}
#endif // HAVE_LAPACK

int main() try
{
#if HAVE_LAPACK
  testSymmetricFieldMatrixValues<double,2>();
  testSymmetricFieldMatrixValues<double,3>();

  testSymmetricFieldMatrixValuesVectors<double,2>();
  testSymmetricFieldMatrixValuesVectors<double,3>();
  std::cout << "no errors occured" << std::endl;
#else
  std::cout << "WARNING: eigenvaluetest needs LAPACK, test disabled" << std::endl;
#endif // HAVE_LAPACK

  return 0;
} catch (Exception exception)
{
  std::cerr << exception << std::endl;
  return 1;
}
