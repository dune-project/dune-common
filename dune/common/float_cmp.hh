// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_FLOAT_CMP_HH
#define DUNE_COMMON_FLOAT_CMP_HH

/** \file
 * \brief Various ways to compare floating-point numbers
 */

/**
   @addtogroup FloatCmp

   @section How_to_compare How to compare floats

   When comparing floating point numbers for equality, one often faces the
   problem that floating point operations are not always exact.  For example on
   i386 the expression
   @code
    0.2 + 0.2 + 0.2 + 0.2 + 0.2 + 0.2 + 0.2 + 0.2 + 0.2 + 0.2 == 2.0
   @endcode
   evaluates to
   @code
    1.99999999999999977796 == 2.00000000000000000000
   @endcode
   which is false.  One solution is to compare approximately, using an epsilon
   which says how much deviation to accept.

   The most straightforward way of comparing is using an @em absolute epsilon.
   This means comparison for equality is replaced by
   @code
    abs(first-second) <= epsilon
   @endcode
   This has a severe disadvantage: if you have an epsilon like 1e-10 but first
   and second are of the magnitude 1e-15 everything will compare equal which is
   certainly not what you want.  This can be overcome by selecting an
   appropriate epsilon.  Nevertheless this method of comparing is not
   recommended in general, and we will present a more robus method in the
   next paragraph.

   There is another way of comparing approximately, using a @em relative
   epsilon which is then scaled with first:
   @code
    abs(first-second) <= epsilon * abs(first)
   @endcode
   Of course the comparison should be symmetric in first and second so we
   cannot arbitrarily select either first or second to scale epsilon.  The are
   two symmetric variants, @em relative_weak
   @code
    abs(first-second) <= epsilon * max(abs(first), abs(second))
   @endcode
   and @em relative_strong
   @code
    abs(first-second) <= epsilon * min(abs(first), abs(second))
   @endcode
   Both variants are good, but in practice the relative_weak variant is
   preferred.  This is also the default variant.

   \note Although using a relative epsilon is better than using an absolute
        epsilon, using a relative epsilon leads to problems if either first or
        second equals 0.  In principle the relative method can be combined
        with an absolute method using an epsilon near the minimum
        representable positive value, but this is not implemented here.

   There is a completely different way of comparing floats.  Instead of giving
   an epsilon, the programmer states how many representable value are allowed
   between first and second.  See the "Comparing using integers" section in
   http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
   for more about that.

   @section Interface Interface

   To do the comparison, you can use the free functions @link
   Dune::FloatCmp::eq eq()@endlink, @link Dune::FloatCmp::ne ne()@endlink,
   @link Dune::FloatCmp::gt gt()@endlink, @link Dune::FloatCmp::lt
   lt()@endlink, @link Dune::FloatCmp::ge ge()@endlink and @link
   Dune::FloatCmp::le le()@endlink from the namespace Dune::FloatCmp.  They
   take the values to compare and optionally an epsilon, which defaults to 8
   times the machine epsilon (the difference between 1.0 and the smallest
   representable value > 1.0) for relative comparisons, or simply 1e-6 for
   absolute comparisons.  The compare style can be given as an optional second
   template parameter and defaults to relative_weak.

   You can also use the class Dune::FloatCmpOps which has @link
   Dune::FloatCmpOps::eq eq()@endlink, @link Dune::FloatCmpOps::ne
   ne()@endlink, @link Dune::FloatCmpOps::gt gt()@endlink, @link
   Dune::FloatCmpOps::lt lt()@endlink, @link Dune::FloatCmpOps::ge ge()@endlink
   and @link Dune::FloatCmpOps::le le()@endlink as member functions.  In this
   case the class encapsulates the epsilon and the comparison style (again the
   defaults from the previous paragraph apply).  This may be more convenient if
   you write your own class utilizing floating point comparisons, and you want
   the user of you class to specify epsilon and compare style.
 */

//! Dune namespace
namespace Dune {
  //! FloatCmp namespace
  //! @ingroup FloatCmp
  namespace FloatCmp {
    // basic constants
    //! How to compare
    //! @ingroup FloatCmp
    enum CmpStyle {
      //! |a-b|/|a| <= epsilon || |a-b|/|b| <= epsilon
      relativeWeak,
      //! |a-b|/|a| <= epsilon && |a-b|/|b| <= epsilon
      relativeStrong,
      //! |a-b| <= epsilon
      absolute,
      //! the global default compare style (relative_weak)
      defaultCmpStyle = relativeWeak
    };
    //! How to round or truncate
    //! @ingroup FloatCmp
    enum RoundingStyle {
      //! always round toward 0
      towardZero,
      //! always round away from 0
      towardInf,
      //! round toward \f$-\infty\f$
      downward,
      //! round toward \f$+\infty\f$
      upward,
      //! the global default rounding style (toward_zero)
      defaultRoundingStyle = towardZero
    };

