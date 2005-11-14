// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <dune/istl/matrixutils.hh>
#include <dune/istl/bcrsmatrix.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/stdstreams.hh>
#include "laplacian.hh"

int main(int argc, char** argv)
{
  Dune::FieldMatrix<double,4,7> fmatrix;

  int ret=0;

  if(4*7!=countNonZeros(fmatrix)) {
    Dune::derr<<"Counting nonzeros of fieldMatrix failed!"<<std::endl;
    ret++;
  }

  const int N=4;

  typedef Dune::BCRSMatrix<Dune::FieldMatrix<double,1,1> > BMatrix;

  BMatrix laplace(N*N,N*N, N*N*5, BMatrix::row_wise);
  setupLaplacian2d<N>(laplace);

  if(N*N*5-4*2-(N-2)*4!=countNonZeros(laplace)) {
    ++ret;
    Dune::derr<<"Counting nonzeros of BCRSMatrix failed!"<<std::endl;
  }

  Dune::BCRSMatrix<Dune::FieldMatrix<double,4,7> > blaplace(N*N,N*N, N*N*5, Dune::BCRSMatrix<Dune::FieldMatrix<double,4,7> >::row_wise);
  setupLaplacian<N>(blaplace);

  if((N*N*5-4*2-(N-2)*4)*4*7!=countNonZeros(blaplace)) {
    ++ret;
    Dune::derr<<"Counting nonzeros of block BCRSMatrix failed!"<<std::endl;
  }

}
