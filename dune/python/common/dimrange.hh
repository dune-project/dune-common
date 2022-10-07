// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_DIMRANGE_HH
#define DUNE_PYTHON_COMMON_DIMRANGE_HH

#include <cstddef>

#include <tuple>
#include <type_traits>
#include <utility>

#if HAVE_DUNE_TYPETREE
#include <dune/typetree/compositenode.hh>
#include <dune/typetree/powernode.hh>
#endif // #if HAVE_DUNE_TYPETREE

#include <dune/python/common/fmatrix.hh>
#include <dune/python/common/fvector.hh>

namespace Dune
{

  namespace Python
  {

    namespace detail
    {

      template< class T >
      inline static constexpr T sum () noexcept
      {
        return static_cast< T >( 0 );
      }

      template< class T >
      inline static constexpr T sum ( T a ) noexcept
      {
        return a;
      }

      template< class T, class... U >
      inline static constexpr T sum ( T a, T b, U... c ) noexcept
      {
        return sum( a+b, c... );
      }


      template< class T, class Enable = void >
      struct DimRange;

      template< class T >
      struct DimRange< T, std::enable_if_t< std::is_arithmetic< T >::value > >
        : public std::integral_constant< std::size_t, 1 >
      {};

      template< class... T >
      struct DimRange< std::tuple< T... >, void >
        : public std::integral_constant< std::size_t, sum< std::size_t >( DimRange< T >::value... ) >
      {};

      template< class K, int n >
      struct DimRange< FieldVector< K, n >, void >
        : public std::integral_constant< std::size_t, n >
      {};

      template< class K, int m, int n >
      struct DimRange< FieldMatrix< K, m, n >, void >
        : public std::integral_constant< std::size_t, m*n >
      {};

#if HAVE_DUNE_TYPETREE
      template< class T >
      struct DimRange< T, std::enable_if_t< std::is_same< typename T::NodeTag, Dune::TypeTree::CompositeNodeTag >::value > >
        : public DimRange< typename T::ChildTypes >
      {};

      template< class T >
      struct DimRange< T, std::enable_if_t< std::is_same< typename T::NodeTag, Dune::TypeTree::PowerNodeTag >::value > >
        : public std::integral_constant< std::size_t, sum< int >( T::CHILDREN * DimRange< typename T::ChildType >::value ) >
      {};
#endif // #if HAVE_DUNE_TYPETREE

      template< class T >
      struct DimRange< T, std::enable_if_t< std::is_class< typename T::FiniteElement >::value > >
        : public DimRange< std::decay_t< decltype( std::declval< typename T::FiniteElement >().localBasis() ) > >
      {};

      template< class T >
      struct DimRange< T, std::enable_if_t< std::is_same< std::size_t, decltype( static_cast< std::size_t >( T::Traits::dimRange ) ) >::value > >
        : public std::integral_constant< std::size_t, static_cast< std::size_t >( T::Traits::dimRange ) >
      {};

    } // namespace detail



    // DimRange
    // --------

    template< class T >
    using DimRange = detail::DimRange< T >;

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_DIMRANGE_HH
