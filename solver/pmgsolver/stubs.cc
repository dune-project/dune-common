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
      double * x;
      double * b;
      GRID & g;
      coefflist & cl;
      discrete<GRID> &mydiscrete;
      array<DIM> add;
    public:
      GaussSeidel(double * _x, double * _b, GRID & _g,
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
      double * x;
      double * b;
      GRID & g;
      coefflist & cl;
      discrete<GRID> &mydiscrete;
      array<DIM> add;
    public:
      GaussSeidelBorder(double * _x, double * _b, GRID & _g,
                        int l, coefflist & _cl, discrete<GRID> &d) :
        x(_x), b(_b), g(_g), cl(_cl), mydiscrete(d), add(0) {
        add = g.init_add(l);
      }
      void evaluate(int l, const array<DIM> & coord, int i) {
        /* Coeffs */
        /*
           coefflist & coeffs(coefflist &cl,
           mygrid &g, typename mygrid::level l,
           const array<DIM> &coord, int id_i) const
         */
        mydiscrete.coeffs(cl, g, l, add, coord, i);

        x[i]=b[i];
        for (int n=0; n<cl.size; n++) {
          int j = cl.j[n];
          x[i] -= cl.aij[n] * x[j];
        }
        x[i] /= cl.aii;
      }
    };

  } // PMGStubs
} // Dune
