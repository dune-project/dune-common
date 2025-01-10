// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_HYBRIDUTILITIES_HH
#define DUNE_COMMON_HYBRIDUTILITIES_HH

#include <tuple>
#include <utility>

#include <dune/common/typetraits.hh>
#include <dune/common/typeutilities.hh>
#include <dune/common/fvector.hh>
#include <dune/common/indices.hh>
#include <dune/common/integersequence.hh>
#include <dune/common/rangeutilities.hh>



namespace Dune {
namespace Hybrid {

namespace Impl {

  // Try if std::tuple_size is implemented for class
  template<class T>
  constexpr auto size(const T&, const PriorityTag<2>&)
    -> decltype(std::integral_constant<std::size_t,std::tuple_size<T>::value>())
  {
    return {};
  }

  // Try if there's a static constexpr size() method
  template<class T>
  constexpr auto size(const T&, const PriorityTag<1>&)
    -> decltype(std::integral_constant<std::size_t,T::size()>())
  {
    return {};
  }

  // As a last resort try if there's a non-static size()
  template<class T>
  constexpr auto size(const T& t, const PriorityTag<0>&)
  {
    return t.size();
  }

} // namespace Impl



/**
 * \brief Size query
 *
 * \ingroup HybridUtilities
 *
 * \tparam T Type of container whose size is queried
 *
 * \param t Container whose size is queried
 *
 * \return Size of t
 *
 * If the size of t is known at compile type the size is
 * returned as std::integral_constant<std::size_t, size>.
 * Otherwise the result of t.size() is returned.
 *
 * Supported types for deriving the size at compile time are:
 * * instances of std::integer_sequence
 * * all types std::tuple_size is implemented for
 * * all types that have a static constexpr method ::size()
 * The latter e.g. includes Dune::FieldVector
 */
template<class T>
constexpr auto size(const T& t)
{
  return Impl::size(t, PriorityTag<42>());
}



namespace Impl {

  template<class Container, class Index,
    std::enable_if_t<IsTuple<std::decay_t<Container>>::value, int> = 0>
  constexpr decltype(auto) elementAt(Container&& c, Index&&, PriorityTag<2>)
  {
    return std::get<std::decay_t<Index>::value>(c);
  }

  template<class T, T... t, class Index>
  constexpr decltype(auto) elementAt(std::integer_sequence<T, t...> c, Index, PriorityTag<1>)
  {
    return Dune::get<Index::value>(c);
  }

  template<class Container, class Index>
  constexpr decltype(auto) elementAt(Container&& c, Index&& i, PriorityTag<0>)
  {
    return c[i];
  }

} // namespace Impl



/**
 * \brief Get element at given position from container
 *
 * \ingroup HybridUtilities
 *
 * \tparam Container Type of given container
 * \tparam Index Type of index
 *
 * \param c Given container
 * \param i Index of element to obtain
 *
 * \return The element at position i, i.e. c[i]
 *
 * If this returns the i-th entry of c. It supports the following
 * containers
 * * Containers providing dynamic access via operator[]
 * * Heterogeneous containers providing access via operator[](integral_constant<...>)
 * * std::tuple<...>
 * * std::integer_sequence
 */
template<class Container, class Index>
constexpr decltype(auto) elementAt(Container&& c, Index&& i)
{
  return Impl::elementAt(std::forward<Container>(c), std::forward<Index>(i), PriorityTag<42>());
}



namespace Impl {

  template<class Begin, class End,
    std::enable_if_t<IsIntegralConstant<Begin>::value and IsIntegralConstant<End>::value, int> = 0>
  constexpr auto integralRange(const Begin& /*begin*/, const End& /*end*/, const PriorityTag<1>&)
  {
    static_assert(Begin::value <= End::value, "You cannot create an integralRange where end<begin");
    return Dune::StaticIntegralRange<std::size_t, End::value, Begin::value>();
  }

