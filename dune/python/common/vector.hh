// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_TYPE_TRAITS_HH
#define DUNE_PYTHON_COMMON_TYPE_TRAITS_HH

#include <type_traits>
#include <utility>

#include <dune/common/ftraits.hh>
#include <dune/common/rangeutilities.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/std/type_traits.hh>

#include <dune/python/pybind11/numpy.h>
#include <dune/python/pybind11/pybind11.h>

namespace Dune
{

  // External Forward Declarations
  // -----------------------------

  template< class V >
  class DenseVector;

  template< class K, int n >
  class FieldVector;

  template< class K, int m, int n >
  class FieldMatrix;

  namespace Imp
  {

    template< class B, class A >
    class block_vector_unmanaged;

    template< class B, class A >
    class compressed_block_vector_unmanaged;

  } // namespace Imp



  namespace Python
  {

    // IsDenseVector
    // -------------

    template< class T, class = void >
    struct IsDenseVector
      : public std::false_type
    {};

    template< class T >
    struct IsDenseVector< T, std::enable_if_t< std::is_base_of< Dune::DenseVector< T >, T >::value > >
      : public std::true_type
    {};



    // IsBlockVector
    // -------------

    template< class T, class = void >
    struct IsBlockVector
      : public std::false_type
    {};

    template< class T >
    struct IsBlockVector< T, std::enable_if_t< std::is_base_of< Imp::block_vector_unmanaged< typename T::block_type, typename T::allocator_type >, T >::value > >
      : public std::true_type
    {};

    template< class T >
    struct IsBlockVector< T, std::enable_if_t< std::is_base_of< Imp::compressed_block_vector_unmanaged< typename T::block_type, typename T::allocator_type >, T >::value > >
      : public std::true_type
    {};



    // IsOneTensor
    // -----------

    template< class T, class = void >
    struct IsOneTensor
      : public std::false_type
    {};

    template< class T >
    struct IsOneTensor< T, std::enable_if_t< IsDenseVector< T >::value > >
      : public std::true_type
    {};

    template< class T >
    struct IsOneTensor< T, std::enable_if_t< IsBlockVector< T >::value && IsOneTensor< typename T::block_type >::value > >
      : public std::true_type
    {};



    namespace detail
    {

      // registerOneTensorInterface
      // --------------------------

      template< class T, class... options >
      inline static auto registerOneTensorInterface ( pybind11::class_< T, options... > cls, PriorityTag< 1 > )
        -> std::enable_if_t< IsOneTensor< T >::value >
      {
        cls.def( "__mul__", [] ( const T &self, const T &other ) { return self * other; } );
        cls.def( "__rmul__", [] ( const T &self, const T &other ) { return self * other; } );

        cls.def_property_readonly( "one_norm", [] ( const T &self ) { return self.one_norm(); } );
        cls.def_property_readonly( "one_norm_real", [] ( const T &self ) { return self.one_norm_real(); } );
        cls.def_property_readonly( "two_norm", [] ( const T &self ) { return self.two_norm(); } );
        cls.def_property_readonly( "two_norm2", [] ( const T &self ) { return self.two_norm2(); } );
        cls.def_property_readonly( "infinity_norm", [] ( const T &self ) { return self.infinity_norm(); } );
        cls.def_property_readonly( "infinity_norm_real", [] ( const T &self ) { return self.infinity_norm_real(); } );
      }

      template< class T, class... options >
      inline static void registerOneTensorInterface ( pybind11::class_< T, options... > cls, PriorityTag< 0 > )
      {}

      template< class T, class... options >
      inline static void registerOneTensorInterface ( pybind11::class_< T, options... > cls )
      {
        registerOneTensorInterface( cls, PriorityTag< 42 >() );
      }

    } // namespace detail



    // FixedTensorTraits
    // -----------------

    template< class T, class = void >
    struct FixedTensorTraits;

    template< class T >
    struct FixedTensorTraits< T, std::enable_if_t< IsNumber< T >::value > >
    {
      static constexpr std::array< ssize_t, 0 > shape () noexcept { return {{}}; }
    };

    template< class K, int n >
    struct FixedTensorTraits< FieldVector< K, n >, void >
    {
      static constexpr std::array< ssize_t, 1 > shape () noexcept { return {{ n }}; }
    };

    template< class K, int m, int n >
    struct FixedTensorTraits< FieldMatrix< K, m, n >, void >
    {
      static constexpr std::array< ssize_t, 2 > shape () noexcept { return {{ m, n }}; }
    };



    // extendArray
    // -----------

    template< std::size_t... i, class T, class... X >
    inline static constexpr auto extendArray ( std::index_sequence< i... >, const std::array< T, sizeof...( i ) > &array, X &&... x )
      -> std::enable_if_t< std::conjunction< std::is_convertible< X, T >... >::value, std::array< T, sizeof...( i )+sizeof...( X ) > >
    {
      return {{ array[ i ]..., std::forward< X >( x )... }};
    }