    template<class T> struct EpsilonType;

    //! mapping from a value type and a compare style to a default epsilon
    /**
     * @ingroup FloatCmp
     * @tparam T     The value type to map from
     * @tparam style The compare style to map from
     */
    template<class T, CmpStyle style = defaultCmpStyle>
    struct DefaultEpsilon {
      //! Returns the default epsilon for the given value type and compare style
      static typename EpsilonType<T>::Type value();
    };

    // operations in functional style

    //! @addtogroup FloatCmp
    //! @{

    //! test for equality using epsilon
    /**
     * @tparam T     Type of the values to compare
     * @tparam style How to compare. This defaults to defaultCmpStyle.
     * @param first   left operand of equals operation
     * @param second  right operand of equals operation
     * @param epsilon The epsilon to use in the comparison
     */
    template <class T, CmpStyle style /*= defaultCmpStyle*/>
    bool eq(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, style>::value());
    //! test for inequality using epsilon
    /**
     * @tparam T     Type of the values to compare
     * @tparam style How to compare. This defaults to defaultCmpStyle.
     * @param first   left operand of not-equal operation
     * @param second  right operand of not-equal operation
     * @param epsilon The epsilon to use in the comparison
     * @return        !eq(first, second, epsilon)
     */
    template <class T, CmpStyle style /*= defaultCmpStyle*/>
    bool ne(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, style>::value());
    //! test if first greater than second
    /**
     * @tparam T     Type of the values to compare
     * @tparam style How to compare. This defaults to defaultCmpStyle.
     * @param first   left operand of greater-than operation
     * @param second  right operand of greater-than operation
     * @param epsilon The epsilon to use in the comparison
     * @return        ne(first, second, epsilon) && first > second
     *
     * this is like first > second but the region that compares equal with an
     * epsilon is excluded
     */
    template <class T, CmpStyle style /*= defaultCmpStyle*/>
    bool gt(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, style>::value());
    //! test if first lesser than second
    /**
     * @tparam T     Type of the values to compare
     * @tparam style How to compare. This defaults to defaultCmpStyle.
     * @param first   left operand of less-than operation
     * @param second  right operand of less-than operation
     * @param epsilon The epsilon to use in the comparison
     * @return        ne(first, second, epsilon) && first < second
     *
     * this is like first < second, but the region that compares equal with an
     * epsilon is excluded
     */
    template <class T, CmpStyle style /*= defaultCmpStyle*/>
    bool lt(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, style>::value());
    //! test if first greater or equal second
    /**
     * @tparam T     Type of the values to compare
     * @tparam style How to compare. This defaults to defaultCmpStyle.
     * @param first   left operand of greater-or-equals operation
     * @param second  right operand of greater-or-equals operation
     * @param epsilon The epsilon to use in the comparison
     * @return        eq(first, second, epsilon) || first > second
     *
     * this is like first > second, but the region that compares equal with an
     * epsilon is also included
     */
    template <class T, CmpStyle style /*= defaultCmpStyle*/>
    bool ge(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, style>::value());
    //! test if first lesser or equal second
    /**
     * @tparam T     Type of the values to compare
     * @tparam style How to compare. This defaults to defaultCmpStyle.
     * @param first   left operand of less-or-equals operation
     * @param second  right operand of less-or-equals operation
     * @param epsilon The epsilon to use in the comparison
     * @return        eq(first, second) || first < second
     *
     * this is like first < second, but the region that compares equal with an
     * epsilon is also included
     */
    template <class T, CmpStyle style /*= defaultCmpStyle*/>
    bool le(const T &first,
            const T &second,
            typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, style>::value());

