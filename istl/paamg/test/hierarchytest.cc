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

  const int BS=1, N=100;

  int procs, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  typedef Dune::ParallelIndexSet<int,LocalIndex,512> ParallelIndexSet;
  typedef Dune::Amg::ParallelInformation<ParallelIndexSet> ParallelInformation;
  typedef Dune::FieldMatrix<double,BS,BS> MatrixBlock;
  typedef Dune::BCRSMatrix<MatrixBlock> BCRSMat;
  typedef Dune::FieldVector<double,BS> VectorBlock;
  typedef Dune::BlockVector<VectorBlock> Vector;

  int n;
  ParallelInformation pinfo(MPI_COMM_WORLD);
  ParallelIndexSet& indices = pinfo.indexSet();

  typedef Dune::RemoteIndices<ParallelIndexSet> RemoteIndices;
  RemoteIndices& remoteIndices = pinfo.remoteIndices();
  BCRSMat mat = setupAnisotropic2d<BS>(N, indices, &n);
  Vector b(indices.size());

  remoteIndices.rebuild<false>();

  typedef Dune::Interface<ParallelIndexSet> Interface;

  Interface interface;

  typedef Dune::EnumItem<GridFlag,GridAttributes::overlap> OverlapFlags;
  typedef Dune::Amg::ParallelMatrix<BCRSMat,ParallelIndexSet,Vector,Vector> Operator;
  typedef Dune::Amg::MatrixHierarchy<Operator,ParallelInformation> Hierarchy;
  typedef Dune::Amg::Hierarchy<Vector> VHierarchy;

  interface.build(remoteIndices, Dune::NegateSet<OverlapFlags>(), OverlapFlags());
  Operator op(mat, pinfo);
  Hierarchy hierarchy(op, pinfo);
  VHierarchy vh(b);

  typedef Dune::Amg::CoarsenCriterion<Dune::Amg::SymmetricCriterion<BCRSMat,Dune::Amg::FirstDiagonal> >
  Criterion;

  Criterion criterion(100,4);

  hierarchy.build<OverlapFlags>(criterion);
  hierarchy.coarsenVector(vh);


  std::cout<<"=== Vector hierarchy has "<<vh.levels()<<" levels! ==="<<std::endl;

  hierarchy.recalculateGalerkin();


  MPI_Finalize();

}
