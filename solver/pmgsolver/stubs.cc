// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "stubs.hh"

namespace Dune {
  namespace PMGStubs {
    /**
        one Gauss-Seidel step
     */
    template <class GRID, int SMOOTHER, int TYP>
    class GaussSeidel {
      enum { DIM = GRID::griddim };
      const pmgsolver<GRID,SMOOTHER> & solver;
      Dune::Vector<GRID> & x;
      Dune::Vector<GRID> & b;
      array<DIM> add;
    public:
      GaussSeidel(const pmgsolver<GRID,SMOOTHER> & solver_, level l) :
        solver(solver_), x(solver.x), b(solver.b),
        add(solver.g.init_add(l))
      {}
      void evaluate(int l, const array<DIM> & coord, int i) {
        // something to store the coeffs
        static coefflist cl = solver.discrete.newcoefflist();
        switch(TYP) {
        case Inner : {
          /* Coeffs */
          solver.discrete.coeffs(cl, l, coord, i);
#ifndef NDEBUG
          coefflist cl2 = cl;
          typename GRID::iterator it(i,solver.g);
          solver.discrete.coeffs(cl2, it);
          assert(cl.size == cl2.size);
          for (int n=0; n<cl.size; n++) {
            assert(cl.j[n] == cl2.j[n]);
            assert(cl.aij[n] == cl2.aij[n]);
          }
          assert(cl.aii == cl2.aii);
#endif
          break;
        } // end Inner
        case Border : {
          /* Boundry */
          typename GRID::iterator it(i,solver.g);
          if (! it->owner() ) return;
          if ( solver.discrete.bc.isdirichlet(it) ) return;
          /* Coeffs */
          solver.discrete.coeffs(cl, it);
          break;
        } // end Border
        } // end sitch

        x[i]=b[i];
        for (int n=0; n<cl.size; n++) {
          int j = cl.j[n];
          x[i] -= cl.aij[n] * x[j];
        }
        x[i] /= cl.aii;
        assert(finite(x[i]));
      } // end evaluate
    };

    /**
       the local defect
     */
    template <class GRID, int SMOOTHER, int TYP>
    class Defect {
      enum { DIM = GRID::griddim };
      const pmgsolver<GRID,SMOOTHER> & solver;
      Dune::Vector<GRID> & x;
      Dune::Vector<GRID> & b;
      array<DIM> add;
    public:
      array<2,double> defect_array;
      Defect(const pmgsolver<GRID,SMOOTHER> & solver_, level l) :
        solver(solver_), x(solver.x), b(solver.b),
        add(solver.g.init_add(l)), defect_array(0)
      {}
      void evaluate(level l, const array<DIM> & coord, int i) {
        /* do something */
        double defect = localdefect(l, coord, i);
        // update process defect
        defect_array[0] += defect*defect;
        defect_array[1] ++;
      }
      double localdefect(level l, const array<DIM> & coord, int i) const {
        double defect=b[i];
        // something to store the coeffs
        static coefflist cl = solver.discrete.newcoefflist();
        switch(TYP) {
        case Inner : {
          /* Coeffs */
          solver.discrete.coeffs(cl, l, coord, i);
          break;
        }
        case Border : {
          /* Boundry */
          typename GRID::iterator it(i,solver.g);
          if (! it->owner() ) return 0;
          if ( solver.discrete.bc.isdirichlet(it) ) return 0;
          /* Coeffs */
          solver.discrete.coeffs(cl, it);
          break;
        } // end Border
        } // end sitch
          /* Calc the defect */
        defect -= cl.aii * solver.x[i];
        for (int n=0; n<cl.size; n++) {
          int j = cl.j[n];
          defect -= cl.aij[n] * solver.x[j];
        } // end Inner
#ifndef NDEBUG
        if (!finite(defect))
          std::cerr << "DEFECT ERROR Element " << i
                    << coord << std::endl;
        assert(finite(defect));
#endif
        solver.d[i]=defect;
        return defect;
      } // end evaluate
    };

