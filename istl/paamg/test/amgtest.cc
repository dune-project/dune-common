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

  const int BS=1;
  int N=250;

  if(argc>1)
    N = atoi(argv[1]);

  int procs, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);

  ParallelIndexSet indices;
  typedef Dune::FieldMatrix<double,BS,BS> MatrixBlock;
  typedef Dune::BCRSMatrix<MatrixBlock> BCRSMat;
  typedef Dune::FieldVector<double,BS> VectorBlock;
  typedef Dune::BlockVector<VectorBlock> Vector;

  int n;

  BCRSMat mat = setupAnisotropic2d<BS>(N, indices, &n, 1);

  /*
     if(N<20)
      Dune::printmatrix(std::cout, mat, "A", "row");
   */

  Vector b(indices.size()), x(indices.size());

  b=0;
  x=100;
  Dune::MatrixAdapter<BCRSMat,Vector,Vector> fop(mat);

  Dune::Timer watch;

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

  watch.reset();
  Criterion criterion(15,1200);
  criterion.setMaxDistance(2);

  hierarchy.build(criterion);

  double buildtime = watch.elapsed();
  std::cout<<"Building hierarchy took "<<buildtime<<" seconds"<<std::endl;


  if(N<20)
    Dune::printmatrix(std::cout, hierarchy.matrices().coarsest()->matrix(), "A", "row");


  hierarchy.coarsenVector(vh);

  typedef Dune::SeqSSOR<BCRSMat,Vector,Vector> Smoother;
  typedef Dune::Amg::SmootherTraits<Smoother>::Arguments SmootherArgs;
  SmootherArgs smootherArgs;

  smootherArgs.iterations = 2;

  Dune::MatrixAdapter<BCRSMat,Vector,Vector> op(hierarchy.matrices().coarsest()->matrix());
  Dune::SeqSSOR<BCRSMat,Vector,Vector> cssor(hierarchy.matrices().coarsest()->matrix(),1,1.0);

  Dune::SeqSSOR<BCRSMat,Vector,Vector> ssor(hierarchy.matrices().finest()->matrix(),1,1.0);

  typedef Dune::LoopSolver<Vector> CoarseSolver;
  CoarseSolver csolver(op,cssor,1E-12,8000,0);
  Dune::SeqScalarProduct<Vector> sp;
  typedef Dune::Amg::AMG<MHierarchy,Vector,Smoother> AMG;

  AMG amg(hierarchy, csolver, smootherArgs, 1, 1);

  Dune::CGSolver<Vector> amgCG(fop,amg,10e-8,80,2);

  /*
     Dune::SeqSSOR<BCRSMat,Vector,Vector> fssor(mat,1,1.0);
     Dune::CGSolver<Vector> cg(fop,fssor,10e-8,8,2);
   */
  watch.reset();
  Dune::InverseOperatorResult r;
  amgCG.apply(x,b,r);

  double solvetime = watch.elapsed();

  std::cout<<"AMG solving took "<<solvetime<<" seconds"<<std::endl;

  std::cout<<"AMG building took "<<(buildtime/r.elapsed*r.iterations)<<" iterations"<<std::endl;
  std::cout<<"AMG building together with slving took "<<buildtime+solvetime<<std::endl;

  /*
     watch.reset();
     cg.apply(x,b,r);

     std::cout<<"CG solving took "<<watch.elapsed()<<" seconds"<<std::endl;
   */
  MPI_Finalize();
}
