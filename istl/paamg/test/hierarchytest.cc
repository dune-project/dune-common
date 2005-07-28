// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "mpi.h"
#include <dune/istl/paamg/hierarchy.hh>
#include "anisotropic.hh"
int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  const int BS=1, N=8;

  int procs, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);

  IndexSet indices;
  typedef Dune::FieldMatrix<double,BS,BS> Block;
  typedef Dune::BCRSMatrix<Block> BCRSMat;
  int n;

  BCRSMat mat = setupAnisotropic2d<N,BS>(indices, &n);

  RemoteIndices remoteIndices(indices,indices,MPI_COMM_WORLD);
  remoteIndices.rebuild<false>();

  typedef Dune::Interface<IndexSet> Interface;

  Interface interface;

  typedef Dune::EnumItem<GridFlag,overlap> OverlapFlags;
  typedef Dune::Amg::MatrixHierarchy<BCRSMat,IndexSet,OverlapFlags> Hierarchy;

  interface.build(remoteIndices, Dune::NegateSet<OverlapFlags>(), OverlapFlags());

  Hierarchy hierarchy(mat, indices, remoteIndices, interface);

  typedef Dune::Amg::CoarsenCriterion<Dune::Amg::SymmetricCriterion<BCRSMat,Dune::Amg::FirstDiagonal> >
  Criterion;

  Criterion criterion(10,4);

  hierarchy.build(criterion);

  MPI_Finalize();

}
