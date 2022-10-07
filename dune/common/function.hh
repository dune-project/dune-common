// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_FUNCTION_HH_SILENCE_DEPRECATION
#warning This file is deprecated after Dune 2.7!  Use C++ function objects and std::function stuff instead!
#else // !DUNE_FUNCTION_HH_SILENCE_DEPRECATION
#undef DUNE_FUNCTION_HH_SILENCE_DEPRECATION
#endif // !DUNE_FUNCTION_HH_SILENCE_DEPRECATION

#ifndef DUNE_FUNCTION_HH
#define DUNE_FUNCTION_HH

#include <utility>

#include <dune/common/deprecated.hh>
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
  class
  [[deprecated("Dune::Function is deprecated after Dune 2.7.  Use C++ "
               "function objects instead!")]]
  Function
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



  DUNE_NO_DEPRECATED_BEGIN
  /**
   * \brief Virtual base class template for function classes.
   *
   * \see makeVirtualFunction for a helper to convert lambda functions to
   * `VirtualFunction` objects.
   *
   * \tparam DomainType The type of the input variable is 'const DomainType &'
   *
   * \tparam RangeType The type of the output variable is 'RangeType &'
   */
  template <class DomainType, class RangeType>
  class
  [[deprecated("Dune::VirtualFunction is deprecated after Dune 2.7.  Use C++ "
               "function objects and std::function instead!")]]
  VirtualFunction : public Function<const DomainType&, RangeType&>
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
  DUNE_NO_DEPRECATED_END

  namespace Impl {

  DUNE_NO_DEPRECATED_BEGIN
  template<typename Domain, typename Range, typename F>
  class LambdaVirtualFunction final
    : public VirtualFunction<Domain, Range>
  {
  public:
    LambdaVirtualFunction(F&& f)
      : f_(std::move(f))
      {}

    LambdaVirtualFunction(const F& f)
      : f_(f)
      {}

    void evaluate(const Domain& x, Range& y) const override
      {
        y = f_(x);
      }

  private:
    const F f_;
  };
  DUNE_NO_DEPRECATED_END

  } /* namespace Impl */

  /**
   * \brief make `VirtualFunction` out of a function object
   *
   * This helper function wraps a function object into a class implementing
   * the `VirtualFunction` interface.  It allows for easy use of lambda
   * expressions in places that expect a `VirtualFunction`:
     \code
     void doSomething(const VirtualFunction<double, double>& f);

     auto f = makeVirtualFunction<double, double>(
         [](double x) { return x*x; });
     doSomething(f);
     \endcode
   *
   * \returns object of a class derived from `VirtualFunction<Domain, Range>`
   *
   * \tparam Domain domain of the function
   * \tparam Range  range of the function
   */
  template<typename Domain, typename Range, typename F>
  [[deprecated("Dune::LambdaVirtualFunction is deprecated after Dune 2.7.  "
               "Use std::function instead!")]]
  Impl::LambdaVirtualFunction< Domain, Range, std::decay_t<F> >
  makeVirtualFunction(F&& f)
  {
    return {std::forward<F>(f)};
  }

  /** @} end documentation */

} // end namespace

#endif