  template<class Begin, class End>
  constexpr auto integralRange(const Begin& begin, const End& end, const PriorityTag<0>&)
  {
    assert(begin<=end && "You cannot create an integralRange where end<begin");
    return Dune::IntegralRange<End>(begin, end);
  }

} // namespace Impl



/**
 * \brief Create an integral range
 *
 * \ingroup HybridUtilities
 *
 * \tparam Begin Type of begin entry of the range
 * \tparam End Type of end entry of the range
 *
 * \param begin First entry of the range
 * \param end One past the last entry of the range
 *
 * \returns An object encoding the given range
 *
 * If Begin and End are both instances of type
 * std::integral_constant, the returned range
 * encodes begin and end statically.
 */
template<class Begin, class End>
constexpr auto integralRange(const Begin& begin, const End& end)
{
  return Impl::integralRange(begin, end, PriorityTag<42>());
}

/**
 * \brief Create an integral range starting from 0
 *
 * \ingroup HybridUtilities
 *
 * \tparam End Type of end entry of the range
 *
 * \param end One past the last entry of the range
 *
 * \returns An object encoding the given range
 *
 * This is a short cut for integralRange(_0, end).
 */
template<class End>
constexpr auto integralRange(const End& end)
{
  return Impl::integralRange(Dune::Indices::_0, end, PriorityTag<42>());
}



namespace Impl {

  template<class T>
  constexpr void evaluateFoldExpression(std::initializer_list<T>&&)
  {}

  template<class Range, class F, class Index, Index... i>
  constexpr void forEachIndex(Range&& range, F&& f, std::integer_sequence<Index, i...>)
  {
    evaluateFoldExpression<int>({(f(Hybrid::elementAt(range, std::integral_constant<Index,i>())), 0)...});
  }

  template<class F, class Index, Index... i>
  constexpr void forEach(std::integer_sequence<Index, i...> /*range*/, F&& f, PriorityTag<2>)
  {
    evaluateFoldExpression<int>({(f(std::integral_constant<Index,i>()), 0)...});
  }


  template<class Range, class F,
    std::enable_if_t<IsIntegralConstant<decltype(Hybrid::size(std::declval<Range>()))>::value, int> = 0>
  constexpr void forEach(Range&& range, F&& f, PriorityTag<1>)
  {
    auto size = Hybrid::size(range);
    auto indices = std::make_index_sequence<size>();
    (forEachIndex)(std::forward<Range>(range), std::forward<F>(f), indices);
  }

  template<class Range, class F>
  constexpr void forEach(Range&& range, F&& f, PriorityTag<0>)
  {
      for(auto&& e : range)
        f(e);
  }

} // namespace Impl



/**
 * \brief Range based for loop
 *
 * \ingroup HybridUtilities
 *
 * \tparam Range Type of given range
 * \tparam F Type of given predicate
 *
 * \param range The range to loop over
 * \param f A predicate that will be called with each entry of the range
 *
 * This supports looping over the following ranges
 * * ranges obtained from integralRange()
 * * all ranges that provide Hybrid::size() and Hybrid::elementAt()
 *
 * This especially included instances of std::integer_sequence,
 * std::tuple, Dune::TupleVector, and Dune::MultiTypeBlockVector.
 */
template<class Range, class F>
constexpr void forEach(Range&& range, F&& f)
{
  Impl::forEach(std::forward<Range>(range), std::forward<F>(f), PriorityTag<42>());
}



/**
 * \brief Accumulate values
 *
 * \ingroup HybridUtilities
 *
 * \tparam Range Type of given range
 * \tparam T Type of accumulated value
 * \tparam F Type of binary accumulation operator
 *
 * \param range The range of values to accumulate
 * \param value Initial value for accumulation
 * \param f Binary operator for accumulation
 *
 * This supports looping over the same ranges as Hybrid::forEach
 */
template<class Range, class T, class F>
constexpr T accumulate(Range&& range, T value, F&& f)
{
  forEach(std::forward<Range>(range), [&](auto&& entry) {
    value = f(value, entry);
  });
  return value;
}



namespace Impl {

