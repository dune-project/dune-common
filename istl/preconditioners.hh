// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_PRECONDITIONERS_HH
#define DUNE_PRECONDITIONERS_HH

#include <math.h>
#include <complex>
#include <iostream>
#include <iomanip>
#include <string>

#include "solvercategory.hh"
#include "istlexception.hh"
#include "io.hh"
#include "gsetc.hh"
#include "ilu.hh"


namespace Dune {

  /** @addtogroup ISTL
          @{
   */
  /** \file

     \brief    Define general preconditioner interface

     Wrap the methods implemented by ISTL in this interface.
     However, the interface is extensible such that new preconditioners
     can be implemented and used with the solvers.
   */


  //=====================================================================
  /*! \brief Base class for matrix free definition of preconditioners.

          Note that the operator, which is the basis for the preconditioning,
      is supplied to the preconditioner from the outside in the
      constructor or some other method.

          This interface allows the encapsulation of all parallelization
          aspects into the preconditioners.
   */
  //=====================================================================
  template<class X, class Y>
  class Preconditioner {
  public:
    //! \brief The domain type of the preconditioner.
    typedef X domain_type;
    //! \brief The range type of the preconditioner.
    typedef Y range_type;
    //! \brief The field type of the preconditioner.
    typedef typename X::field_type field_type;

    /*! \brief Prepare the preconditioner.

       A solver solves a linear operator equation A(x)=b by applying
       one or several steps of the preconditioner. The method pre()
       is called before before the first apply operation.
       x and b are right hand side and solution vector of the linear
       system. It may. e.g., scale the system, allocate memory or
       compute a (I)LU decomposition.
       Note: The ILU decomposition could also be computed in the constructor
       or with a seperate method of the derived method if several
       linear systems with the same matrix are to be solved.

       \param x The left hand side of the equation.
       \param b The right hand side of the equation.
     */
    virtual void pre (X& x, Y& b) = 0;

    /*! \brief Apply one step of the preconditioner to the system A(v)=d.

       On entry v=0 and d=b-A(x) (although this might not be
       computed in that way. On exit v contains the update, i.e
       one step computes \f$ v = M^{-1} d \f$ where \f$ M \f$ is the
       approximate inverse of the operator \f$ A \f$ characterizing
       the preconditioner.
       \param[out] v The update to be computed
       \param d The current defect.
     */
    virtual void apply (X& v, const Y& d) = 0;

    /*! \brief Clean up.

       This method is called after the last apply call for the
       linear system to be solved. Memory may be deallocated safely
       here. x is the solution of the linear equation.

       \param x The right hand side of the equation.
     */
    virtual void post (X& x) = 0;

    // every abstract base class has a virtual destructor
    virtual ~Preconditioner () {}
  };


  //=====================================================================
  // Implementation of this interface for sequential ISTL-preconditioners
  //=====================================================================


  /*!
     \brief Sequential SSOR preconditioner.

     Wraps the naked ISTL generic SOR preconditioner into the
      solver framework.
   */
  template<class M, class X, class Y>
  class SeqSSOR : public Preconditioner<X,Y> {
  public:
    //! \brief The matrix type the preconditioner is for.
    typedef M matrix_type;
    //! \brief The domain type of the preconditioner.
    typedef X domain_type;
    //! \brief The range type of the preconditioner.
    typedef Y range_type;
    //! \brief The field type of the preconditioner.
    typedef typename X::field_type field_type;

    // define the category
    enum {
      //! \brief The category the precondtioner is part of.
      category=SolverCategory::sequential
    };

    /*! \brief Constructor.

       constructor gets all parameters to operate the prec.
       \param A The matrix to operate on.
       \param n The number of iterations to perform.
       \param w The relaxation factor.
     */
    SeqSSOR (const M& A, int n, field_type w)
      : _A_(A), _n(n), _w(w)
    {   }

    /*!
       \brief Prepare the preconditioner.

       \copydoc Preconditioner::pre(X&,Y&)
     */
    virtual void pre (X& x, Y& b) {}

    /*!
       \brief Apply the precondtioner

       \copydoc Preconditioner::apply(X&,Y&)
     */
    virtual void apply (X& v, const Y& d)
    {
      for (int i=0; i<_n; i++) {
        bsorf(_A_,v,d,_w);
        bsorb(_A_,v,d,_w);
      }
    }

    /*!
       \brief Clean up.

       \copydoc Preconditioner::post(X&)
     */
    virtual void post (X& x) {}

