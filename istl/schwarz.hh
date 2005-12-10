// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_SCHWARZ_HH
#define DUNE_SCHWARZ_HH

#include <iostream>               // for input/output to shell
#include <fstream>                // for input/output to files
#include <vector>                 // STL vector class
#include <sstream>

#include <math.h>                 // Yes, we do some math here
#include <stdio.h>                // There is nothing better than sprintf
#include <sys/times.h>           // for timing measurements

#include "dune/common/timer.hh"

#include "io.hh"
#include "bvector.hh"
#include "vbvector.hh"
#include "bcrsmatrix.hh"
#include "io.hh"
#include "gsetc.hh"
#include "ilu.hh"
#include "operators.hh"
#include "solvers.hh"
#include "preconditioners.hh"
#include "scalarproducts.hh"

namespace Dune {

  /**
     @addtogroup ISTL
     @{
   */

  /**
   * @brief Categories for the solvers.
   */

  template<class M, class X, class Y, class C>
  class OverlappingSchwarzOperator : public AssembledLinearOperator<M,X,Y>
  {
  public:
    //! export types
    typedef M matrix_type;
    typedef X domain_type;
    typedef Y range_type;
    typedef typename X::field_type field_type;

    //! define the category
    enum {category=SolverCategory::overlapping};

    //! constructor: just store a reference to a matrix
    OverlappingSchwarzOperator (const M& A, const C& com)
      : _A_(A), communication(com)
    {}

    //! apply operator to x:  \f$ y = A(x) \f$
    virtual void apply (const X& x, Y& y) const
    {
      y = 0;
      _A_.umv(x,y);     // result is consistent on interior+border
      communication.project(y);
    }

    //! apply operator to x, scale and add:  \f$ y = y + \alpha A(x) \f$
    virtual void applyscaleadd (field_type alpha, const X& x, Y& y) const
    {
      _A_.usmv(alpha,x,y);     // result is consistent on interior+border
      communication.project(y);
    }

    //! get matrix via *
    virtual const M& getmat () const
    {
      return _A_;
    }

  private:
    const M& _A_;
    const C& communication;
  };



  //! Scalar product assuming consistent vectors in interior+border
  template<class X, class C>
  class OverlappingSchwarzScalarProduct : public ScalarProduct<X>
  {
  public:
    //! export types
    typedef X domain_type;
    typedef typename X::field_type field_type;

    //! define the category
    enum {category=SolverCategory::overlapping};

    /*! \brief Constructor needs to know the grid
     */
    OverlappingSchwarzScalarProduct (const C& com)
      : communication(com)
    {}

    /*! \brief Dot product of two vectors.
       It is assumed that the vectors are consistent on the interior+border
       partition.
     */
    virtual field_type dot (const X& x, const X& y)
    {
      field_type result;
      communication.dot(x,y,result);
      return result;
    }

    /*! \brief Norm of a right-hand side vector.
       The vector must be consistent on the interior+border partition
     */
    virtual double norm (const X& x)
    {
      return communication.norm(x);
    }

  private:
    const C& communication;
  };





  template<class M, class X, class Y, class C>
  class ParSSOR : public Preconditioner<X,Y> {
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
      category=SolverCategory::overlapping
    };

    /*! \brief Constructor.

       constructor gets all parameters to operate the prec.
       \param A The matrix to operate on.
       \param n The number of iterations to perform.
       \param w The relaxation factor.
     */
    ParSSOR (const M& A, int n, field_type w, const C& c)
      : _A_(A), _n(n), _w(w), communication(c)
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
      communication.copyOwnerToAll(v,v);
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
    //! \brief the communication object
    const C& communication;
  };

  /** @} end documentation */

} // end namespace

#endif
