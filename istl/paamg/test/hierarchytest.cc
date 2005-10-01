// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include "mpi.h"
#include <dune/istl/paamg/hierarchy.hh>
#include <dune/istl/paamg/smoother.hh>
#include <dune/istl/preconditioners.hh>
#include "anisotropic.hh"
int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  const int BS=1, N=8;

  int procs, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);

  IndexSet indices;
  typedef Dune::FieldMatrix<double,BS,BS> MatrixBlock;
  typedef Dune::BCRSMatrix<MatrixBlock> BCRSMat;
  typedef Dune::FieldVector<double,BS> VectorBlock;
  typedef Dune::BlockVector<VectorBlock> Vector;

  int n;

  BCRSMat mat = setupAnisotropic2d<N,BS>(indices, &n);
  Vector b(indices.size());

  RemoteIndices remoteIndices(indices,indices,MPI_COMM_WORLD);
  remoteIndices.rebuild<false>();

  typedef Dune::Interface<IndexSet> Interface;

  Interface interface;

  typedef Dune::EnumItem<GridFlag,overlap> OverlapFlags;
  typedef Dune::Amg::MatrixHierarchy<BCRSMat,IndexSet,OverlapFlags> Hierarchy;
  typedef Dune::Amg::Hierarchy<Vector> VHierarchy;

  interface.build(remoteIndices, Dune::NegateSet<OverlapFlags>(), OverlapFlags());

  Hierarchy hierarchy(mat, indices, remoteIndices, interface);
  VHierarchy vh(b);

  typedef Dune::Amg::CoarsenCriterion<Dune::Amg::SymmetricCriterion<BCRSMat,Dune::Amg::FirstDiagonal> >
  Criterion;

  Criterion criterion(10,4);

  hierarchy.build(criterion);
  hierarchy.coarsenVector(vh);

  typedef Dune::SeqSSOR<BCRSMat,Vector,Vector> Smoother;
  Dune::Amg::SmootherTraits<Smoother>::Arguments smootherArgs;

  Smoother smoother(mat, 1, 1.0);

  Dune::Amg::Hierarchy<Smoother> sh(smoother);

  hierarchy.coarsenSmoother(sh,smootherArgs);

  std::cout<<"=== Vector hierarchy has "<<vh.levels()<<" levels! ==="<<std::endl;

  std::cout<<"=== Smoother hierarchy has "<<sh.levels()<<" levels! ==="<<std::endl;
  //typedef typename Hierarchy::ConstIterator Iterator;
  //for(Iterator level = matrices_.finest(), coarsest=matrices_.coarsest(); level!=coarsest; ++amap){
  hierarchy.recalculateGalerkin();


  MPI_Finalize();

}
