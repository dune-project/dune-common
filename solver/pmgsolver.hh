// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef PMGSOLVER_HH
#define PMGSOLVER_HH

#include <mpi.h>
#include <cstdlib>

#include <math.h>
#include <dune/grid/spgrid/array.hh>
#include "common/operator.hh"
#include "pmgsolver/stubs.hh"

namespace Dune {

  template <class T>
  T* realloc(T* & pointer, int size) {
    if (size==0) {
      std::cerr << "Warning request for realloc with size=0\n";
      size = 1;
    }
    pointer =
      static_cast<T*>(::realloc(static_cast<void*>(pointer),
                                size*sizeof(T)));
    if (pointer == 0)
      throw std::string("Bad realloc");
    return pointer;
  }

  template <class T>
  T* malloc(int size) {
    return static_cast<T*>(::malloc(size*sizeof(T)));
  }

#warning we need an intelligent way of coordinating the MPI_Send-Tags
  const int exchange_tag=42;

  enum smootherTyp { GaussSeidel, Jacobi };

  double TIME_SMOOTHER;
  double TIME_REST;
  double TIME_PROL;
  double TIME_EX;
  double TIME_DEFECT;

  /**
     Mehrgitterlöser einer Finite-Volumen-Diskretisierung
   */
  template <class GRID, int SMOOTHER>
  class pmgsolver {
  private:
    /** Friendclasses implementing out loopstubs */
    friend class PMGStubs::GaussSeidel<GRID,SMOOTHER,PMGStubs::Inner>;
    friend class PMGStubs::GaussSeidel<GRID,SMOOTHER,PMGStubs::Border>;
    friend class PMGStubs::Defect<GRID,SMOOTHER,PMGStubs::Inner>;
    friend class PMGStubs::Defect<GRID,SMOOTHER,PMGStubs::Border>;
    friend class PMGStubs::Restrict<GRID,SMOOTHER,PMGStubs::Inner>;
    friend class PMGStubs::Restrict<GRID,SMOOTHER,PMGStubs::Border>;
    friend class PMGStubs::Prolongate<GRID,SMOOTHER,PMGStubs::Inner>;
    friend class PMGStubs::Prolongate<GRID,SMOOTHER,PMGStubs::Border>;
    friend class PMGStubs::InitExchange<GRID,SMOOTHER>;
    friend class PMGStubs::InitIterator<GRID>;

    enum { DIM = GRID::griddim };
    typedef double data;
    typedef typename GRID::level level;
    typedef struct {
      int id; data value;
    } rhs;
    GRID &g;
    int n1, n2; // Vor-/Nachglaetter-Zyklen
    double reduction; // defekt-reduction
    int Processes; // Anzahl der Prozesse
    int rank;
    bool need_recalc; // true if no dirichlet-condition is specified
    Discrete<GRID> & discrete;
    Vector<GRID> & x; /**< solution vector */
    Vector<GRID> & b; /**< rhs vector */
    Vector<GRID> & d; /**< defect vector */
  private:
    double defect(level l) const;
    void restrict (level l);
    void   prolongate(level l);
    void   smoother(level l) {
      switch (SMOOTHER) {
      case GaussSeidel : smootherGaussSeidel(l); break;
      case Jacobi : smootherJacobi(l); break;
      default :
        throw std::string("Unknown Smoother");
      }
    };
    void   smootherGaussSeidel(level l);
    void   smootherJacobi(level l);
    /* datatyp for exchange */
    typedef struct {
      int size;
      int* id;
    } exchange_data;
    exchange_data** exchange_data_from;
    exchange_data** exchange_data_to;
    MPI_Status mpi_status;
    /**< prepare dataexchange */
    void   initExchange();
    /**< exchange data on level l */
    void   exchange(level l, Vector<GRID> & ex);
    /**< Multi-Grid-Cycle */
    void   mgc (level l);
  public:
    pmgsolver(GRID &_g, double _reduction, Discrete<GRID> &dis,
              int _n1, int _n2,
              Vector<GRID> & X, Vector<GRID> & B, Vector<GRID> & D) :
      g(_g), n1(_n1), n2(_n2), reduction(_reduction), discrete(dis),
      x(X), b(B), d(D)
    {
      MPI_Comm_size(g.comm(), &Processes);
      MPI_Comm_rank(g.comm(), &rank);
    };
    ~pmgsolver() {
      std::cerr << "exchange_data_from not cleaned up!!!\n";
      std::cerr << "exchange_data_to not cleaned up!!!\n";
      /*
         free(exchange_data_from);
         free(exchange_data_to);
       */
    }
    void solve(int);
    void init();
    inline void initIterator(typename GRID::iterator it) {
      int i=it.id();
      b[i] = discrete.rhs(it);
      x[i] = 0;
      if (discrete.isdirichlet(it))
        for (int d=0; d<DIM; d++) {
          for (int s = Dune::left; s != Dune::end; s++) {
            Boundry bd = discrete.bc.boundry(it,d,(Dune::side)s);
            if (bd.typ == dirichlet) x[i]=bd.value;
          }
        }
    }
    #warning debugcode
    void restricttest() {
      init();
      level l=g.smoothest();
      for(typename GRID::iterator it=g.begin(l);
          it != g.end(l); ++it)
        x[it.id()] = 0;
      for(typename GRID::iterator it=g.begin(l-1);
          it != g.end(l-1); ++it) {
        //        if ( discrete.bc.isdirichlet(it) ) continue;
        x[it.id()] = it.coord(0)+it.coord(1)+it.coord(2);
        std::cout << it.coord()
                  << it.coord(0)+it.coord(1)+it.coord(2)
                  << std::endl;
      }
      dump(g,l,x,"restrict","D before restrict");
      dump(g,l-1,x,"restrict","B before restrict");
      prolongate(l);
      dump(g,l,x,"restrict","D after restrict");
      dump(g,l-1,x,"restrict","B after restrict");
    }
  }; // class pmgsolver

} // namespace Dune

#include "pmgsolver/stubs.cc"
#include "pmgsolver/pmgsolver.cc"

#endif // PMGSOLVER_HH