  struct Id {
    template<class T>
    constexpr T operator()(T&& x) const {
      return std::forward<T>(x);
    }
  };

  template<class IfFunc, class ElseFunc>
  constexpr decltype(auto) ifElse(std::true_type, IfFunc&& ifFunc, ElseFunc&& /*elseFunc*/)
  {
    return ifFunc(Id{});
  }

  template<class IfFunc, class ElseFunc>
  constexpr decltype(auto) ifElse(std::false_type, IfFunc&& /*ifFunc*/, ElseFunc&& elseFunc)
  {
    return elseFunc(Id{});
  }

  template<class IfFunc, class ElseFunc>
  decltype(auto) ifElse(const bool& condition, IfFunc&& ifFunc, ElseFunc&& elseFunc)
  {
    if (condition)
      return ifFunc(Id{});
    else
      return elseFunc(Id{});
  }

} // namespace Impl



/**
 * \brief A conditional expression
 *
 * \ingroup HybridUtilities
 *
 * This will call either ifFunc or elseFunc depending
 * on the condition. In any case a single argument
 * will be passed to the called function. This will always
 * be the identity function. Passing an expression through
 * this function will lead to lazy evaluation. This way both
 * 'branches' can contain expressions that are only valid
 * within this branch if the condition is a std::integral_constant<bool,*>.
 *
 * In order to do this, the passed functors must have a single
 * argument of type auto.
 *
 * Due to the lazy evaluation mechanism and support for
 * std::integral_constant<bool,*> this allows to emulate
 * a static if statement.
 */
template<class Condition, class IfFunc, class ElseFunc>
decltype(auto) ifElse(const Condition& condition, IfFunc&& ifFunc, ElseFunc&& elseFunc)
{
  return Impl::ifElse(condition, std::forward<IfFunc>(ifFunc), std::forward<ElseFunc>(elseFunc));
}

/**
 * \brief A conditional expression
 *
 * \ingroup HybridUtilities
 *
 * This provides an ifElse conditional with empty else clause.
 */
template<class Condition, class IfFunc>
void ifElse(const Condition& condition, IfFunc&& ifFunc)
{
  ifElse(condition, std::forward<IfFunc>(ifFunc), [](auto&&) {});
}



namespace Impl {

  struct Max {
    template<class... Args>
    constexpr decltype(auto) operator()(Args&&... args) const
    {
      using T = std::common_type_t<Args...>;
      return std::max({T(args)...});
    }
  };

  struct Min {
    template<class... Args>
    constexpr decltype(auto) operator()(Args&&... args) const
    {
      using T = std::common_type_t<Args...>;
      return std::min({T(args)...});
    }
  };

} // namespace Impl


/**
 * \brief Adapter of a hybrid functor that maintains results hybrid
 *
 * \ingroup HybridUtilities
 *
 * This adapter will return an integral constant if all of the arguments are
 * integral constants. That's helpful to maintain the hybrid nature of a variable
 * after a transformation. For example, applying an operator + between two integral
 * constants will promote the result to its underlying type (e.g. std::size_t).
 * That's inconventient since the value of the result is not encoded in the type
 * anymore, thus, losing its hybrid attribute (maybe still being constexpr).
 *
 * \code{.cpp}
 *  using namespace Dune::Indices;
 *  { // non-hybrid transformation!
 *    auto i =  1 +  2;      // -> 3
 *    auto j =  1 + _2;      // -> 3
 *    auto k = _1 + _2;      // -> 3
 *    // depending of the context, `k` may or may not be constexpr
 *  }
 *  { // hybrid transformation!
 *    auto plus = Hybrid::HybridFunctor<std::plus<>>{};
 *    auto j = plus( 1,  2); // -> 3
 *    auto j = plus( 1, _2); // -> 3
 *    auto k = plus(_1, _2); // -> Dune::Indices::_3
 *    // independent of the context, `k` encodes its value in the type system
 *  }
 * \endcode
 *
 */
template<class Functor>
class HybridFunctor {

