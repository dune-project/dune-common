// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef EXEULER_HH
#define EXEULER_HH

#include "common/operator.hh"

#if 0

int main(int argc, char ** argv) {
  MPI_Init(&argc, &argv);
  enum { DIM=3 };
  Dune::array<DIM> size = 2;
  typedef Dune::spgrid<DIM> GRID;
  GRID grid(size,0,false);
  grid.refine(Dune::spgrid<DIM>::keepNumber);
  grid.refine(Dune::spgrid<DIM>::keepNumber);
  grid.refine(Dune::spgrid<DIM>::keepNumber);
  grid.refine(Dune::spgrid<DIM>::keepNumber);
  grid.refine(Dune::spgrid<DIM>::keepNumber);
  //  grid.refine(Dune::spgrid<DIM>::keepNumber);
  //  grid.refine(Dune::spgrid<DIM>::keepNumber);
  Dune::Vector<GRID> c(grid,4);
  c[1] = 5;
  Dune::Const c2(3);
  using namespace Dune;
  Const c3(2);
  Const c4(1);
  // Component wise calculation
  //Vector<GRID> v(grid); v = (c + (c2 + (c3 + (c4 + c))));// - c;
  Vector<GRID> v(grid); v = c + c2 + c3 + c4 + c; // - c;
  Vector<GRID> v2(grid); v2 = ( c + c2 ) * (c3 - c);
  //  Vector<GRID> v3(grid); v3 = shift(grid,c);
  /*
     std::cout << c << std::endl;
     std::cout << c2 << std::endl;
     std::cout << c3 << std::endl;
     std::cout << v << std::endl;
     std::cout << v2 << std::endl;
   */
  //  std::cout << v3 << std::endl;
  MPI_Finalize();
}

#endif

namespace Dune {
  /**
     exEuler calculates the next step
     \f$ x_{i,j,k}^{n+1} = \delta t \cdot RHS_{i,j,k}^n - x_{i,j,k}^n \f$
   */
  template <class GRID, class RHS>
  class exEuler : public gridOperator< GRID, exEuler<GRID,RHS> > {
  private:
    enum { DIM = GRID::griddim };
    typedef typename GRID::level level;
    level lvl;
    RHS & rhs;
    //! current solution
    Vector<GRID> &x;
    //! new solution
    Vector<GRID> &x_new;
  public:
    exEuler(const GRID &G, level LVL,
            RHS & Rhs,
            Vector<GRID> &X, Vector<GRID> &X_NEW) :
      gridOperator< GRID, exEuler<GRID,RHS> >(G),
      lvl(LVL), rhs(Rhs), x(X), x_new(X_NEW) {};
    void preProcess() { x_new = rhs - x; };
    void postProcess() {};
    FLOAT applyLocal(int i) {
      return x_new[i];
    };
  };

  template <class GRID, class RHS>
  exEuler<GRID,RHS> &
  exeuler(const GRID &g, typename GRID::level lvl,
          RHS & rhs,
          Vector<GRID> &x, Vector<GRID> &x_new) {
    static exEuler<GRID,RHS> *e=0;
    if (e) delete e;
    e = new exEuler<GRID,RHS>(g,lvl,rhs,x,x_new);
    return *e;
  };

}
#endif // EXEULER_HH
