// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ISTLOPERATORS_HH__
#define __DUNE_ISTLOPERATORS_HH__

#include <math.h>
#include <complex>
#include <iostream>
#include <iomanip>
#include <string>


/** \file

   \brief Define general, extensible interface for operators.
        The available implementation wraps a matrix.
 */

namespace Dune {

  /** @addtogroup ISTL
          @{
   */


  //=====================================================================
  // Abstract operator interface
  //=====================================================================

  /*! Abstract base class defining an operator \f$ A : X\to Y\f$. The
     simplest solvers just need the application  \f$ A(x)\f$ of
     the operator. The operator might even be nonlinear (but is not in
     our application here).

        - enables on the fly computation through operator concept. If explicit
        representation of the operator is required use AssembledLinearOperator

        - Some inverters may need an explicit formation of the operator
        as a matrix, e.g. BiCGStab, ILU, AMG, etc. In that case use the
        derived class
   */
  template<class X, class Y>
  class Operator {
  public:
    //! export types, they come from the derived class
    typedef X domain_type;
    typedef Y range_type;
    typedef typename X::field_type field_type;

    //! apply operator to x:  \f$ y = A(x) \f$
    virtual void apply (const X& x, Y& y) const = 0;

    //! every abstract base class has a virtual destructor
    virtual ~Operator () {}
  };


  /*! This type ensures that the operator \f$ A \f$ is a
      linear operator, i.e. \f$ A(\alpha x) = \alpha A(x) \f$ and
      \f$ A(x+y) = A(x)+A(y)\f$. The additional interface function
      reflects this fact.
   */
  template<class X, class Y>
  class LinearOperator : public Operator<X,Y> {
  public:
    //! export types, they come from the derived class
    typedef X domain_type;
    typedef Y range_type;
    typedef typename X::field_type field_type;

    //! apply operator to x, scale and add:  \f$ y = y + \alpha A(x) \f$
    virtual void applyscaleadd (field_type alpha, const X& x, Y& y) const = 0;

    //! every abstract base class has a virtual destructor
    virtual ~LinearOperator () {}
  };


  /*! Linear Operator that exports the operator in
     matrix form. This is needed for certain solvers, such as
     LU decomposition, ILU preconditioners or BiCG-Stab (because
     of multiplication with A^T).
   */
  template<class M, class X, class Y>
  class AssembledLinearOperator : public LinearOperator<X,Y> {
  public:
    //! export types, usually they come from the derived class
    typedef M matrix_type;
    typedef X domain_type;
    typedef Y range_type;
    typedef typename X::field_type field_type;

    //! get matrix via *
    virtual const M& getmat () const = 0;

    //! every abstract base class has a virtual destructor
    virtual ~AssembledLinearOperator () {}
  };



  //=====================================================================
  // Implementation for ISTL-matrix based operator
  //=====================================================================

  //! Adapts a matrix to the assembled linear operator interface
  template<class M, class X, class Y>
  class MatrixAdapter : public AssembledLinearOperator<M,X,Y>
  {
  public:
    //! export types
    typedef M matrix_type;
    typedef X domain_type;
    typedef Y range_type;
    typedef typename X::field_type field_type;

    //! constructor: just store a reference to a matrix
    MatrixAdapter (const M& A) : _A(A) {}

    //! apply operator to x:  \f$ y = A(x) \f$
    virtual void apply (const X& x, Y& y) const
    {
      y = 0;
      _A.umv(x,y);
    }

    //! apply operator to x, scale and add:  \f$ y = y + \alpha A(x) \f$
    virtual void applyscaleadd (field_type alpha, const X& x, Y& y) const
    {
      _A.usmv(alpha,x,y);
    }

    //! get matrix via *
    virtual const M& getmat () const
    {
      return _A;
    }

  private:
    const M& _A;
  };

  /** @} end documentation */

} // end namespace

#endif