  static_assert(std::is_default_constructible_v<Functor>,
    "Operator in integral expressions shall be constexpr default constructible");

  inline static constexpr Functor _functor = Functor{};

public:

/**
 * \brief Adapter of a hybrid functor that keeps results hybrid
 *
 * \ingroup HybridUtilities
 *
 * Implements an operator that promotes the results of the underlying functor to
 * an integral constant if all the function arguments are integral constants,
 * otherwise, usual promotion rules apply.
 */
  template<class... Args>
  constexpr decltype(auto) operator()(const Args&... args) const
  {
    if constexpr (std::conjunction_v<IsCompileTimeConstant<Args>...>)
    {
      constexpr auto result = _functor(Args::value...);
      // apply functor on integral constant arguments and return an integral constant of the result
      // this is guaranteed to be evaluated at compile-time
      return std::integral_constant<std::remove_cv_t<decltype(result)>,result>{};
    } else {
      // apply functor directly on arguments and return the result of the functor
      // (integral constants are likely to be casted to underlying type)
      // this not is guaranteed to be evaluated at compile-time although is possible if expression is constexpr
      return _functor(args...);
    }
  }
};

/**
 * \brief Returns an HybridFunctor adaptor
 * \see HybridFunctor
 */
template<class Functor>
constexpr HybridFunctor<Functor> hybridFunctor(const Functor&)
{
  return {};
}

/**
 * \brief Function object that returns the greater of the given values
 *
 * \ingroup HybridUtilities
 * \see HybridFunctor
 *
 * If all arguments have a static member value, the maximum value of
 * these is returned as std::integral_constant<*, *>. Otherwise
 * the result of a direct max of the arguments is returned.
 *
 * \code{.cpp}
 *  using namespace Dune::Indices;
 *  { // hybrid transformation!
 *    auto j = Dune::Hybrid::max( 1,  2); // -> 2
 *    auto j = Dune::Hybrid::max( 1, _2); // -> 2
 *    auto k = Dune::Hybrid::max(_1, _2); // -> Dune::Indices::_2
 *    // independent of the context, `k` encodes its value in the type system
 *  }
 * \endcode
 */
inline constexpr auto max = hybridFunctor(Impl::Max{});

/**
 * \brief Function object that returns the smaller of the given values
 *
 * \ingroup HybridUtilities
 * \see HybridFunctor
 *
 * If all arguments have a static member value, the minimum value of
 * these is returned as std::integral_constant<*, *>. Otherwise
 * the result of a direct min of the arguments is returned.
 *
 * \code{.cpp}
 *  using namespace Dune::Indices;
 *  { // hybrid transformation!
 *    auto j = Dune::Hybrid::min( 1,  2); // -> 1
 *    auto j = Dune::Hybrid::min( 1, _2); // -> 1
 *    auto k = Dune::Hybrid::min(_1, _2); // -> Dune::Indices::_1
 *    // independent of the context, `k` encodes its value in the type system
 *  }
 * \endcode
 */
inline constexpr auto min = hybridFunctor(Impl::Min{});

/**
 * \brief Function object for performing addition
 *
 * \ingroup HybridUtilities
 * \see HybridFunctor
 *
 * If all arguments have a static member value, the added value of
 * these is returned as std::integral_constant<*, *>. Otherwise
 * the result of a direct addition of the arguments is returned.
 *
 * \code{.cpp}
 *  using namespace Dune::Indices;
 *  { // hybrid transformation!
 *    auto j = Dune::Hybrid::plus( 1,  2); // -> 3
 *    auto j = Dune::Hybrid::plus( 1, _2); // -> 3
 *    auto k = Dune::Hybrid::plus(_1, _2); // -> Dune::Indices::_3
 *    // independent of the context, `k` encodes its value in the type system
 *  }
 * \endcode
 */
inline constexpr auto plus = hybridFunctor(std::plus<>{});

/**
 * \brief Function object for performing subtraction
 *
 * \ingroup HybridUtilities
 * \see HybridFunctor
 *
 * If all arguments have a static member value, the subtracted value of
 * these is returned as std::integral_constant<*, *>. Otherwise
 * the result of a direct subtraction of the arguments is returned.
 *
 * \code{.cpp}
 *  using namespace Dune::Indices;
 *  { // hybrid transformation!
 *    auto j = Dune::Hybrid::minus( 2,  1); // -> 1
 *    auto j = Dune::Hybrid::minus( 2, _1); // -> 1
 *    auto k = Dune::Hybrid::minus(_2, _1); // -> Dune::Indices::_1
 *    // independent of the context, `k` encodes its value in the type system
 *  }
 * \endcode
 */
inline constexpr auto minus = hybridFunctor(std::minus<>{});

/**
 * \brief Function object for performing equality comparison
 *
 * \ingroup HybridUtilities
 * \see HybridFunctor
 *
 * If both arguments have a static member value, the result of comparing
 * these for equality is returned as std::integral_constant<bool, *>. Otherwise
 * the result of a comparison of the two arguments is directly returned.
 *
 * \code{.cpp}
 *  using namespace Dune::Indices;
 *  { // hybrid transformation!
 *    auto j = Dune::Hybrid::equal_to( 2,  1); // -> false
 *    auto j = Dune::Hybrid::equal_to( 2, _1); // -> false
 *    auto k = Dune::Hybrid::equal_to(_2, _1); // -> std::false_type
 *    // independent of the context, `k` encodes its value in the type system
 *  }
 * \endcode
 */
inline constexpr auto equal_to = hybridFunctor(std::equal_to<>{});


/**
 * \brief Equality comparison
 *
 * \ingroup HybridUtilities
 *
 * If both types have a static member value, the result of comparing
 * these is returned as std::integral_constant<bool, *>. Otherwise
 * the result of a runtime comparison of t1 and t2 is directly returned.
 *
 * \deprecated
 */
template<class T1, class T2>
[[deprecated("Use Hybrid::equal_to instead!")]] constexpr auto equals(T1&& t1,  T2&& t2){
  return equal_to(std::forward<T1>(t1), std::forward<T2>(t2));
}


namespace Impl {

