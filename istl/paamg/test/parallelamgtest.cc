// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "config.h"
#include "anisotropic.hh"
#include <dune/common/timer.hh>
#include <dune/istl/paamg/amg.hh>
#include <dune/istl/paamg/pinfo.hh>
#include <dune/istl/indexset.hh>
#include <dune/istl/schwarz.hh>
#include <dune/istl/owneroverlapcopy.hh>
#include <dune/common/mpicollectivecommunication.hh>

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  const int BS=1;
  int N=250;
  int coarsenTarget=1200;

  if(argc>1)
    N = atoi(argv[1]);

  if(argc>2)
    coarsenTarget = atoi(argv[2]);

  std::cout<<"N="<<N<<" coarsenTarget="<<coarsenTarget<<std::endl;

  int procs, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);

  typedef Dune::FieldMatrix<double,BS,BS> MatrixBlock;
  typedef Dune::BCRSMatrix<MatrixBlock> BCRSMat;
  typedef Dune::FieldVector<double,BS> VectorBlock;
  typedef Dune::BlockVector<VectorBlock> Vector;
  typedef int LocalId;
  typedef int GlobalId;
  typedef Dune::OwnerOverlapCopyCommunication<LocalId,GlobalId> Communication;
  typedef Dune::OverlappingSchwarzOperator<BCRSMat,Vector,Vector,Communication> Operator;
  int n;

  Communication comm(MPI_COMM_WORLD);

  BCRSMat mat = setupAnisotropic2d<BS>(N, comm.indexSet(), &n, 1);

  comm.remoteIndices().rebuild<false>();

  Vector b(mat.N()), x(mat.M());

  b=0;
  x=100;

  if(N<6)
    Dune::printmatrix(std::cout, mat, "A", "row");


  Dune::Timer watch;

  watch.reset();
  Operator fop(mat, comm);

  typedef Dune::Amg::CoarsenCriterion<Dune::Amg::SymmetricCriterion<BCRSMat,Dune::Amg::FirstDiagonal> >
  Criterion;
  typedef Dune::SeqSSOR<BCRSMat,Vector,Vector> Smoother;
  typedef Dune::ParSSOR<BCRSMat,Vector,Vector,Communication> ParSmoother;
  typedef Dune::Amg::SmootherTraits<ParSmoother>::Arguments SmootherArgs;

  SmootherArgs smootherArgs;

  smootherArgs.iterations = 2;


  Criterion criterion(15,coarsenTarget);
  criterion.setMaxDistance(2);

  typedef Dune::Amg::AMG<Operator,Vector,ParSmoother,Communication> AMG;

  AMG amg(fop, criterion, smootherArgs, 1, 1, comm);


  double buildtime = watch.elapsed();

  std::cout<<"Building hierarchy took "<<buildtime<<" seconds"<<std::endl;

  Dune::OverlappingSchwarzScalarProduct<Vector,Communication> sp(comm);
  Dune::CGSolver<Vector> amgCG(fop, sp, amg, 10e-8, 80, 2);
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
