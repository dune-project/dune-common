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
    friend class PMGStubs::RelocateIterator<GRID>;
    friend class PMGStubs::InitIterator<GRID>;

    enum { DIM = GRID::griddim };
    typedef double data;
    typedef struct {
      int id; data value;
    } rhs;
    GRID &g;
    int n1, n2; // Vor-/Nachglaetter-Zyklen
    double reduction; // defekt-reduction
    int Processes; /**< number of der processes */
    int rank;
    bool need_relocate; /**< true if no dirichlet-condition is specified */
    double relocate_array[2]; /**< array for average calulation */
    Discrete<GRID> & discrete;
    Vector<GRID> & x; /**< solution vector */
    Vector<GRID> & b; /**< rhs vector */
    Vector<GRID> & d; /**< defect vector */
  private:
    double defect(level l) const;
    void restrict (level l);
    void   prolongate(level l);
    void   relocate(level l);
    void   smoother(level l) {
      relocate_array[0] = 0;
      relocate_array[1] = 0;
      switch (SMOOTHER) {
      case GaussSeidel : smootherGaussSeidel(l); break;
      case Jacobi : smootherJacobi(l); break;
      default :
        throw std::string("Unknown Smoother");
      }
      relocate(l);
    };
    void   smootherGaussSeidel(level l);
    void   smootherJacobi(level l);
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
    ~pmgsolver() {}
    void solve(int,level);
    void solveNested();
    void init(level);
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
  }; // class pmgsolver

} // namespace Dune

#include "pmgsolver/stubs.cc"
#include "pmgsolver/pmgsolver.cc"

#endif // PMGSOLVER_HH
