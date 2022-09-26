// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_OVERLOADSET_HH
#define DUNE_COMMON_OVERLOADSET_HH

#include <utility>
#include <type_traits>
#include <dune/common/typetraits.hh>

namespace Dune {

namespace Impl {

  template<typename... F>
  class OverloadSet
    : public F...
  {

  public:

    template<typename... FF>
    OverloadSet(FF&&... ff)
      : F(std::forward<FF>(ff))...
    {}

    using F::operator()...;

  };

} // end namespace Impl



/**
 * \brief Create an overload set
 *
 * \tparam F List of function object types
 * \param f List of function objects
 *
 * This returns an object that contains all
 * operator() implementations of the passed
 * functions. All those are available when
 * calling operator() of the returned object.
 *
 * The returned object derives from
 * those implementations such that it contains
 * all operator() implementations in its
 * overload set. When calling operator()
 * this will select the best overload.
 * If multiple overload are equally good this
 * will lead to ambiguity.
 *
 * Notice that the passed function objects are
 * stored by value and must be copy-constructible.
 *
 * \ingroup CxxUtilities
 */
template<class... F>
auto overload(F&&... f)
{
  return Impl::OverloadSet<std::decay_t<F>...>(std::forward<F>(f)...);
}



namespace Impl {

  template<class F0, class... F>
  class OrderedOverloadSet: public OrderedOverloadSet<F...>, F0
  {
    using Base = OrderedOverloadSet<F...>;
  public:

    template<class FF0, class... FF>
    OrderedOverloadSet(FF0&& f0, FF&&... ff) :
      Base(std::forward<FF>(ff)...),
      F0(std::forward<FF0>(f0))
    {}

    // Forward to operator() of F0 if it can be called with the given arguments.
    template<class...  Args,
        std::enable_if_t<IsCallable<F0(Args&&...)>::value, int> = 0>
    decltype(auto) operator()(Args&&... args)
    {
      return F0::operator()(std::forward<Args>(args)...);
    }

    // Forward to operator() of base class if F0 cannot be called with the given
    // arguments. In this case the base class will successively try operator()
    // of all F... .
    template<class...  Args,
        std::enable_if_t<not IsCallable<F0(Args&&...)>::value, int> = 0>
    decltype(auto) operator()(Args&&... args)
    {
      return Base::operator()(std::forward<Args>(args)...);
    }

  };

  template<class F0>
  class OrderedOverloadSet<F0>: public F0
  {
  public:

    template<class FF0>
    OrderedOverloadSet(FF0&& f0) :
      F0(std::forward<FF0>(f0))
    {}

    // Forward to operator() of F0. If it cannot be called with
    // the given arguments a static assertion will fail.
    template<class...  Args>
    decltype(auto) operator()(Args&&... args)
    {
      static_assert(IsCallable<F0(Args&&...)>::value,
                      "No matching overload found in OrderedOverloadSet");
      return F0::operator()(std::forward<Args>(args)...);
    }
  };

} // end namespace Impl



/**
 * \brief Create an ordered overload set
 *
 * \tparam F List of function object types
 * \param f List of function objects
 *
 * This returns an object that contains all
 * operator() implementations of the passed
 * functions. All those are available when
 * calling operator() of the returned object.
 *
 * In contrast to overload() these overloads
 * are ordered in the sense that the first
 * matching overload for the given arguments
 * is selected and later ones are ignored.
 * Hence such a call is never ambiguous.
 *
 * Notice that the passed function objects are
 * stored by value and must be copy-constructible.
 *
 * \ingroup CxxUtilities
 */
template<class... F>
auto orderedOverload(F&&... f)
{
  return Impl::OrderedOverloadSet<std::decay_t<F>...>(std::forward<F>(f)...);
}



} // end namespace Dune

#endif // DUNE_COMMON_OVERLOADSET_HH
