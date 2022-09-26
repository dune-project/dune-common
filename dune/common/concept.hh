// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_CONCEPT_HH
#define DUNE_COMMON_CONCEPT_HH

#include <type_traits>
#include <utility>
#include <tuple>

#include <dune/common/typeutilities.hh>
#include <dune/common/typelist.hh>
#include <dune/common/tupleutility.hh>
#include <dune/common/std/type_traits.hh>

/**
 * \file
 *
 * \brief Infrastructure for concepts.
 */

namespace Dune {

/**
 * \brief Namespace for concepts
 *
 * This namespace contains helper functions for
 * concept definitions and the concept definitions
 * themselves.
 *
 * \ingroup CxxConcepts
 */
namespace Concept {



/**
 * \brief Base class for refined concepts.
 *
 * If a new concept should refine one or more existing concepts,
 * this can be achieved by deriving the new concept from
 * Refines<C1,...,CN> where C1, ..., CN are the concepts
 * to be refined. If you want to refine several concepts
 * they should all be put in a single Refines<...> base
 * class.
 *
 * \tparam BaseConcepts The list of concepts to be refined.
 *
 * \ingroup CxxConcepts
 */
template<class... BaseConcepts>
struct Refines
{
  typedef TypeList<BaseConcepts...> BaseConceptList;
};


#ifndef DOXYGEN

namespace Impl {

  // #############################################################################
  // # All functions following here are implementation details
  // # for the models() function below.
  // #############################################################################

  // Forward declaration
  template<class C, class... T>
  constexpr bool models();



  // Here is the implementation of the concept checking.
  // The first two overloads do the magic for checking
  // if the requirements of a concept are satisfied.
  // The rest is just for checking base concepts in case
  // of refinement.

  // This overload is present if type substitution for
  // C::require(T...) is successful, i.e., if the T...
  // matches the requirement of C. In this case this
  // overload is selected because PriorityTag<1>
  // is a better match for PrioriryTag<42> than
  // PriorityTag<0> in the default overload.
  template<class C, class... T,
    decltype(std::declval<C>().require(std::declval<T>()...), 0) =0>
  constexpr std::true_type matchesRequirement(PriorityTag<1>)
  { return {}; }

  // If the above overload is ruled out by SFINAE because
  // the T... does not match the requirements of C, then
  // this default overload drops in.
  template<class C, class... T>
  constexpr std::false_type matchesRequirement(PriorityTag<0>)
  { return {}; }



  // An empty list C of concepts is always matched by T...
  template<class...T>
  constexpr bool modelsConceptList(TypeList<>)
  { return true; }

  // A nonempty list C0,..,CN of concepts is modeled
  // by T...  if it models the concept C0
  // and all concepts in the list C1,..,CN.
  template<class...T, class C0, class... CC>
  constexpr bool modelsConceptList(TypeList<C0, CC...>)
  { return models<C0, T...>() and modelsConceptList<T...>(TypeList<CC...>()); }



  // If C is an unrefined concept, then T... models C
  // if it matches the requirement of C.
  template<class C, class... T>
  constexpr bool modelsConcept(PriorityTag<0>)
  { return matchesRequirement<C, T...>(PriorityTag<42>()); }

  // If C is a refined concept, then T... models C
  // if it matches the requirement of C and of
  // all base concepts.
  //
  // This overload is used if C::BaseConceptList exists
  // due to its higher priority.
  template<class C, class... T,
    decltype(typename C::BaseConceptList(), 0) = 0>
  constexpr bool modelsConcept(PriorityTag<1>)
  { return matchesRequirement<C, T...>(PriorityTag<42>()) and modelsConceptList<T...>(typename C::BaseConceptList()); }

  // This is the full concept check. It's defined here in the
  // implementation namespace with 'constexpr bool' return type
  // because we need a forward declaration in order to use it
  // internally above.
  //
  // The actual interface function can then call this one and
  // return the result as std::integral_constant<bool,*> which
  // does not allow for a forward declaration because the return
  // type is deduced.
  template<class C, class... T>
  constexpr bool models()
  {
    return modelsConcept<C, T...>(PriorityTag<42>());
  }

} // namespace Dune::Concept::Impl

#endif // DOXYGEN

} // namespace Dune::Concept



/**
 * \brief Check if concept is modeled by given types
 *
 * This will check if the given concept is modeled by the given
 * list of types. This is true if the list of types models all
 * the base concepts that are refined by the given concept
 * and if it satisfies all additional requirements of the latter.
 *
 * Notice that a concept may be defined for a list of interacting types.
 * The function will check if the given list of types matches the requirements
 * on the whole list. It does not check if each individual type in the list
 * satisfies the concept.
 *
 * This concept check mechanism is inspired by the concept checking
 * facility in Eric Nieblers range-v3. For more information please
 * refer to the libraries project page https://github.com/ericniebler/range-v3
 * or this blog entry: http://ericniebler.com/2013/11/23/concept-checking-in-c11.
 * In fact the interface provided here is almost exactly the same as in range-v3.
 * However the implementation differs, because range-v3 uses its own meta-programming
 * library whereas our implementation is more straight forward.
 *
 * The result is returned as std::integral_constant<bool, ...> which
 * allows to nicely use this method with Hybrid::ifElse.
 *
 * \tparam C The concept to check
 * \tparam T The list of type to check against the concept
 *
 * \ingroup CxxConcepts
 */
template<class C, class... T>
constexpr auto models()
{
  return Std::bool_constant<Concept::Impl::models<C, T...>()>();
}



namespace Concept {

#ifndef DOXYGEN

namespace Impl {

