// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_TYPEUTILITIES_HH
#define DUNE_COMMON_TYPEUTILITIES_HH

#include <type_traits>
#include <tuple>


namespace Dune {

  /**
   * \file
   * \brief Utilities for type computations, constarining overloads, ...
   * \author Carsten Gräser
   */



  /**
   * \brief Helper to disable constructor as copy and move constructor
   *
   * \ingroup TypeUtilities
   *
   * Helper typedef to remove constructor with forwarding reference from
   * overload set for copy and move constructor or assignment.
   */
  template<class This, class... T>
  using disableCopyMove = typename std::enable_if<
    (not(std::is_same<This, typename std::tuple_element<0, std::tuple<typename std::decay<T>::type...> >::type >::value)
    and not(std::is_base_of<This, typename std::tuple_element<0, std::tuple<typename std::decay<T>::type...> >::type >::value)), int>::type;



} // namespace Dune



#endif // DUNE_COMMON_TYPEUTILITIES_HH
