// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/*****************************************************************************
*  CG scheme
*****************************************************************************/
#ifndef __DUNE_CG_HH
#define __DUNE_CG_HH

#include <assert.h>

namespace Dune
{

#ifndef ABS
#define ABS(a) ((a > 0) ? a : -a )
#endif

  /* euklidean scalar product  */
  double SKP (double *vektor_1, double *vektor_2, int dim)
  {
    double skp= 0.0;
    for(int i=0; i<dim; i++)
      skp += vektor_1[i] * vektor_2[i];
    return skp;
  }

  /*  maximum norm  */
  double MaxNorm(double *vektor, int dim)
  {
    double max = 0.0;
    for (int i=0; i<dim; i++)
      if(ABS(vektor[i]) > max) max = ABS(vektor[i]);
    return max;
  }

  /* euclidean norm */
  double euklidNorm(double *x, double *y, int dim)
  {
    double norm = 0.0;
    for(int n=0; n<dim; n++)
      norm += (x[n] - y[n])* (x[n] - y[n]);
    return sqrt(norm);
  }

  //  the class Matrix only needs a method:
  //  which multiplies the vector vec with the matrix
  //
  //  void mult(double *return , const double * vec)
  //
  //  u_out is the start vector an return vector
  //  b right hand side
  //  eps is break criterion
  //  dim Dimension of rhs and Matrix
  //  brek = 3* dim max iterations
  template <class Matrix, class Vector>
  int CG(Matrix& matrix, Vector* u_out, Vector* b, double eps,int brek, int dim)
  {
    typedef typename Matrix::Ttype T;

    int iteration=0;
    double d_max = 0.0, tau = 1.0;
    double omega = 1.0, gamma  = 1.0;
    double alpha = 1.0, beta = 1.0;

    Vector* g = new Vector [dim];
    Vector* d_slash = new Vector [dim];
    Vector* d = new Vector [dim];
    /*  start  of cg alogrithm   */

    /*  u_out is the start vector an return vector */
    matrix.mult(g,u_out); /* g is changed here */


    for(int i=0; i<dim; i++)
    {
      g [i] -= b[i];
      d [i] = -g [i];
    }

    gamma = SKP ( g , g,dim );
    d_max = MaxNorm(g,dim);

    /* begin iteration   */
    while (eps < d_max )
    {
      d_max = MaxNorm(g,dim);

      /* only one matrix vector multipilcation per iteration step */
      matrix.mult(d_slash,d);

      omega = SKP(d , d_slash,dim);
      if (omega < eps)
      {
        d_max = omega;
        break;
      }

      tau = gamma / omega ;
      for (int i=0; i<dim; i++)
      {
        u_out[i] += tau * d[i];
        g[i] += tau * d_slash[i];
      }

      alpha = SKP (g,g,dim);
      beta  = alpha / gamma;
      gamma = alpha;

      for(int i=0; i<dim; i++)
      {
        d[i] *= beta;
        d[i] -= g[i];
      }

      iteration++;
      if (iteration > brek )
        break;
    } // end while

    // free memory
    delete g; delete d; delete d_slash;

    return iteration;
  }; /*  end CG scheme */


} // end namespace Dune

#endif
