// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef PMGSOLVER_HH
#define PMGSOLVER_HH

#include <mpi.h>
#include <cstdlib>

#include <math.h>
#include <dune/grid/spgrid/array.hh>

using namespace Dune;

#warning we need an intelligent way of coordinating the MPI_Send-Tags
const int exchange_tag=42;

enum smootherTyp { GaussSeidel, Jacobi };

double TIME_SMOOTHER;
double TIME_REST;
double TIME_PROL;
double TIME_EX;
double TIME_DEFECT;

/* Forward declaration of the loopstubs */
template <class GRID>
class loopstubGaussSeidel;
template <class GRID>
class loopstubGaussSeidelBorder;
template <class GRID, int SMOOTHER>
class loopstubDefect;
template <class GRID, int SMOOTHER>
class loopstubRestrict;
template <class GRID, int SMOOTHER>
class loopstubProlongate;
template <class GRID, int SMOOTHER>
class loopstubInitExchange;
template <class GRID>
class loopstubInitIterator;

/**
   Mehrgitterlöser einer Finite-Volumen-Diskretisierung
 */
template <class GRID, int SMOOTHER>
class pmgsolver {
private:
  /** Friendclasses implementing out loopstubs */
  friend class loopstubGaussSeidel<GRID>;
  friend class loopstubGaussSeidelBorder<GRID>;
  friend class loopstubDefect<GRID,SMOOTHER>;
  friend class loopstubRestrict<GRID,SMOOTHER>;
  friend class loopstubProlongate<GRID,SMOOTHER>;
  friend class loopstubInitExchange<GRID,SMOOTHER>;
  friend class loopstubInitIterator<GRID>;

  enum { DIM = GRID::griddim };
  typedef double data;
  typedef data* vec;
  typedef typename GRID::level level;
  typedef struct {
    int id; data value;
  } rhs;
  GRID &g;
  int n1, n2; // Vor-/Nachglaetter-Zyklen
  double d; // defekt-reduction
  int Processes; // Anzahl der Prozesse
  int rank;
  bool need_recalc; // true if no dirichlet-condition is specified
  discrete<GRID> &mydiscrete;
public:
  data* b; // rhs, stores the the inbox flow
  data* defect_v; // defect
private:
  double defect(level l, vec x, vec b, vec d=0) const;
  double localdefect(level, const array<DIM>&, const array<DIM>&,
                     int, vec x, vec b, vec d=0) const;
  void adddefect(double, vec, int,
                 typename GRID::level, array<DIM>, array<DIM>);
  void restrict (level l, vec x, vec b);
  double correction(vec x, int dir,
                    typename GRID::level, array<DIM>, array<DIM>);
  void   prolongate(level l, vec x);
  void   smoother(level l, vec x, vec b) {
    switch (SMOOTHER) {
    case GaussSeidel : smootherGaussSeidel(l,x,b); break;
    case Jacobi : smootherJacobi(l,x,b); break;
    default :
      throw std::string("Unknown Smoother");
    }
  };
  void   smootherGaussSeidel(level l, vec x, vec b);
  void   smootherJacobi(level l, vec x, vec b);
  /* datatyp for exchange */
  MPI_Datatype indexed_double_type;
  typedef struct {
    int id;
    double data;
  } indexed_double;
  MPI_Status mpi_status;
  typedef struct {
    int size;
    int* id;
  } exchange_data;
  exchange_data** exchange_data_from;
  exchange_data** exchange_data_to;
  void   initExchange();              /**< Datenabgleich vorbereiten */
  void   exchange(level l, vec ex);   /**< Datenabgleich auf Level l */
  void   mgc (level l, vec x, vec b); /**< Multi-Grid-Cycle */
public:
  pmgsolver(GRID &_g, double _d, discrete<GRID> &d,
            int _n1, int _n2) :
    g(_g), n1(_n1), n2(_n2), d(_d), mydiscrete(d)
  {

    MPI_Comm_size(g.comm(), &Processes);
    MPI_Comm_rank(g.comm(), &rank);

    /* create a new  type and register the according MPI_Datatype */
    int array_of_blocklengths[2] = { 1, 1 };
    MPI_Aint array_of_displacements[2] = {0, sizeof(int)};
    MPI_Datatype array_of_types[2] = { MPI_INT, MPI_DOUBLE };
    MPI_Type_struct(2, array_of_blocklengths,
                    array_of_displacements, array_of_types,
                    &indexed_double_type );
    MPI_Type_commit( &indexed_double_type );
  };
  ~pmgsolver() {
    std::cerr << "exchange_data_from not cleaned up!!!\n";
    std::cerr << "exchange_data_to not cleaned up!!!\n";
    /*
       free(exchange_data_from);
       free(exchange_data_to);
     */
    MPI_Type_free(&indexed_double_type);
  }
  void solve(vec);
  void init(vec);
  inline void initIterator(vec x, typename GRID::iterator it) {
    int i=it.id();
    b[i] = mydiscrete.rhs(it);
    x[i] = 0;
    if (mydiscrete.isdirichlet(it))
      for (int d=0; d<DIM; d++) {
        for (int s = Dune::left; s != Dune::end; s++) {
          Boundry bd = mydiscrete.boundry(it,d,(Dune::side)s);
          if (bd.typ == dirichlet) x[i]=bd.value;
        }
      }
  }
};

#include "pmgsolver/pmgsolver.cc"

#endif // PMGSOLVER_HH
