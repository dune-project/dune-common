// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
namespace Dune {
  namespace PMGStubs {

    /**
        one Gauss-Seidel step
     */
    template <class GRID>
    class GaussSeidel {
      enum { DIM = GRID::griddim };
      Dune::Vector<GRID> & x;
      Dune::Vector<GRID> & b;
      GRID & g;
      coefflist & cl;
      discrete<GRID> &mydiscrete;
      array<DIM> add;
    public:
      GaussSeidel(Dune::Vector<GRID> & _x, Dune::Vector<GRID> & _b,
                  GRID & _g,
                  int l, coefflist & _cl, discrete<GRID> &d) :
        x(_x), b(_b), g(_g), cl(_cl), mydiscrete(d), add(0) {
        add = g.init_add(l);
      }
      void evaluate(int l, const array<DIM> & coord, int i) {
        /* Coeffs */
        mydiscrete.coeffs(cl, g, l, add, coord, i);

        x[i]=b[i];
        for (int n=0; n<cl.size; n++) {
          int j = cl.j[n];
          x[i] -= cl.aij[n] * x[j];
        }
        x[i] /= cl.aii;
      }
    };

    /**
        one Gauss-Seidel step on the border
     */
    template <class GRID>
    class GaussSeidelBorder {
      enum { DIM = GRID::griddim };
      Dune::Vector<GRID> & x;
      Dune::Vector<GRID> & b;
      GRID & g;
      coefflist & cl;
      discrete<GRID> &mydiscrete;
      array<DIM> add;
    public:
      GaussSeidelBorder(Dune::Vector<GRID> & _x,
                        Dune::Vector<GRID> & _b, GRID & _g,
                        int l, coefflist & _cl, discrete<GRID> &d) :
        x(_x), b(_b), g(_g), cl(_cl), mydiscrete(d), add(0) {
        add = g.init_add(l);
      }
      void evaluate(int l, const array<DIM> & coord, int i) {
        /* Coeffs */
#warning BORDER GS not implemented!!
        return;

        mydiscrete.coeffs(cl, g, l, add, coord, i);

        x[i]=b[i];
        for (int n=0; n<cl.size; n++) {
          int j = cl.j[n];
          x[i] -= cl.aij[n] * x[j];
        }
        x[i] /= cl.aii;
      }
    };

    template <class GRID, int SMOOTHER>
    class Defect {
      typedef int level;
      enum { DIM = GRID::griddim };
      const pmgsolver<GRID,SMOOTHER> & solver;
      Dune::Vector<GRID> & x;
      Dune::Vector<GRID> & b;
      array<DIM> add;
    public:
      array<2,double> defect_array;
      Defect(const pmgsolver<GRID,SMOOTHER> & solver_,
             Dune::Vector<GRID> & X, Dune::Vector<GRID> & B,
             level l) :
        solver(solver_), x(X), b(B),
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

        //    discrete... get the coeffs
        static coefflist cl = solver.mydiscrete.newcoefflist();
        solver.mydiscrete.coeffs(cl, solver.g, l, add, coord, i);

        defect -= cl.aii * solver.x[i];

        for (int n=0; n<cl.size; n++) {
          int j = cl.j[n];
          defect -= cl.aij[n] * solver.x[j];
        }
#ifndef NDEBUG
        if (!finite(defect))
          std::cerr << "DEFECT ERROR Element " << i
                    << coord << std::endl;
        assert(finite(defect));
#endif

        solver.d[i]=defect;

        return defect;
      }

    };

    template <class GRID, int SMOOTHER>
    class Restrict {
      typedef int level;
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
          coord_shift[d] = solver.g.has_coord_shift(l,d);
      }
      void evaluate(level l, const array<DIM> & coord, int i) {
        adddefect(solver.d[i], DIM, l, coord, coord_shift);
      }
      void adddefect(double d, int dir,
                     typename GRID::level l, array<DIM> coord,
                     array<DIM> coord_shift){
        dir--;

        if (dir<0) {
          int f = g.father_id(l,coord);
          solver.b[f] += d;
          return;
        }

        if (coord[dir]%2==coord_shift[dir]) {
          adddefect(d,dir,l,coord,coord_shift);
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
            adddefect(d/2.0,dir,l,shiftr,coord_shift);
          }
          if (! (!g.do_front_share(dir) &&
                 coord[dir]==0)) {
            adddefect(d/2.0,dir,l,shiftl,coord_shift);
          }
        }
      }
    };

    template <class GRID, int SMOOTHER>
    class Prolongate {
      typedef int level;
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
          coord_shift[d] = solver.g.has_coord_shift(l,d);
      }
      void evaluate(level l, const array<DIM> & coord, int i) {
        x[i] += correction(DIM, l, coord);
      }
      double correction(int dir, typename GRID::level l,
                        array<DIM> coord) {
        dir--;

        if (dir<0) {
          int f=g.father_id(l, coord);
          return x[f];
        }

        if (coord[dir]%2==coord_shift[dir])
          return correction(dir,l,coord);
        else {
          array<DIM> shiftl=coord;
          array<DIM> shiftr=coord;
          shiftl[dir]-=1;
          shiftr[dir]+=1;
          return 0.5*correction(dir,l,shiftl) +
                 0.5*correction(dir,l,shiftr);
        }
      }
    };

    template <class GRID, int SMOOTHER>
    class InitExchange {
      typedef int level;
      enum { DIM = GRID::griddim };
      pmgsolver<GRID,SMOOTHER> & solver;
    public:
      InitExchange(pmgsolver<GRID,SMOOTHER> & solver_) :
        solver(solver_) {}
      void evaluate(level l, const array<DIM> & coord, int id) {
        typename GRID::iterator it(id,solver.g);
        typename GRID::index i=*it;
        typename GRID::remotelist remote=i.remote();
        /* if i own the data, I'll search all processes
           to receive the data */
        if(i.owner()) {
          for (int r=0; r<remote.size; r++) {
            int & size = solver.exchange_data_to[l][remote.list[r].process()].size;
            realloc<int>(solver.exchange_data_to[l][remote.list[r].process()].id, size + 1);
            solver.exchange_data_to[l][remote.list[r].process()].id[size] = it.id();
            size ++;
          }
        }
        /* if I share the data, find the owner-processes */
        else {
          for (int r=0; r<remote.size; r++) {
            if (remote.list[r].owner()) {
              int & size = solver.exchange_data_from[l][remote.list[r].process()].size;
              realloc<int>(solver.exchange_data_from[l][remote.list[r].process()].id, size + 1);
              solver.exchange_data_from[l][remote.list[r].process()].id[size] = it.id();
              size ++;
              continue;
            }
          }
        }
      }
    };

  }   // PMGStubs
} // Dune
