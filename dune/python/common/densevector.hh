// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_DENSEVECTOR_HH
#define DUNE_PYTHON_COMMON_DENSEVECTOR_HH

#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include <dune/python/common/vector.hh>
#include <dune/python/pybind11/pybind11.h>
#include <dune/python/pybind11/operators.h>

namespace Dune
{

  namespace Python
  {

    namespace detail
    {

      // registerScalarCopyingDenseVectorMethods
      // ---------------------------------------

      template< class T, class... options >
      inline static std::enable_if_t< std::is_copy_constructible< T >::value && (T::dimension == 1) >
      registerScalarCopyingDenseVectorMethods ( pybind11::class_< T, options... > cls, PriorityTag< 2 > )
      {
        using ValueType = typename T::value_type;

        cls.def( "__add__", [] ( const T &self, int a ) { T *copy = new T( self ); (*copy)[ 0 ] += ValueType( a ); return copy; } );
        cls.def( "__add__", [] ( const T &self, const ValueType &a ) { T *copy = new T( self ); (*copy)[ 0 ] += a; return copy; } );
        cls.def( "__sub__", [] ( const T &self, int a ) { T *copy = new T( self ); (*copy)[ 0 ] -= ValueType( a ); return copy; } );
        cls.def( "__sub__", [] ( const T &self, const ValueType &a ) { T *copy = new T( self ); (*copy)[ 0 ] -= a; return copy; } );

        cls.def( "__radd__", [] ( const T &self, int a ) { T *copy = new T( self ); (*copy)[ 0 ] = ValueType( a ) + (*copy)[ 0 ]; return copy; } );
        cls.def( "__radd__", [] ( const T &self, const ValueType &a ) { T *copy = new T( self ); (*copy)[ 0 ] = a + (*copy)[ 0 ]; return copy; } );
        cls.def( "__rsub__", [] ( const T &self, int a ) { T *copy = new T( self ); (*copy)[ 0 ] = ValueType( a ) - (*copy)[ 0 ]; return copy; } );
        cls.def( "__rsub__", [] ( const T &self, const ValueType &a ) { T *copy = new T( self ); (*copy)[ 0 ] = a - (*copy)[ 0 ]; return copy; } );
      }

      template< class T, class... options >
      inline static std::enable_if_t< std::is_copy_constructible< T >::value >
      registerScalarCopyingDenseVectorMethods ( pybind11::class_< T, options... > cls, PriorityTag< 1 > )
      {
        using ValueType = typename T::value_type;

        cls.def( "__add__", [] ( pybind11::object self, int a ) {
            if( a != 0 )
              throw pybind11::value_error( "Cannot add " + std::to_string( a ) + " to multidimensional dense vector." );
            return self;
          } );
        cls.def( "__sub__", [] ( pybind11::object self, int a ) {
            if( a != 0 )
              throw pybind11::value_error( "Cannot subtract " + std::to_string( a ) + " from multidimensional dense vector." );
            return self;
          } );

        cls.def( "__radd__", [] ( pybind11::object self, int a ) {
            if( a != 0 )
              throw pybind11::value_error( "Cannot add multidimensional dense vector to " + std::to_string( a ) + "." );
            return self;
          } );
        cls.def( "__rsub__", [] ( const T &self, int a ) {
            if( a != 0 )
              throw pybind11::value_error( "Cannot subtract multidimensional dense vector from " + std::to_string( a ) + "." );
            T *copy = new T( self ); *copy *= ValueType( -1 ); return copy;
          } );
      }

      template< class T, class... options >
      inline static void registerScalarCopyingDenseVectorMethods ( pybind11::class_< T, options... > cls, PriorityTag< 0 > )
      {}

      template< class T, class... options >
      inline static void registerScalarCopyingDenseVectorMethods ( pybind11::class_< T, options... > cls )
      {
        registerScalarCopyingDenseVectorMethods ( cls, PriorityTag< 42 >() );
      }




