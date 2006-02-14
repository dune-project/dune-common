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
#include <string>

template<class T>
class DoubleStepPreconditioner
  : public Dune::Preconditioner<typename T::domain_type, typename T::range_type>
{
public:
  typedef typename T::domain_type X;
  typedef typename T::range_type Y;

  enum {category = T::category};

  DoubleStepPreconditioner(T& preconditioner_)
    : preconditioner(&preconditioner_)
  {}

  virtual void pre (X& x, Y& b)
  {
    preconditioner->pre(x,b);
  }

  virtual void apply(X& v, const Y& d)
  {
    preconditioner->apply(v,d);
    preconditioner->apply(v,d);
  }

  virtual void post (X& x)
  {
    preconditioner->post(x);
  }
private:
  T* preconditioner;
};


class MPIError {
public:
  /** @brief Constructor. */
  MPIError(std::string s, int e) : errorstring(s), errorcode(e){}
  /** @brief The error string. */
  std::string errorstring;
  /** @brief The mpi error code. */
  int errorcode;
};

void MPI_err_handler(MPI_Comm *comm, int *err_code, ...){
  char *err_string=new char[MPI_MAX_ERROR_STRING];
  int err_length;
  MPI_Error_string(*err_code, err_string, &err_length);
  std::string s(err_string, err_length);
  std::cerr << "An MPI Error ocurred:"<<std::endl<<s<<std::endl;
  delete[] err_string;
  throw MPIError(s, *err_code);
}

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);
  MPI_Errhandler handler;
  MPI_Errhandler_create(MPI_err_handler, &handler);
  MPI_Errhandler_set(MPI_COMM_WORLD, handler);

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

  const BCRSMat& cmat = mat;

  comm.remoteIndices().rebuild<false>();

  Vector b(cmat.N()), x(cmat.M());

  b=0;
  x=100;

  setBoundary(x, N, comm.indexSet());

  Vector b1=b, x1=x;

  if(N<6 && rank==0) {
    Dune::printmatrix(std::cout, cmat, "A", "row");
    Dune::printvector(std::cout, x, "x", "row");
    Dune::printvector(std::cout, b, "b", "row");
    Dune::printvector(std::cout, b1, "b1", "row");
    Dune::printvector(std::cout, x1, "x1", "row");
  }

  Dune::Timer watch;

  watch.reset();
  Operator fop(cmat, comm);

  typedef Dune::Amg::CoarsenCriterion<Dune::Amg::SymmetricCriterion<BCRSMat,Dune::Amg::FirstDiagonal> >
  Criterion;
  typedef Dune::SeqJac<BCRSMat,Vector,Vector> Smoother;
  typedef Dune::BlockPreconditioner<Vector,Vector,Communication,Smoother> ParSmoother;
  //typedef Dune::ParSSOR<BCRSMat,Vector,Vector,Communication> ParSmoother;
  typedef Dune::Amg::SmootherTraits<ParSmoother>::Arguments SmootherArgs;
  //typedef Dune::Amg::BlockPreconditionerConstructionArgs<Smoother,Communication> SmootherArgs;

  SmootherArgs smootherArgs;

  smootherArgs.iterations = 1;


  Criterion criterion(15,coarsenTarget);
  criterion.setMaxDistance(2);

  typedef Dune::Amg::AMG<Operator,Vector,ParSmoother,Communication> AMG;

  AMG amg(fop, criterion, smootherArgs, 1, 1, comm);


  double buildtime = watch.elapsed();

  std::cout<<"Building hierarchy took "<<buildtime<<" seconds"<<std::endl;

  Dune::OverlappingSchwarzScalarProduct<Vector,Communication> sp(comm);

  Dune::CGSolver<Vector> amgCG(fop, sp, amg, 10e-8, 800, (rank==0) ? 2 : 0);
  watch.reset();
  Dune::InverseOperatorResult r, r1;

  amgCG.apply(x,b,r);
  MPI_Barrier(MPI_COMM_WORLD);
  Smoother ssm(fop.getmat(),1,.8);
  ParSmoother sm(ssm,comm);
  DoubleStepPreconditioner<ParSmoother> dsp(sm);
  Dune::CGSolver<Vector> cg(fop, sp, sm, 10e-08, 800, (rank==0) ? 2 : 0);

  double solvetime = watch.elapsed();

  std::cout<<"AMG solving took "<<solvetime<<" seconds"<<std::endl;

  std::cout<<"AMG building took "<<(buildtime/r.elapsed*r.iterations)<<" iterations"<<std::endl;
  std::cout<<"AMG building together with slving took "<<buildtime+solvetime<<std::endl;

  watch.reset();

  cg.apply(x1,b1,r1);

  //std::cout<<"CG solving took "<<watch.elapsed()<<" seconds"<<std::endl;

  MPI_Finalize();
}
