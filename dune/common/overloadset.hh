// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_OVERLOADSET_HH
#define DUNE_COMMON_OVERLOADSET_HH

#include <utility>
#include <type_traits>

#include <dune/common/std/type_traits.hh>



namespace Dune {

namespace Impl {

#if __cpp_variadic_using >= 201611

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

#else // __cpp_variadic_using >= 201611

  // This overload set derives from
  // all passed functions. Since we
  // cannot do argument pack expansion
  // on using statements this is done recursively.
  template<class F0, class... F>
  class OverloadSet: public OverloadSet<F...>, F0
  {
    using Base = OverloadSet<F...>;
  public:

    template<class FF0, class... FF>
    OverloadSet(FF0&& f0, FF&&... ff) :
      Base(std::forward<FF>(ff)...),
      F0(std::forward<FF0>(f0))
    {}

    // pull in operator() of F0 and of all F... via the base class
    using F0::operator();
    using Base::operator();
  };

  template<class F0>
  class OverloadSet<F0>: public F0
  {
  public:

    template<class FF0>
    OverloadSet(FF0&& f0) :
      F0(std::forward<FF0>(f0))
    {}

    // pull in operator() of F0
    using F0::operator();
  };

#endif // __cpp_variadic_using >= 201611

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
 * On gcc 5 and gcc 6 mixing templated overloads
 * (i.e. using auto-parameter) and non-templated
 * ones may not compile if both they are captureless
 * lambdas. The problem can be avoided by capturing
 * a dummy value.
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
        std::enable_if_t<Std::is_callable<F0(Args&&...)>::value, int> = 0>
    decltype(auto) operator()(Args&&... args)
    {
      return F0::operator()(std::forward<Args>(args)...);
    }

    // Forward to operator() of base class if F0 cannot be called with the given
    // arguments. In this case the base class will successively try operator()
    // of all F... .
    template<class...  Args,
        std::enable_if_t< not Std::is_callable<F0(Args&&...)>::value, int> = 0>
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
      static_assert(Std::is_callable<F0(Args&&...)>::value, "No matching overload found in OrderedOverloadSet");
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
 * On gcc 5 and gcc 6 mixing templated overloads
 * (i.e. using auto-parameter) and non-templated
 * ones may not compile if both they are captureless
 * lambdas. The problem can be avoided by capturing
 * a dummy value.
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
