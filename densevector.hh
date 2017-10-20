// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_PYTHON_COMMON_DENSEVECTOR_HH
#define DUNE_PYTHON_COMMON_DENSEVECTOR_HH

#include <string>
#include <tuple>
#include <type_traits>

#include <dune/common/std/utility.hh>

#include <dune/python/common/vector.hh>
#include <dune/python/pybind11/pybind11.h>
#include <dune/python/pybind11/operators.h>

namespace Dune
{

  namespace CorePy
  {

    namespace detail
    {

      template< class T, class... options >
      inline static std::enable_if_t< std::is_copy_constructible< T >::value >
      registerCopyingDenseVectorMethods ( pybind11::class_< T, options... > cls )
      {
        using ValueType = typename T::value_type;

        using pybind11::operator""_a;

        cls.def( pybind11::self + pybind11::self );
        cls.def( pybind11::self - pybind11::self );

        cls.def( "__mul__", [] ( const T &self, ValueType x ) { T copy( self ); copy *= x; return copy; }, "x"_a );
        cls.def( "__div__", [] ( const T &self, ValueType x ) { T copy( self ); copy /= x; return copy; }, "x"_a );

        cls.def( "__add__", [] ( T &self, pybind11::list x ) { return self + x.cast< T >(); }, "x"_a );
        cls.def( "__sub__", [] ( T &self, pybind11::list x ) { return self - x.cast< T >(); }, "x"_a );
        cls.def( "__mul__", [] ( T &self, pybind11::list x ) { return self * x.cast< T >(); }, "x"_a );

        cls.def( "__radd__", [] ( T &self, pybind11::list x ) { return x.cast< T >() + self; }, "x"_a );
        cls.def( "__rsub__", [] ( T &self, pybind11::list x ) { return x.cast< T >() - self; }, "x"_a );
        cls.def( "__rmul__", [] ( T &self, pybind11::list x ) { return x.cast< T >() * self; }, "x"_a );
      }

      template< class T, class... options >
      inline static std::enable_if_t< !std::is_copy_constructible< T >::value >
      registerCopyingDenseVectorMethods ( pybind11::class_< T, options... > cls )
      {}

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

      pybind11::implicitly_convertible< pybind11::list, T >();
    }

  } // namespace CorePy

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_DENSEVECTOR_HH
