// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_SOLVERS_HH__
#define __DUNE_SOLVERS_HH__

#include <math.h>
#include <complex>
#include <iostream>
#include <iomanip>
#include <string>
#include <stdio.h> // there is nothing better than printf

#include "istlexception.hh"
#include "operators.hh"
#include "preconditioners.hh"
#include "dune/common/timer.hh"

/*! \file __FILE__

    Define general, extensible interface for inverse operators.
        Implementation here coverse only inversion of linear operators,
        but the implementation might be used for nonlinear operators
    as well.
 */

namespace Dune {

  /** @addtogroup ISTL
          @{
   */


  /*! The return value of an application of the inverse
      operator delivers some important information about
          the iteration.
   */
  struct InverseOperatorResult
  {
    InverseOperatorResult ()
    {
      clear();
    }

    void clear ()
    {
      iterations = 0;
      reduction = 0;
      converged = false;
      conv_rate = 1;
      elapsed = 0;
    }

    int iterations;       // number of iterations
    double reduction;     // reduction achieved: \f$ \|b-A(x^n)\|/\|b-A(x^0)\|\f$
    bool converged;       // true if convergence criterion has been met
    double conv_rate;     // convergence rate (average reduction per step)
    double elapsed;       // elapsed time in seconds
  };


  //=====================================================================
  /*! An InverseOperator computes the solution of \f$ A(x)=b\f$ where
      \f$ A : X \to Y \f$ is an operator.
      Note that the solver "knows" which operator
      to invert and which preconditioner to apply (if any). The
      user is only interested in inverting the operator.
          InverseOperator might be a Newton scheme, a Krylov subspace method,
      or a direct solver or just anything.
   */
  //=====================================================================
  template<class X, class Y>
  class InverseOperator {
  public:
    //! export types, usually they come from the derived class
    typedef X domain_type;
    typedef Y range_type;
    typedef typename X::field_type field_type;

    //! apply inverse operator, Note: right hand side b may be overwritten!
    virtual void apply (X& x, Y& b, InverseOperatorResult& r) = 0;

    //! apply inverse operator, with given convergence criteria, right hand side b may be overwritten!
    virtual void apply (X& x, Y& b, double reduction, InverseOperatorResult& r) = 0;

    //! the usual thing
    virtual ~InverseOperator () {}
  };


  //=====================================================================
  // Implementation of this interface
  //=====================================================================

  /*! Implements a preconditioned loop.
          Verbose levels are:
          0 : print nothing
          1 : print initial and final defect and statistics
          2 : print line for each iteration
   */
  template<class X, class Y>
  class LoopSolver : public InverseOperator<X,Y> {
  public:
    //! export types, usually they come from the derived class
    typedef X domain_type;
    typedef Y range_type;
    typedef typename X::field_type field_type;

    //! set up Loop solver
    LoopSolver (LinearOperator<X,Y>& op, Preconditioner<X,Y>& prec,
                double reduction, int maxit, int verbose) :
      _op(op), _prec(prec), _reduction(reduction), _maxit(maxit), _verbose(verbose)
    {}

    //! apply inverse operator
    virtual void apply (X& x, Y& b, InverseOperatorResult& r)
    {
      // clear solver statistics
      r.clear();

      // start a timer
      Timer watch;

      // prepare preconditioner
      _prec.pre(x,b);

      // overwrite b with defect
      _op.applyscaleadd(-1,x,b);

      // compute norm, \todo parallelization
      double def0 = _prec.norm(b);

      // printing
      if (_verbose>0)
      {
        printf("=== LoopSolver\n");
        if (_verbose>1) printf(" Iter       Defect         Rate\n");
        if (_verbose>1) printf("%5d %12.4E\n",0,def0);
      }

      // allocate correction vector
      X v(x);

      // iteration loop
      int i=1; double def=def0;
      for ( ; i<=_maxit; i++ )
      {
        v = 0;                                // clear correction
        _prec.apply(v,b);                     // apply preconditioner
        x += v;                               // update solution
        _op.applyscaleadd(-1,v,b);            // update defect
        double defnew=_prec.norm(b);          // comp defect norm
        if (_verbose>1)                       // print
          printf("%5d %12.4E %12.4g\n",i,defnew,defnew/def);
        def = defnew;                         // update norm
        if (def<def0*_reduction)              // convergence check
        {
          r.converged  = true;
          break;
        }
      }

      // print
      if (_verbose==1)
        printf("%5d %12.4E\n",i,def);

      // postprocess preconditioner
      _prec.post(x);

      // fill statistics
      r.iterations = i;
      r.reduction = def/def0;
      r.conv_rate  = pow(r.reduction,1.0/i);
      r.elapsed = watch.elapsed();

      // final print
      if (_verbose>0)
        printf("=== rate=%g, T=%g, TIT=%g\n",r.conv_rate,r.elapsed,r.elapsed/i);
    }