  // This overload is selected if the passed value is already a compile time constant.
  template<class Result, class T, T t0, T... tt, class ValueType, ValueType value, class Branches, class ElseBranch>
  constexpr Result switchCases(std::integer_sequence<T, t0, tt...>, const std::integral_constant<ValueType, value>& /*value*/, Branches&& branches, ElseBranch&& elseBranch)
  {
    // In case we pass a value known at compile time, we no longer have to do
    // a dynamic to static dispatch via recursion. The only thing that's left
    // is to check if the value is contained in the passed range.
    // If this is true, we have to pass it to the branches callback
    // as an appropriate integral_constant type. Otherwise we have to
    // execute the else callback.
    if constexpr (((t0 == value) || ... || (tt == value)))
      return branches(std::integral_constant<T, value>{});
    else
      return elseBranch();
  }

  // This overload is selected if the passed value is dynamic.
  template<class Result, class T, class Value, class Branches, class ElseBranch>
  constexpr Result switchCases(std::integer_sequence<T>, const Value& /*value*/, Branches&& /*branches*/, ElseBranch&& elseBranch)
  {
    return elseBranch();
  }

  template<class Result, class T, T t0, T... tt, class Value, class Branches, class ElseBranch>
  constexpr Result switchCases(std::integer_sequence<T, t0, tt...>, const Value& value, Branches&& branches, ElseBranch&& elseBranch)
  {
    if (t0 == value)
      return branches(std::integral_constant<T, t0>());
    else
      return Impl::switchCases<Result>(std::integer_sequence<T, tt...>(), value, branches, elseBranch);
  }

