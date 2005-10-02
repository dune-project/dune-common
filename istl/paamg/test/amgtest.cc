// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "config.h"
#include "anisotropic.hh"
#include <dune/istl/paamg/amg.hh>
#include <dune/istl/indexset.hh>

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
  Vector b(indices.size()), x(indices.size());

  b=1;
  x=100;

  RemoteIndices remoteIndices(indices,indices,MPI_COMM_WORLD);
  remoteIndices.rebuild<false>();

  typedef Dune::Interface<IndexSet> Interface;

  Interface interface;

  typedef Dune::EnumItem<GridFlag,overlap> OverlapFlags;
  typedef Dune::Amg::MatrixHierarchy<BCRSMat,IndexSet,OverlapFlags> MHierarchy;
  typedef Dune::Amg::Hierarchy<Vector> VHierarchy;

  interface.build(remoteIndices, Dune::NegateSet<OverlapFlags>(), OverlapFlags());

  MHierarchy hierarchy(mat, indices, remoteIndices, interface);
  VHierarchy vh(b);

  typedef Dune::Amg::CoarsenCriterion<Dune::Amg::SymmetricCriterion<BCRSMat,Dune::Amg::FirstDiagonal> >
  Criterion;

  Criterion criterion(10,4);

  hierarchy.build(criterion);
  hierarchy.coarsenVector(vh);

  typedef Dune::SeqSSOR<BCRSMat,Vector,Vector> Smoother;
  typedef Dune::Amg::SmootherTraits<Smoother>::Arguments SmootherArgs;
  SmootherArgs smootherArgs;

  Dune::MatrixAdapter<BCRSMat,Vector,Vector> op(hierarchy.matrices().coarsest()->matrix());
  Dune::SeqSSOR<BCRSMat,Vector,Vector> ssor(hierarchy.matrices().coarsest()->matrix(),1,1.0);
  typedef Dune::LoopSolver<Vector> Solver;
  Solver solver(op,ssor,1E-6,8000,2);
  Dune::SeqScalarProduct<Vector> sp;
  typedef Dune::Amg::AMG<MHierarchy,Vector,Vector,Solver,Smoother> AMG;

  AMG amg(hierarchy, solver, smootherArgs, 1, 1);

  amg.pre(x, b);
  amg.apply(x,b);
  amg.post(x);

  MPI_Finalize();
}