    //! apply inverse operator, with given reduction factor
    virtual void apply (X& x, Y& b, double reduction, InverseOperatorResult& r)
    {
      _reduction = reduction;
      (*this).apply(x,b,r);
    }

  private:
    LinearOperator<X,Y>& _op;
    Preconditioner<X,Y>& _prec;
    double _reduction;
    int _maxit;
    int _verbose;
  };




  //! conjugate gradient method
  template<class X, class Y>
  class CGSolver : public InverseOperator<X,Y> {
  public:
    //! export types, usually they come from the derived class
    typedef X domain_type;
    typedef Y range_type;
    typedef typename X::field_type field_type;

    //! set up Loop solver
    CGSolver (LinearOperator<X,Y>& op, Preconditioner<X,Y>& prec,
              double reduction, int maxit, int verbose) :
      _op(op), _prec(prec), _reduction(reduction), _maxit(maxit), _verbose(verbose)
    {}

    //! apply inverse operator
    virtual void apply (X& x, Y& b, InverseOperatorResult& r)
    {
      r.clear();                      // clear solver statistics
      Timer watch;                    // start a timer
      _prec.pre(x,b);                 // prepare preconditioner
      _op.applyscaleadd(-1,x,b);      // overwrite b with defect

      X p(x);                         // create local vectors
      Y q(b);

      double def0 = _prec.norm(b);    // compute norm

      if (_verbose>0)                 // printing
      {
        printf("=== CGSolver\n");
        if (_verbose>1) printf(" Iter       Defect         Rate\n");
        if (_verbose>1) printf("%5d %12.4E\n",0,def0);
      }

      int i=1; double def=def0;       // loop variables
      field_type rho,rholast,lambda;
      for ( ; i<=_maxit; i++ )
      {
        p = 0;                                // clear correction
        _prec.apply(p,b);                     // apply preconditioner
        rho = _prec.dot(p,b);                 // orthogonalization
        if (i>1) p.axpy(rho/rholast,q);
        rholast = rho;                        // remember rho for recurrence
        _op.apply(p,q);                       // q=Ap
        lambda = rho/_prec.dot(q,p);          // minimization
        x.axpy(lambda,p);                     // update solution
        b.axpy(-lambda,q);                    // update defect
        q=p;                                  // remember search direction
        double defnew=_prec.norm(b);          // comp defect norm
        if (_verbose>1)                       // print
          printf("%5d %12.4E %12.4g\n",i,defnew,defnew/def);
        def = defnew;                         // update norm
        if (def<def0*_reduction)              // convergence check
        {
          r.converged  = true;
          break;
        }
      }

      if (_verbose==1)                    // printing for non verbose
        printf("%5d %12.4E\n",i,def);
      _prec.post(x);                      // postprocess preconditioner
      r.iterations = i;                   // fill statistics
      r.reduction = def/def0;
      r.conv_rate  = pow(r.reduction,1.0/i);
      r.elapsed = watch.elapsed();
      if (_verbose>0)                     // final print
        printf("=== rate=%g, T=%g, TIT=%g\n",r.conv_rate,r.elapsed,r.elapsed/i);
    }

    //! apply inverse operator, with given reduction factor
    virtual void apply (X& x, Y& b, double reduction, InverseOperatorResult& r)
    {
      _reduction = reduction;
      (*this).apply(x,b,r);
    }

  private:
    LinearOperator<X,Y>& _op;
    Preconditioner<X,Y>& _prec;
    double _reduction;
    int _maxit;
    int _verbose;
  };


  /** @} end documentation */

} // end namespace

#endif
