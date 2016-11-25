// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_FUNCTION_HH
#define DUNE_FUNCTION_HH


#include "typetraits.hh"

namespace Dune {

  /** @addtogroup Common
     @{
   */

  /*! \file
      \brief Simple base class templates for functions.
   */

  /**
   * \brief Base class template for function classes
   *
   * \tparam Domain Type of input variable. This could be some 'const T' or 'const T&'.
   *
   * \tparam Range Type of output variable. This should be some non-const 'T&' to allow to return results.
   */
  template <class Domain, class Range>
  class Function
  {
    typedef typename std::remove_cv<typename std::remove_reference< Domain >::type >::type RawDomainType;
    typedef typename std::remove_cv<typename std::remove_reference< Range >::type >::type RawRangeType;

  public:

    //! Raw type of input variable with removed reference and constness
    typedef RawRangeType RangeType;

    //! Raw type of output variable with removed reference and constness
    typedef RawDomainType DomainType;

    //! Traits class containing raw types
    struct Traits
    {
      typedef RawDomainType DomainType;
      typedef RawRangeType RangeType;
    };

    /**
     * \brief Function evaluation.
     *
     * \param x Argument for function evaluation.
     * \param y Result of function evaluation.
     */
    void evaluate(const typename Traits::DomainType& x, typename Traits::RangeType& y) const;
  }; // end of Function class



  /**
   * \brief Virtual base class template for function classes.
   *
   * \tparam DomainType The type of the input variable is 'const DomainType &'
   *
   * \tparam RangeType The type of the output variable is 'RangeType &'
   */
  template <class DomainType, class RangeType>
  class VirtualFunction :
    public Function<const DomainType&, RangeType&>
  {
  public:
    typedef typename Function<const DomainType&, RangeType&>::Traits Traits;

    virtual ~VirtualFunction() {}
    /**
     * \brief Function evaluation.
     *
     * \param x Argument for function evaluation.
     * \param y Result of function evaluation.
     */
    virtual void evaluate(const typename Traits::DomainType& x, typename Traits::RangeType& y) const = 0;
  }; // end of VirtualFunction class

  /** @} end documentation */

} // end namespace

#endif