    template <class GRID, int SMOOTHER, int TYP>
    class Restrict {
      enum { DIM = GRID::griddim };
      pmgsolver<GRID,SMOOTHER> & solver;
      GRID & g;
      array<DIM> coord_shift;
      level L;
    public:
      Restrict(pmgsolver<GRID,SMOOTHER> & solver_, level l) :
        solver(solver_), g(solver.g), L(l)
      {
        for(int d=0; d<DIM; d++)
          coord_shift[d] = solver.g.coord_shift(l,d);
      }
      void evaluate(level l, const array<DIM> & coord, int i) {
        switch (TYP) {
        case Inner : {
          adddefect(solver.d[i], DIM, l, coord);
          break;
        }
        case Border : {
          typename GRID::iterator it(i,g);
          adddefect(solver.d[i], DIM, it);
          break;
        }
        }
      }
      void adddefect(double d, int dir, level l, array<DIM> coord){
        dir--;

        if (dir<0) {
          int f = g.father_id(l,coord);
          solver.b[f] += d;
          assert(finite(solver.b[f]));
          return;
        }

        if ( (coord[dir] + coord_shift[dir]) %2 == 0) {
          adddefect(d,dir,l,coord);
        }
        else {
          array<DIM> shiftl=coord;
          array<DIM> shiftr=coord;
          shiftl[dir]-=1;
          shiftr[dir]+=1;
          if (! (!g.do_end_share(dir) &&
                 coord[dir] ==
                 g.size(l,dir)
                 +g.end_overlap(l,dir) +g.front_overlap(l,dir)-1)) {
            adddefect(d/2.0,dir,l,shiftr);
          }
          if (! (!g.do_front_share(dir) &&
                 coord[dir]==0)) {
            adddefect(d/2.0,dir,l,shiftl);
          }
        }
      }
      void adddefect(double d, int dir,
                     typename GRID::iterator it){
        dir--;

        if (dir<0) {
          typename GRID::iterator f = it.father();
          solver.b[f.id()] += d;
          assert(finite(solver.b[f.id()]));
          return;
        }

        if ( (it.coord(dir) + coord_shift[dir]) %2 == 0) {
          adddefect(d,dir,it);
        }
        else {
          typename GRID::iterator left = it.left(dir);
          typename GRID::iterator right = it.right(dir);
          if (it!=left)
            adddefect(d/2.0,dir,left);
          if (it!=right)
            adddefect(d/2.0,dir,right);
        }
      }
    };

    template <class GRID, int SMOOTHER, int TYP>
    class Prolongate {
      enum { DIM = GRID::griddim };
      pmgsolver<GRID,SMOOTHER> & solver;
      GRID & g;
      array<DIM> coord_shift;
      Dune::Vector<GRID> & x;
    public:
      Prolongate(pmgsolver<GRID,SMOOTHER> & solver_, level l) :
        solver(solver_), g(solver.g), x(solver.x)
      {
        for(int d=0; d<DIM; d++)
          coord_shift[d] = solver.g.coord_shift(l,d);
      }
      void evaluate(level l, const array<DIM> & coord, int i) {
        switch (TYP) {
        case Inner : {
          x[i] += correction(DIM, l, coord);
          break;
        }
        case Border : {
          typename GRID::iterator it(i,g);
          if (!it->owner()) return;
          x[i] += correction(DIM, it);
          break;
        }
        }
        assert(finite(x[i]));
      }
      double correction(int dir, level l, array<DIM> coord) {
        dir--;

        if (dir<0) {
          int f=g.father_id(l, coord);
          return x[f];
        }

        if ( (coord[dir] + coord_shift[dir]) %2 == 0) {
          return correction(dir,l,coord);
        }
        else {
          array<DIM> shiftl=coord;
          array<DIM> shiftr=coord;
          shiftl[dir]-=1;
          shiftr[dir]+=1;
          return 0.5*correction(dir,l,shiftl) +
                 0.5*correction(dir,l,shiftr);
        }
      }
      double correction(int dir, typename GRID::iterator it) {
        dir--;

        if (dir<0) {
          typename GRID::iterator f=it.father();
          return x[f.id()];
        }

        if ( (it.coord(dir) + coord_shift[dir]) %2 == 0) {
          return correction(dir,it);
        }
        else {
          typename GRID::iterator left = it.left(dir);
          typename GRID::iterator right = it.right(dir);
          return 0.5*correction(dir,left) +
                 0.5*correction(dir,right);
        }
      }
    };

    template <class GRID>
    class InitIterator {
      enum { DIM = GRID::griddim };
      Dune::Vector<GRID> & b;
      Dune::Vector<GRID> & x;
      Discrete<GRID> & discrete;
      GRID & g;
    public:
      InitIterator(Dune::Vector<GRID> & B, Dune::Vector<GRID> & X,
                   Discrete<GRID> & D, GRID & G) :
        b(B), x(X), discrete(D), g(G) {};
      void evaluate(level l, const array<DIM> & coord, int i) {
        typename GRID::iterator it(i,g);
        b[i] = discrete.rhs(it);
        if (discrete.bc.isdirichlet(it))
          for (int d=0; d<DIM; d++) {
            for (Dune::side s = Dune::left; s <= Dune::right; s++) {
              Boundry bd = discrete.bc.boundry(it,d,s);
              if (bd.typ == dirichlet) x[i]=bd.value;
            }
          }
      }
    };

  } // PMGStubs
} // Dune