    // rounding operations
    //! round using epsilon
    /**
     * @tparam I      The integral type to round to
     * @tparam T      Type of the value to round
     * @tparam cstyle How to compare. This defaults to defaultCmpStyle.
     * @tparam rstyle How to round. This defaults to defaultRoundingStyle.
     * @param val     The value to round
     * @param epsilon The epsilon to use in comparisons
     * @return        The rounded value
     *
     * Round according to rstyle.  If val is already near the mean of two
     * adjacent integers in terms of epsilon, the result will be the rounded
     * mean.
     */
    template<class I, class T, CmpStyle cstyle /*= defaultCmpStyle*/, RoundingStyle rstyle /*= defaultRoundingStyle*/>
    I round(const T &val, typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, cstyle>::value());
    // truncation
    //! truncate using epsilon
    /**
     * @tparam I      The integral type to truncate to
     * @tparam T      Type of the value to truncate
     * @tparam cstyle How to compare. This defaults to defaultCmpStyle.
     * @tparam rstyle How to truncate. This defaults to defaultRoundingStyle.
     * @param val     The value to truncate
     * @param epsilon The epsilon to use in comparisons
     * @return        The truncated value
     *
     * Truncate according to rstyle.  If val is already near an integer in
     * terms of epsilon, the result will be that integer instead of the real
     * truncated value.
     */
    template<class I, class T, CmpStyle cstyle /*= defaultCmpStyle*/, RoundingStyle rstyle /*= defaultRoundingStyle*/>
    I trunc(const T &val, typename EpsilonType<T>::Type epsilon = DefaultEpsilon<T, cstyle>::value());

    //! @}
    // group FloatCmp
  } //namespace FloatCmp


  // oo interface
  //! Class encapsulating a default epsilon
  /**
   * @ingroup FloatCmp
   * @tparam T       Type of the values to compare
   * @tparam cstyle_ How to compare
   * @tparam rstyle_ How to round
   */
  template<class T, FloatCmp::CmpStyle cstyle_ = FloatCmp::defaultCmpStyle,
      FloatCmp::RoundingStyle rstyle_ = FloatCmp::defaultRoundingStyle>
  class FloatCmpOps {
    typedef FloatCmp::CmpStyle CmpStyle;
    typedef FloatCmp::RoundingStyle RoundingStyle;

  public:
    // record template parameters
    //! How comparisons are done
    static const CmpStyle cstyle = cstyle_;
    //! How rounding is done
    static const RoundingStyle rstyle = rstyle_;
    //! Type of the values to compare
    typedef T ValueType;
    //! Type of the epsilon.
    /**
     * May be different from the value type, for example for complex<double>
     */
    typedef typename FloatCmp::EpsilonType<T>::Type EpsilonType;

  private:
    EpsilonType epsilon_;

    typedef FloatCmp::DefaultEpsilon<EpsilonType, cstyle> DefaultEpsilon;

  public:
    //! construct an operations object
    /**
     * @param epsilon Use the specified epsilon for comparing
     */
    FloatCmpOps(EpsilonType epsilon = DefaultEpsilon::value());

    //! return the current epsilon
    EpsilonType epsilon() const;
    //! set new epsilon
    void epsilon(EpsilonType epsilon__);

    //! test for equality using epsilon
    bool eq(const ValueType &first, const ValueType &second) const;
    //! test for inequality using epsilon
    /**
     * this is exactly !eq(first, second)
     */
    bool ne(const ValueType &first, const ValueType &second) const;
    //! test if first greater than second
    /**
     * this is exactly ne(first, second) && first > second, i.e. greater but
     * the region that compares equal with an epsilon is excluded
     */
    bool gt(const ValueType &first, const ValueType &second) const;
    //! test if first lesser than second
    /**
     * this is exactly ne(first, second) && first < second, i.e. lesser but
     * the region that compares equal with an epsilon is excluded
     */
    bool lt(const ValueType &first, const ValueType &second) const;
    //! test if first greater or equal second
    /**
     * this is exactly eq(first, second) || first > second, i.e. greater but
     * the region that compares equal with an epsilon is also included
     */
    bool ge(const ValueType &first, const ValueType &second) const;
    //! test if first lesser or equal second
    /**
     * this is exactly eq(first, second) || first > second, i.e. lesser but
     * the region that compares equal with an epsilon is also included
     */
    bool le(const ValueType &first, const ValueType &second) const;

    //! round using epsilon
    /**
     * @tparam I   The integral type to round to
     *
     * @param  val The value to round
     *
     * Round according to rstyle.  If val is already near the mean of two
     * adjacent integers in terms of epsilon, the result will be the rounded
     * mean.
     */
    template<class I>
    I round(const ValueType &val) const;

    //! truncate using epsilon
    /**
     * @tparam I   The integral type to truncate to
     *
     * @param  val The value to truncate
     *
     * Truncate according to rstyle.  If val is already near an integer in
     * terms of epsilon, the result will be that integer instead of the real
     * truncated value.
     */
    template<class I>
    I trunc(const ValueType &val) const;

  };

} //namespace Dune

#include "float_cmp.cc"

#endif //DUNE_COMMON_FLOAT_CMP_HH