    template< class T, std::size_t n, class... X >
    inline static constexpr std::array< T, n+sizeof...( X ) > extendArray ( const std::array< T, n > &array, X &&... x )
    {
      return extendArray( std::make_index_sequence< n >(), array, std::forward< X >( x )... );
    }



    // getFixedTensor
    // --------------

    template< std::size_t... k, class X, class Y, std::size_t n, class... I >
    inline static auto getFixedTensor ( std::index_sequence< k... >, X &x, const Y &y, std::array< ssize_t, n > j, I... i )
      -> std::enable_if_t< (sizeof...( k ) == n) >
    {
      x = y( j[ k ]..., i... );
    }

    template< std::size_t... k, class X, class Y, std::size_t n, class... I >
    inline static auto getFixedTensor ( std::index_sequence< k... >, X &x, const Y &y, std::array< ssize_t, n > j, I... i )
      -> std::enable_if_t< (sizeof...( k ) < n) >
    {
      ssize_t &it = j[ sizeof...( k ) ];
      ssize_t end = it;
      for( it = 0; it < end; ++it )
        getFixedTensor( std::index_sequence< k..., sizeof...( k ) >(), x[ it ], y, j, i... );
    }

    template< class X, class Y, class... I >
    inline static auto getFixedTensor ( X &x, const Y &y, I... i )
    {
      getFixedTensor( std::index_sequence<>(), x, y, FixedTensorTraits< X >::shape(), i... );
    }



    // setFixedTensor
    // --------------

    template< std::size_t... k, class X, class Y, std::size_t n, class... I >
    inline static auto setFixedTensor ( std::index_sequence< k... >, const X &x, Y &y, std::array< ssize_t, n > j, I... i )
      -> std::enable_if_t< (sizeof...( k ) == n) >
    {
      y( j[ k ]..., i... ) = x;
    }

    template< std::size_t... k, class X, class Y, std::size_t n, class... I >
    inline static auto setFixedTensor ( std::index_sequence< k... >, const X &x, Y &y, std::array< ssize_t, n > j, I... i )
      -> std::enable_if_t< (sizeof...( k ) < n) >
    {
      ssize_t &it = j[ sizeof...( k ) ];
      ssize_t end = it;
      for( it = 0; it < end; ++it )
        setFixedTensor( std::index_sequence< k..., sizeof...( k ) >(), x[ it ], y, j, i... );
    }

    template< class X, class Y, class... I >
    inline static auto setFixedTensor ( const X &x, Y &y, I... i )
    {
      setFixedTensor( std::index_sequence<>(), x, y, FixedTensorTraits< X >::shape(), i... );
    }



    // vectorize
    // ---------

    template< class F, class Y, class X >
    inline static pybind11::object vectorize ( F &&f, Y (*)( X ), pybind11::array_t< typename FieldTraits< std::decay_t< X > >::field_type > xArray )
    {
      const auto xShape = FixedTensorTraits< std::decay_t< X > >::shape();

      auto x = xArray.unchecked();
      if( (std::size_t)x.ndim() < xShape.size() )
        throw pybind11::value_error( "Tensor has too few dimensions" );

      for( auto i : range( xShape.size() ) )
      {
        if( x.shape( i ) != xShape[ i ] )
          throw pybind11::value_error( "Tensor has wrong shape" );
      }

      std::decay_t< X > xi;
      if( (xShape.size() > 0) && (x.ndim() == xShape.size()) )
      {
        getFixedTensor( xi, x );
        return pybind11::cast( f( xi ) );
      }
      else if( x.ndim() == xShape.size() + 1 )
      {
        const ssize_t size = x.shape( xShape.size() );
        const auto yShape = extendArray( FixedTensorTraits< std::decay_t< Y > >::shape(), size );

        pybind11::array_t< typename FieldTraits< std::decay_t< Y > >::field_type > yArray( yShape );
        auto y = yArray.template mutable_unchecked< yShape.size() >();

        for( auto i : range( size ) )
        {
          getFixedTensor( xi, x, i );
          setFixedTensor( f( xi ), y, i );
        }
        return std::move(yArray);
      }
      else
        throw pybind11::value_error( "Tensor has too many dimensions" );
    }

    template< class F, class X >
    inline static auto vectorize ( F &&f, pybind11::array_t< X > xArray )
      -> decltype( vectorize( std::forward< F >( f ), static_cast< pybind11::detail::function_signature_t< F > * >( nullptr ), std::move( xArray ) ) )
    {
      return vectorize( std::forward< F >( f ), static_cast< pybind11::detail::function_signature_t< F > * >( nullptr ), std::move( xArray ) );
    }

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_TYPE_TRAITS_HH