  // This overload is selected if the range of cases is an IntegralRange
  template <class Result, class T, class Value, class Branches, class ElseBranch>
  constexpr Result switchCases(IntegralRange<T> range, const Value& value, Branches&& branches, ElseBranch&& elseBranch)
  {
    return range.contains(value) ? branches(T(value)) : elseBranch();
  }

  // This overload is selected if the range of cases is a StaticIntegralRange
  template <class Result, class T, T to, T from, class Value, class Branches, class ElseBranch>
  constexpr Result switchCases(StaticIntegralRange<T, to, from> range, const Value& value, Branches&& branches, ElseBranch&& elseBranch)
  {
    using seq = typename decltype(range)::integer_sequence;
    return Impl::switchCases<Result>(seq{}, value, branches, elseBranch);
  }

} // namespace Impl



/**
 * \brief Switch statement
 *
 * \ingroup HybridUtilities
 *
 * \tparam Cases Type of case range
 * \tparam Value Type of value to check against the cases
 * \tparam Branches Type of branch function
 * \tparam ElseBranch Type of branch function
 *
 * \param cases A range of cases to check for
 * \param value The value to check against the cases
 * \param branches A callback that will be executed with matching entry from case list
 * \param elseBranch A callback that will be executed if no other entry matches
 *
 * Value is checked against all entries of the given range.
 * If one matches, then branches is executed with the matching
 * value as single argument. If the range is an std::integer_sequence,
 * or StaticIntegralRange, the value is passed as std::integral_constant.
 * If non of the entries matches, then elseBranch is executed
 * without any argument.
 *
 * Notice that this short circuits, e.g., if one case matches,
 * the others are no longer evaluated.
 *
 * The return value will be deduced from the else branch.
 */
template<class Cases, class Value, class Branches, class ElseBranch>
constexpr decltype(auto) switchCases(const Cases& cases, const Value& value, Branches&& branches, ElseBranch&& elseBranch)
{
  return Impl::switchCases<decltype(elseBranch())>(cases, value, std::forward<Branches>(branches), std::forward<ElseBranch>(elseBranch));
}

/**
 * \brief Switch statement
 *
 * \ingroup HybridUtilities
 *
 * \tparam Cases Type of case range
 * \tparam Value Type of value to check against the cases
 * \tparam Branches Type of branch function
 *
 * \param cases A range of cases to check for
 * \param value The value to check against the cases
 * \param branches A callback that will be executed with matching entry from case list
 *
 * Value is checked against all entries of the given range.
 * If one matches, then branches is executed with the matching
 * value as single argument. If the range is an std::integer_sequence,
 * or StaticIntegralRange, the value is passed as std::integral_constant.
 * If non of the entries matches, the behavior is undefined.
 */
template<class Cases, class Value, class Branches>
constexpr void switchCases(const Cases& cases, const Value& value, Branches&& branches)
{
  Impl::switchCases<void>(cases, value, std::forward<Branches>(branches),
    []{ assert(false && "value not found in range"); });
}

/**
 * \brief Switch statement
 *
 * \ingroup HybridUtilities
 *
 * \tparam T The type of the cases
 * \tparam Value Type of value to check against the cases
 * \tparam Branches Type of branch function
 *
 * \param range A dynamic range of cases to check for
 * \param value The value to check against the cases
 * \param branches A callback that will be executed with matching entry from case list
 *
 * This overload of the `switchCases` utility is selected if the
 * range of cases is passed as an `IntegralRange`. If the `value`
 * is contained in that `range`, it is passed as single argument
 * to the callback `branches`. If not, the behavior is undefined.
 */
template <class T, class Value, class Branches>
constexpr void switchCases(IntegralRange<T> range, const Value& value, Branches&& branches)
{
  assert(range.contains(value) && "value not found in range");
  branches(T(value));
}

} // namespace Hybrid
} // namespace Dune


#endif // #ifndef DUNE_COMMON_HYBRIDUTILITIES_HH
