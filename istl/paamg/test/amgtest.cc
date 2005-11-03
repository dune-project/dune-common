// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "config.h"
#include "anisotropic.hh"
#include <dune/common/timer.hh>
#include <dune/istl/paamg/amg.hh>
#include <dune/istl/indexset.hh>

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  const int BS=1, N=1000;

  int procs, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);

  ParallelIndexSet indices;
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

  typedef Dune::Interface<ParallelIndexSet> Interface;

  Interface interface;

  typedef Dune::EnumItem<GridFlag,overlap> OverlapFlags;
  typedef Dune::Amg::MatrixHierarchy<BCRSMat,ParallelIndexSet,OverlapFlags> MHierarchy;
  typedef Dune::Amg::Hierarchy<Vector> VHierarchy;

  interface.build(remoteIndices, Dune::NegateSet<OverlapFlags>(), OverlapFlags());

  MHierarchy hierarchy(mat, indices, remoteIndices, interface);
  VHierarchy vh(b);

  typedef Dune::Amg::CoarsenCriterion<Dune::Amg::SymmetricCriterion<BCRSMat,Dune::Amg::FirstDiagonal> >
  Criterion;

  Dune::Timer watch;
  watch.reset();
  Criterion criterion(1,100);
  criterion.setMaxDistance(3);

  hierarchy.build(criterion);
  std::cout<<"Building hierarchy took "<<watch.elapsed()<<" seconds"<<std::endl;
  /*
     hierarchy.coarsenVector(vh);

     typedef Dune::SeqSSOR<BCRSMat,Vector,Vector> Smoother;
     typedef Dune::Amg::SmootherTraits<Smoother>::Arguments SmootherArgs;
     SmootherArgs smootherArgs;

     Dune::MatrixAdapter<BCRSMat,Vector,Vector> op(hierarchy.matrices().coarsest()->matrix());
     Dune::SeqSSOR<BCRSMat,Vector,Vector> cssor(hierarchy.matrices().coarsest()->matrix(),1,1.0);
     Dune::SeqSSOR<BCRSMat,Vector,Vector> ssor(hierarchy.matrices().finest()->matrix(),1,1.0);
     typedef Dune::LoopSolver<Vector> CoarseSolver;
     CoarseSolver csolver(op,cssor,1E-12,8000,0);
     Dune::SeqScalarProduct<Vector> sp;
     typedef Dune::Amg::AMG<MHierarchy,Vector,Vector,CoarseSolver,Smoother> AMG;

     AMG amg(hierarchy, csolver, smootherArgs, 1, 1);
     typedef Dune::MatrixAdapter<BCRSMat,Vector,Vector> Operator;
     Operator fop(hierarchy.matrices().finest()->matrix());
     Dune::CGSolver<Vector> amgCG(fop,amg,10e-8,8000,2);
     Dune::CGSolver<Vector> cg(fop,ssor,10e-8,8000,2);

     watch.reset();
     Dune::InverseOperatorResult r;
     amgCG.apply(x,b,r);

     std::cout<<"AMG solving took "<<watch.elapsed()<<" seconds"<<std::endl;

     watch.reset();
     cg.apply(x,b,r);

     std::cout<<"CG solving took "<<watch.elapsed()<<" seconds"<<std::endl;
   */
  MPI_Finalize();
}
