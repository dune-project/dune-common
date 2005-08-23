// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_FUNCTION_HH__
#define __DUNE_FUNCTION_HH__

#include <dune/common/mapping.hh>
#include <dune/common/fvector.hh>

namespace Dune {
  /** @defgroup FunctionCommon Functions
      @ingroup AlgebraCommon
      Functions are Mappings from \f$K^n\f$ into \f$L^m\f$ where
      \f$K\f$ and \f$L\f$ are fields.
   */

  /** @defgroup Function Function
     \ingroup FunctionCommon

     @{
   */


  typedef int deriType;

  /** \brief Class representing a function
   * \todo Please doc me!
   */
  template< class FunctionSpaceImp, class FunctionImp>
  class Function :
    public Mapping < typename FunctionSpaceImp::DomainFieldType,
        typename FunctionSpaceImp::RangeFieldType ,
        typename FunctionSpaceImp::DomainType,
        typename FunctionSpaceImp::RangeType > {

  public:
    typedef FunctionSpaceImp FunctionSpaceType;
    //! ???
    typedef typename FunctionSpaceType::DomainType DomainType ;
    //! ???
    typedef typename FunctionSpaceType::RangeType RangeType ;
    //! ???
    typedef typename FunctionSpaceType::JacobianRangeType JacobianRangeType;
    //! ???
    typedef typename FunctionSpaceType::HessianRangeType HessianRangeType;

    //! Constructor
    Function (const FunctionSpaceType & f) : functionSpace_ (f) {} ;

    //! application operator
    virtual void operator()(const DomainType & arg, RangeType & dest) const {
      eval(arg,dest);
    }

    //! evaluate Function
    void eval(const DomainType & arg, RangeType & dest) const {
      asImp().eval(arg, dest);
    }

    //! evaluate function and derivatives
    template <int derivation>
    void evaluate  ( const FieldVector<deriType, derivation> &diffVariable,
                     const DomainType& arg, RangeType & dest) const {
      asImp().evaluate(diffVariable, arg, dest);
    }

    //! Get access to the related function space
    const FunctionSpaceType& getFunctionSpace() const { return functionSpace_; }

  protected:
    //! Barton-Nackman trick
    FunctionImp& asImp() {
      return static_cast<FunctionImp&>(*this);
    }
    const FunctionImp& asImp() const {
      return static_cast<const FunctionImp&>(*this);
    }

    //! The related function space
    const FunctionSpaceType & functionSpace_;
  private:
    //! Helper function for Mapping
    //! With this function, a combined mapping can choose the right application
    //! operator (i.e. the one from Mapping itself, or from Function/Operator)
    //! \note: Do not override this definition
    virtual void apply (const DomainType& arg, RangeType& dest) const {
      operator()(arg, dest);
    }
  };

  /** @} end documentation group */

}

#endif