  private:
    //! \brief The matrix we operate on.
    const M& _A_;
    //! \brief The number of steps to do in apply
    int _n;
    //! \brief The relaxation factor to use
    field_type _w;
  };


  /*!
     \brief Sequential SOR preconditioner.

     Wraps the naked ISTL generic SOR preconditioner into the
     solver framework.
   */
  template<class M, class X, class Y>
  class SeqSOR : public Preconditioner<X,Y> {
  public:
    //! \brief The matrix type the preconditioner is for.
    typedef M matrix_type;
    //! \brief The domain type of the preconditioner.
    typedef X domain_type;
    //! \brief The range type of the preconditioner.
    typedef Y range_type;
    //! \brief The field type of the preconditioner.
    typedef typename X::field_type field_type;

    // define the category
    enum {
      //! \brief The category the preconditioner is part of.
      category=SolverCategory::sequential
    };

    /*! \brief Constructor.

       constructor gets all parameters to operate the prec.
       \param A The matrix to operate on.
       \param n The number of iterations to perform.
       \param w The relaxation factor.
     */
    SeqSOR (const M& A, int n, field_type w)
      : _A_(A), _n(n), _w(w)
    {   }

    /*!
       \brief Prepare the preconditioner.

       \copydoc Preconditioner::pre(X&,Y&)
     */
    virtual void pre (X& x, Y& b) {}

    /*!
       \brief Apply the preconditioner.

       \copydoc Preconditioner::apply(X&,Y&)
     */
    virtual void apply (X& v, const Y& d)
    {
      for (int i=0; i<_n; i++) {
        bsorf(_A_,v,d,_w);
      }
    }

    /*!
       \brief Clean up.

       \copydoc Preconditioner::post(X&)
     */
    virtual void post (X& x) {}

  private:
    //! \brief the matrix we operate on.
    const M& _A_;
    //! \brief The number of steps to perform in apply.
    int _n;
    //! \brief The relaxation factor to use.
    field_type _w;
  };


  /*! \brief Sequential Gauss Seidel preconditioner

     Wraps the naked ISTL generic block Gauss-Seidel preconditioner into the
      solver framework.
   */
  template<class M, class X, class Y>
  class SeqGS : public Preconditioner<X,Y> {
  public:
    //! \brief The matrix type the preconditioner is for.
    typedef M matrix_type;
    //! \brief The domain type of the preconditioner.
    typedef X domain_type;
    //! \brief The range type of the preconditioner.
    typedef Y range_type;
    //! \brief The field type of the preconditioner.
    typedef typename X::field_type field_type;

    // define the category
    enum {
      //! \brief The category the preconditioner is part of.
      category=SolverCategory::sequential
    };

    /*! \brief Constructor.

       Constructor gets all parameters to operate the prec.
       \param A The matrix to operate on.
       \param n The number of iterations to perform.
       \param w The relaxation factor.
     */
    SeqGS (const M& A, int n, field_type w)
      : _A_(A), _n(n), _w(w)
    {   }

    /*!
       \brief Prepare the preconditioner.

       \copydoc Preconditioner::pre(X&,Y&)
     */
    virtual void pre (X& x, Y& b) {}

    /*!
       \brief Apply the preconditioner.

       \copydoc Preconditioner::apply(X&,Y&)
     */
    virtual void apply (X& v, const Y& d)
    {
      for (int i=0; i<_n; i++) {
        dbgs(_A_,v,d,_w);
      }
    }

    /*!
       \brief Clean up.

       \copydoc Preconditioner::post(X&)
     */
    virtual void post (X& x) {}

  private:
    //! \brief The matrix we operate on.
    const M& _A_;
    //! \brief The number of iterations to perform in apply.
    int _n;
    //! \brief The relaxation factor to use.
    field_type _w;
  };


  /*! \brief The sequential jacobian preconditioner.

     Wraps the naked ISTL generic block Jacobi preconditioner into the
      solver framework.
   */
  template<class M, class X, class Y>
  class SeqJac : public Preconditioner<X,Y> {
  public:
    //! \brief The matrix type the preconditioner is for.
    typedef M matrix_type;
    //! \brief The domain type of the preconditioner.
    typedef X domain_type;
    //! \brief The range type of the preconditioner.
    typedef Y range_type;
    //! \brief The field type of the preconditioner.
    typedef typename X::field_type field_type;

    // define the category
    enum {
      //! \brief The category the preconditioner is part of
      category=SolverCategory::sequential
    };

    /*! \brief Constructor.

       Constructor gets all parameters to operate the prec.
       \param A The matrix to operate on.
       \param n The number of iterations to perform.
       \param w The relaxation factor.
     */
    SeqJac (const M& A, int n, field_type w)
      : _A_(A), _n(n), _w(w)
    {   }

