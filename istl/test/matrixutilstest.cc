// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <dune/istl/matrixutils.hh>
#include <dune/istl/bcrsmatrix.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/stdstreams.hh>

template<int N, class B>
void setupSparsityPattern(Dune::BCRSMatrix<B>& A)
{
  for (typename Dune::BCRSMatrix<B>::CreateIterator i = A.createbegin(); i != A.createend(); ++i) {
    int x = i.index()%N; // x coordinate in the 2d field
    int y = i.index()/N;  // y coordinate in the 2d field

    if(y>0)
      // insert lower neighbour
      i.insert(i.index()-N);
    if(x>0)
      // insert left neighbour
      i.insert(i.index()-1);

    // insert diagonal value
    i.insert(i.index());

    if(x<N-1)
      //insert right neighbour
      i.insert(i.index()+1);
    if(y<N-1)
      // insert upper neighbour
      i.insert(i.index()+N);
  }
}

template<int N, class B>
void setupLaplacian(Dune::BCRSMatrix<B>& A)
{
  setupSparsityPattern<N>(A);

  B diagonal = 0, bone=0, beps=0;
  for(typename B::RowIterator b = diagonal.begin(); b !=  diagonal.end(); ++b)
    b->operator[](b.index())=4;


  for(typename B::RowIterator b=bone.begin(); b !=  bone.end(); ++b)
    b->operator[](b.index())=-1.0;


  for (typename Dune::BCRSMatrix<B>::RowIterator i = A.begin(); i != A.end(); ++i) {
    int x = i.index()%N; // x coordinate in the 2d field
    int y = i.index()/N;  // y coordinate in the 2d field

    i->operator[](i.index())=diagonal;

    if(y>0)
      i->operator[](i.index()-N)=bone;

    if(y<N-1)
      i->operator[](i.index()+N)=bone;

    if(x>0)
      i->operator[](i.index()-1)=bone;

    if(x < N-1)
      i->operator[](i.index()+1)=bone;
  }
}

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
  setupLaplacian<N>(laplace);

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
