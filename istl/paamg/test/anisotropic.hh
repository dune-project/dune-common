// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef ANISOTROPIC_HH
#define  ANISOTROPIC_HH
#include <dune/istl/indexset.hh>
#include <dune/istl/communicator.hh>
#include <dune/istl/interface.hh>
#include <dune/istl/bcrsmatrix.hh>
#include <dune/common/fmatrix.hh>
#include <dune/istl/owneroverlapcopy.hh>

typedef Dune::OwnerOverlapCopyAttributeSet GridAttributes;
typedef GridAttributes::AttributeSet GridFlag;
typedef Dune::ParallelLocalIndex<GridFlag> LocalIndex;

template<class M, class G, class L, int n>
void setupPattern(int N, M& mat, Dune::ParallelIndexSet<G,L,n>& indices, int overlapStart, int overlapEnd,
                  int start, int end);

template<class M>
void fillValues(int N, M& mat, int overlapStart, int overlapEnd, int start, int end);


template<int BS, class G, class L, int n>
Dune::BCRSMatrix<Dune::FieldMatrix<double,BS,BS> > setupAnisotropic2d(int N, Dune::ParallelIndexSet<G,L,n>& indices, int *nout, double eps=1.0);


template<class M, class G, class L, int s>
void setupPattern(int N, M& mat, Dune::ParallelIndexSet<G,L,s>& indices, int overlapStart, int overlapEnd,
                  int start, int end)
{
  int n = overlapEnd - overlapStart;

  typename M::CreateIterator iter = mat.createbegin();
  indices.beginResize();

  for(int j=0; j < N; j++)
    for(int i=overlapStart; i < overlapEnd; i++, ++iter) {
      int global = j*N+i;
      GridFlag flag = GridAttributes::owner;
      bool isPublic = false;

      if((i<start && i > 0) || (i>= end && i < N-1))
        flag=GridAttributes::copy;

      if(i<start+1 || i>= end-1)
        isPublic = true;

      indices.add(global, LocalIndex(iter.index(), flag, isPublic));

      iter.insert(iter.index());

      // i direction
      if(i > overlapStart )
        // We have a left neighbour
        iter.insert(iter.index()-1);

      if(i < overlapEnd-1)
        // We have a rigt neighbour
        iter.insert(iter.index()+1);

      // j direction
      // Overlap is a dirichlet border, discard neighbours
      if(flag != GridAttributes::copy) {
        if(j>0)
          // lower neighbour
          iter.insert(iter.index()-n);
        if(j<N-1)
          // upper neighbour
          iter.insert(iter.index()+n);
      }
    }
  indices.endResize();
}

template<class M>
void fillValues(int N, M& mat, int overlapStart, int overlapEnd, int start, int end, double eps)
{
  typedef typename M::block_type Block;
  Block dval = 0, bone=0, bmone=0, beps=0;

  for(typename Block::RowIterator b = dval.begin(); b !=  dval.end(); ++b)
    b->operator[](b.index())=2.0+2.0*eps;

  for(typename Block::RowIterator b=bone.begin(); b !=  bone.end(); ++b)
    b->operator[](b.index())=1.0;

  for(typename Block::RowIterator b=bmone.begin(); b !=  bmone.end(); ++b)
    b->operator[](b.index())=-1.0;

  for(typename Block::RowIterator b=beps.begin(); b !=  beps.end(); ++b)
    b->operator[](b.index())=-eps;

  int n = overlapEnd-overlapStart;
  typedef typename Dune::BCRSMatrix<Block>::ColIterator ColIterator;
  typedef typename Dune::BCRSMatrix<Block>::RowIterator RowIterator;

  for (RowIterator i = mat.begin(); i != mat.end(); ++i) {
    // calculate coordinate
    int y = i.index() / n;
    int x = overlapStart + i.index() - y * n;

    ColIterator endi = (*i).end();

    if(x<start || x >= end || x==0 || x==N-1 || y==0 || y==N-1) {
      // overlap node is dirichlet border
      ColIterator j = (*i).begin();

      for(; j.index() < i.index(); ++j)
        *j=0;

      *j = bone;

      for(++j; j != endi; ++j)
        *j=0;
    }else{
      for(ColIterator j = (*i).begin(); j != endi; ++j)
        if(j.index() == i.index())
          *j=dval;
        else if(j.index()+1==i.index() || j.index()-1==i.index())
          *j=beps;
        else
          *j=bmone;
    }
  }
}

template<int BS, class G, class L, int s>
void setBoundary(Dune::BlockVector<Dune::FieldVector<double,BS> >& lhs,
                 Dune::BlockVector<Dune::FieldVector<double,BS> >& rhs,
                 const G& n, Dune::ParallelIndexSet<G,L,s>& indices)
{
  typedef typename Dune::ParallelIndexSet<G,L,s>::const_iterator Iter;
  for(Iter i=indices.begin(); i != indices.end(); ++i) {
    G x = i->global()/n;
    G y = i->global()%n;

    if(x==0 || y ==0 || x==n-1 || y==n-1) {
      lhs[i->local()]=rhs[i->local()];
    }
  }
}

template<int BS, class G, class L, int s>
Dune::BCRSMatrix<Dune::FieldMatrix<double,BS,BS> > setupAnisotropic2d(int N, Dune::ParallelIndexSet<G,L,s>& indices, int *nout, double eps)
{
  int procs, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);


  typedef Dune::FieldMatrix<double,BS,BS> Block;
  typedef Dune::BCRSMatrix<Block> BCRSMat;

  // calculate size of lokal matrix in the distributed direction
  int start, end, overlapStart, overlapEnd;

  int n = N/procs; // number of unknowns per process
  int bigger = N%procs; // number of process with n+1 unknows

  // Compute owner region
  if(rank<bigger) {
    start = rank*(n+1);
    end   = (rank+1)*(n+1);
  }else{
    start = bigger + rank * n;
    end   = bigger + (rank + 1) * n;
  }

  // Compute overlap region
  if(start>0)
    overlapStart = start - 1;
  else
    overlapStart = start;

  if(end<N)
    overlapEnd = end + 1;
  else
    overlapEnd = end;

  int noKnown = overlapEnd-overlapStart;

  *nout = noKnown;

  BCRSMat mat(noKnown*N, noKnown*N, noKnown*N*5, BCRSMat::row_wise);

  setupPattern(N, mat, indices, overlapStart, overlapEnd, start, end);
  fillValues(N, mat, overlapStart, overlapEnd, start, end, eps);

  //  Dune::printmatrix(std::cout,mat,"aniso 2d","row",9,1);

  return mat;
}
#endif
