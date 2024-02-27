// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_DIMRANGE_HH
#define DUNE_PYTHON_COMMON_DIMRANGE_HH

#include <cstddef>

#include <tuple>
#include <type_traits>
#include <utility>

#include <dune/python/common/fmatrix.hh>
#include <dune/python/common/fvector.hh>

namespace Dune
{

  namespace Python
  {

    namespace detail
    {

      template< class T, class Enable = void >
      struct DimRange;

      template< class T >
      struct DimRange< T, std::enable_if_t< std::is_arithmetic_v< T > > >
        : public std::integral_constant< std::size_t, 1 >
      {};

      template< class... T >
      struct DimRange< std::tuple< T... >, void >
        : public std::integral_constant< std::size_t, (DimRange< T >::value + ...) >
      {};

      template< class K, int n >
      struct DimRange< FieldVector< K, n >, void >
        : public std::integral_constant< std::size_t, n >
      {};

      template< class K, int m, int n >
      struct DimRange< FieldMatrix< K, m, n >, void >
        : public std::integral_constant< std::size_t, m*n >
      {};

    } // namespace detail



    // DimRange
    // --------

    template< class T >
    using DimRange = detail::DimRange< T >;

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_DIMRANGE_HH