      // registerCopyingDenseVectorMethods
      // ---------------------------------

      template< class T, class... options >
      inline static std::enable_if_t< std::is_copy_constructible< T >::value >
      registerCopyingDenseVectorMethods ( pybind11::class_< T, options... > cls, PriorityTag< 1 > )
      {
        using ValueType = typename T::value_type;

        using pybind11::operator""_a;

        cls.def( "__pos__", [] ( pybind11::object self ) { return self; } );
        cls.def( "__neg__", [] ( T &self ) { T *copy = new T( self ); *copy *= ValueType( -1 ); return copy; } );

        cls.def( pybind11::self + pybind11::self );
        cls.def( pybind11::self - pybind11::self );

        cls.def( "__add__", [] ( T &self, pybind11::list x ) { return self + x.cast< T >(); }, "x"_a );
        cls.def( "__sub__", [] ( T &self, pybind11::list x ) { return self - x.cast< T >(); }, "x"_a );

        cls.def( "__radd__", [] ( T &self, pybind11::list x ) { return x.cast< T >() + self; }, "x"_a );
        cls.def( "__rsub__", [] ( T &self, pybind11::list x ) { return x.cast< T >() - self; }, "x"_a );

        cls.def( "__mul__", [] ( const T &self, ValueType x ) { T *copy = new T( self ); *copy *= x; return copy; }, "x"_a );
        cls.def( "__div__", [] ( const T &self, ValueType x ) { T *copy = new T( self ); *copy /= x; return copy; }, "x"_a );
        cls.def( "__truediv__", [] ( const T &self, ValueType x ) { T *copy = new T( self ); *copy /= x; return copy; }, "x"_a );

        cls.def( "__rmul__", [] ( const T &self, ValueType x ) { T *copy = new T( self ); *copy *= x; return copy; }, "x"_a );
      }

      template< class T, class... options >
      inline static void registerCopyingDenseVectorMethods ( pybind11::class_< T, options... > cls, PriorityTag< 0 > )
      {}

      template< class T, class... options >
      inline static void registerCopyingDenseVectorMethods ( pybind11::class_< T, options... > cls )
      {
        registerCopyingDenseVectorMethods ( cls, PriorityTag< 42 >() );
      }

    } // namespace detail



    // registerDenseVector
    // -------------------

    template< class T, class... options >
    inline static void registerDenseVector ( pybind11::class_< T, options... > cls )
    {
      using ValueType = typename T::value_type;

      using pybind11::operator""_a;

      cls.def( "assign", [] ( T &self, const T &x ) { self = x; }, "x"_a );

      cls.def( "__getitem__", [] ( const T &self, std::size_t i ) -> ValueType {
          if( i < self.size() )
            return self[ i ];
          else
            throw pybind11::index_error();
        }, "i"_a );

      cls.def( "__setitem__", [] ( T &self, std::size_t i, ValueType x ) {
          if( i < self.size() )
            self[ i ] = x;
          else
            throw pybind11::index_error();
        }, "i"_a, "x"_a );

      cls.def( "__len__", [] ( const T &self ) -> std::size_t { return self.size(); } );

      cls.def( pybind11::self += pybind11::self );
      cls.def( pybind11::self -= pybind11::self );

      cls.def( pybind11::self == pybind11::self );
      cls.def( pybind11::self != pybind11::self );

      cls.def( pybind11::self += ValueType() );
      cls.def( pybind11::self -= ValueType() );
      cls.def( pybind11::self *= ValueType() );
      cls.def( pybind11::self /= ValueType() );

      detail::registerOneTensorInterface( cls );
      detail::registerCopyingDenseVectorMethods( cls );
      detail::registerScalarCopyingDenseVectorMethods( cls );

      pybind11::implicitly_convertible< pybind11::list, T >();
    }

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_DENSEVECTOR_HH