  // #############################################################################
  // # All functions following here are implementation details for the
  // # for the tupleEntriesModel() function below.
  // #############################################################################

  template<class C, class Tuple>
  struct TupleEntriesModelHelper
  {
    template<class Accumulated, class T>
    struct AccumulateFunctor
    {
      using type = typename std::integral_constant<bool, Accumulated::value and models<C, T>()>;
    };
    using Result = typename ReduceTuple<AccumulateFunctor, Tuple, std::true_type>::type;
  };

} // namespace Dune::Concept::Impl

#endif // DOXYGEN


// #############################################################################
// # The method tupleEntriesModel() does the actual check if the types in a tuple
// # model a concept using the implementation details above.
// #############################################################################

template<class C, class Tuple>
constexpr auto tupleEntriesModel()
  -> typename Impl::TupleEntriesModelHelper<C, Tuple>::Result
{
  return {};
}

// #############################################################################
// # The following require*() functions are just helpers that allow to
// # propagate a failed check as substitution failure. This is useful
// # inside of a concept definition.
// #############################################################################

// Helper function for use in concept definitions.
// If the passed value b is not true, the concept will to be satisfied.
template<bool b, typename std::enable_if<b, int>::type = 0>
constexpr bool requireTrue()
{
  return true;
}

// Helper function for use in concept definitions.
template<class C, class... T, typename std::enable_if<models<C, T...>(), int>::type = 0>
constexpr bool requireConcept()
{
  return true;
}

// Helper function for use in concept definitions.
// This allows to avoid using decltype
template<class C, class... T, typename std::enable_if<models<C, T...>(), int>::type = 0>
constexpr bool requireConcept(T&&... /*t*/)
{
  return true;
}

// Helper function for use in concept definitions.
// This checks if the concept given as first type is modelled by all types in the tuple passed as argument
template<class C, class Tuple, typename std::enable_if<tupleEntriesModel<C, Tuple>(), int>::type = 0>
constexpr bool requireConceptForTupleEntries()
{
  return true;
}

// Helper function for use in concept definitions.
// If the first passed type is not convertible to the second, the concept will not be satisfied.
template<class From, class To,
  typename std::enable_if< std::is_convertible<From, To>::value, int>::type = 0>
constexpr bool requireConvertible()
{
  return true;
}

// Helper function for use in concept definitions.
// If passed argument is not convertible to the first passed type, the concept will not be satisfied.
template<class To, class From,
  typename std::enable_if< std::is_convertible<From, To>::value, int>::type = 0>
constexpr bool requireConvertible(const From&)
{
  return true;
}

// Helper function for use in concept definitions.
// This will always evaluate to true. If just allow
// to turn a type into an expression. The failure happens
// already during substitution for the type argument.
template<typename T>
constexpr bool requireType()
{
  return true;
}

// Helper function for use in concept definitions.
// If first passed type is not a base class of second type, the concept will not be satisfied.
template<class Base, class Derived,
  typename std::enable_if< std::is_base_of<Base, Derived>::value, int>::type = 0>
constexpr bool requireBaseOf()
{
  return true;
}

// Helper function for use in concept definitions.
// If first passed type is not a base class of first arguments type, the concept will not be satisfied.
template<class Base, class Derived,
  typename std::enable_if< std::is_base_of<Base, Derived>::value, int>::type = 0>
constexpr bool requireBaseOf(const Derived&)
{
  return true;
}

// Helper function for use in concept definitions.
// If the passed types are not the same, the concept will not be satisfied.
template<class A, class B,
  typename std::enable_if< std::is_same<A, B>::value, int>::type = 0>
constexpr bool requireSameType()
{
  return true;
}



} // namespace Dune::Concept

  /** @} */

} // namespace Dune




#endif // DUNE_COMMON_CONCEPT_HH