    /*!
       \brief Prepare the preconditioner.

       \copydoc Preconditioner::pre(X&,Y&)
     */
    virtual void pre (X& x, Y& b) {}

    /*!
       \brief Apply the preconditioner.

       \copydoc Preconditioner::apply(X&,Y&)
     */
    virtual void apply (X& v, const Y& d)
    {
      for (int i=0; i<_n; i++) {
        dbjac(_A_,v,d,_w);
      }
    }

    /*!
       \brief Clean up.

       \copydoc Preconditioner::post(X&)
     */
    virtual void post (X& x) {}

  private:
    //! \brief The matrix we operate on.
    const M& _A_;
    //! \brief The number of steps to perform during apply.
    int _n;
    //! \brief The relaxation parameter to use.
    field_type _w;
  };



  /*!
     \brief Sequential ILU0 preconditioner.

     Wraps the naked ISTL generic ILU0 preconditioner into the
      solver framework.
   */
  template<class M, class X, class Y>
  class SeqILU0 : public Preconditioner<X,Y> {
  public:
    //! \brief The matrix type the preconditioner is for.
    typedef M matrix_type;
    //! \brief The domain type of the preconditioner.
    typedef X domain_type;
    //! \brief The range type of the preconditioner.
    typedef Y range_type;
    //! \brief The field type of the preconditioner.
    typedef typename X::field_type field_type;

    // define the category
    enum {
      //! \brief The category the preconditioner is part of.
      category=SolverCategory::sequential
    };

    /*! \brief Constructor.

       Constructor gets all parameters to operate the prec.
       \param A The matrix to operate on.
       \param w The relaxation factor.
     */
    SeqILU0 (const M& A, field_type w)
      : ILU(A) // copy A
    {
      _w =w;
      bilu0_decomposition(ILU);
    }

    /*!
       \brief Prepare the preconditioner.

       \copydoc Preconditioner::pre(X&,Y&)
     */
    virtual void pre (X& x, Y& b) {}

    /*!
       \brief Apply the preconditoner.

       \copydoc Preconditioner::apply(X&,Y&)
     */
    virtual void apply (X& v, const Y& d)
    {
      bilu_backsolve(ILU,v,d);
      v *= _w;
    }

    /*!
       \brief Clean up.

       \copydoc Preconditioner::post(X&)
     */
    virtual void post (X& x) {}

  private:
    //! \brief The relaxation factor to use.
    field_type _w;
    //! \brief The ILU0 decomposition of the matrix.
    M ILU;
  };


  /*!
     \brief Sequential ILU(n) preconditioner.

     Wraps the naked ISTL generic ILU(n) preconditioner into the
     solver framework.
   */
  template<class M, class X, class Y>
  class SeqILUn : public Preconditioner<X,Y> {
  public:
    //! \brief The matrix type the preconditioner is for.
    typedef M matrix_type;
    //! \brief The domain type of the preconditioner.
    typedef X domain_type;
    //! \brief The range type of the preconditioner.
    typedef Y range_type;
    //! \brief The field type of the preconditioner.
    typedef typename X::field_type field_type;

    // define the category
    enum {
      //! \brief The category the preconditioner is part of.
      category=SolverCategory::sequential
    };

    /*! \brief Constructor.

       Constructor gets all parameters to operate the prec.
       \param A The matrix to operate on.
       \param n The number of iterations to perform.
       \param w The relaxation factor.
     */
    SeqILUn (const M& A, int n, field_type w)
      : ILU(A.N(),A.M(),M::row_wise)
    {
      _n = n;
      _w = w;
      bilu_decomposition(A,n,ILU);
    }

    /*!
       \brief Prepare the preconditioner.

       \copydoc Preconditioner::pre(X&,Y&)
     */
    virtual void pre (X& x, Y& b) {}

    /*!
       \brief Apply the precondioner.

       \copydoc Preconditioner::apply(X&,Y&)
     */
    virtual void apply (X& v, const Y& d)
    {
      bilu_backsolve(ILU,v,d);
      v *= _w;
    }

    /*!
       \brief Clean up.

       \copydoc Preconditioner::post(X&)
     */
    virtual void post (X& x) {}

  private:
    //! \brief ILU(n) decomposition of the matrix we operate on.
    M ILU;
    //! \brief The number of steps to perform in apply.
    int _n;
    //! \brief The relaxation factor to use.
    field_type _w;
  };



  /** @} end documentation */

} // end namespace

#endif
