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
  template< class FunctionSpaceType, class FunctionImp>
  class Function : public Mapping < typename FunctionSpaceType::DomainField,
                       typename FunctionSpaceType::RangeField , typename FunctionSpaceType::Domain, typename FunctionSpaceType::Range > {

  public:
    //! ???
    typedef typename FunctionSpaceType::Domain Domain ;
    //! ???
    typedef typename FunctionSpaceType::Range Range ;
    //! ???
    typedef typename FunctionSpaceType::JacobianRange JacobianRange;
    //! ???
    typedef typename FunctionSpaceType::HessianRange HessianRange;
    //! ???
    typedef FunctionSpaceType FunctionSpace;

    //! ???
    Function ( FunctionSpaceType & f ) : functionSpace_ (f) {} ;

    //! evaluate Function
    void eval ( const Domain & , Range &) const ;

    //! evaluate function and derivatives
    template <int derivation>

    //! ???
    void evaluate  ( const FieldVector<deriType, derivation> &diffVariable,
                     const Domain & , Range &) const {};

    //! ???
    FunctionSpaceType &getFunctionSpace() const { return functionSpace_; }

  protected:

    //! ???
    FunctionSpaceType & functionSpace_;

  };

  /** @} end documentation group */

}

#endif
