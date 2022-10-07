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
#include <dune/common/assertandreturn.hh>
#include <dune/common/rangeutilities.hh>



namespace Dune {
namespace Hybrid {

namespace Impl {

  // Try if tuple_size is implemented for class
  template<class T, int i>
  constexpr auto size(const Dune::FieldVector<T, i>&, const PriorityTag<5>&)
    -> decltype(std::integral_constant<std::size_t,i>())
  {
    return {};
  }

  // Try if tuple_size is implemented for class
  template<class T>
  constexpr auto size(const T&, const PriorityTag<3>&)
    -> decltype(std::integral_constant<std::size_t,std::tuple_size<T>::value>())
  {
    return {};
  }

  // Try if there's a static constexpr size()
  template<class T>
  constexpr auto size(const T&, const PriorityTag<1>&)
    -> decltype(std::integral_constant<std::size_t,T::size()>())
  {
    return {};
  }

  // As a last resort try if there's a static constexpr size()
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
 * * all typed that have a static method ::size()
 * * instances of Dune::FieldVector
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
    return Dune::integerSequenceEntry(c, std::integral_constant<std::size_t, Index::value>());
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

  // This should be constexpr but gcc-4.9 does not support
  // the relaxed constexpr requirements. Hence for being
  // constexpr the function body can only contain a return
  // statement and no assertion before this.
  template<class Begin, class End>
  constexpr auto integralRange(const Begin& begin, const End& end, const PriorityTag<0>&)
  {
    return DUNE_ASSERT_AND_RETURN(begin<=end, Dune::IntegralRange<End>(begin, end));
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

  template<class T1, class T2>
  constexpr auto equals(const T1& /*t1*/, const T2& /*t2*/, PriorityTag<1>) -> decltype(T1::value, T2::value, std::integral_constant<bool,T1::value == T2::value>())
  { return {}; }

  template<class T1, class T2>
  constexpr auto equals(const T1& t1, const T2& t2, PriorityTag<0>)
  {
    return t1==t2;
  }

} // namespace Impl



/**
 * \brief Equality comparison
 *
 * \ingroup HybridUtilities
 *
 * If both types have a static member value, the result of comparing
 * these is returned as std::integral_constant<bool, *>. Otherwise
 * the result of a runtime comparison of t1 and t2 is directly returned.
 */
template<class T1, class T2>
constexpr auto equals(T1&& t1,  T2&& t2)
{
  return Impl::equals(std::forward<T1>(t1), std::forward<T2>(t2), PriorityTag<1>());
}



namespace Impl {

  template<class Result, class T, class Value, class Branches, class ElseBranch>
  constexpr Result switchCases(std::integer_sequence<T>, const Value& /*value*/, Branches&& /*branches*/, ElseBranch&& elseBranch)
  {
    return elseBranch();
  }

  template<class Result, class T, T t0, T... tt, class Value, class Branches, class ElseBranch>
  constexpr Result switchCases(std::integer_sequence<T, t0, tt...>, const Value& value, Branches&& branches, ElseBranch&& elseBranch)
  {
    return ifElse(
        Hybrid::equals(std::integral_constant<T, t0>(), value),
      [&](auto id) -> decltype(auto) {
        return id(branches)(std::integral_constant<T, t0>());
      }, [&](auto id) -> decltype(auto) {
        return Impl::switchCases<Result>(id(std::integer_sequence<T, tt...>()), value, branches, elseBranch);
    });
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
 * the value is passed as std::integral_constant.
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
 * the value is passed as std::integral_constant.
 * If non of the entries matches, then elseBranch is executed
 * without any argument.
 */
template<class Cases, class Value, class Branches>
constexpr void switchCases(const Cases& cases, const Value& value, Branches&& branches)
{
  Impl::switchCases<void>(cases, value, std::forward<Branches>(branches), []() {});
}


} // namespace Hybrid
} // namespace Dune


#endif // #ifndef DUNE_COMMON_HYBRIDUTILITIES_HH
