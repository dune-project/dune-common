// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_FVECTOR_HH
#define DUNE_PYTHON_COMMON_FVECTOR_HH

#include <cstddef>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

#include <dune/common/fvector.hh>
#include <dune/common/classname.hh>

#include <dune/python/common/typeregistry.hh>
#include <dune/python/common/densevector.hh>
#include <dune/python/common/string.hh>
#include <dune/python/pybind11/pybind11.h>
#include <dune/python/pybind11/operators.h>

namespace Dune
{

  namespace Python
  {

    // to_string
    // ---------

    template< class K, int size >
    inline static std::string to_string ( const FieldVector< K, size > &x )
    {
      return "(" + join( ", ", [] ( auto &&x ) { return to_string( x ); }, x.begin(), x.end() ) + ")";
    }



    // registerFieldVector
    // -------------------
    template< class K, int size, class ...options >
    void registerFieldVector ( pybind11::handle scope, pybind11::class_<Dune::FieldVector<K,size>, options...> cls )
    {
      using pybind11::operator""_a;

      typedef Dune::FieldVector<K, size> FV;
      cls.def( pybind11::init( [] () { return new FV(); } ) );

      if( size == 1 )
      {
        cls.def( pybind11::init( [] ( int a ) { return new FV( K( a ) ); } ) );
        cls.def( pybind11::init( [] ( K a ) { return new FV( a ); } ) );
        cls.def( "__float__", [] ( const FV &self ) { return self[ 0 ]; } );
        pybind11::implicitly_convertible< int, FV >();
        pybind11::implicitly_convertible< K, FV >();
      }

      cls.def( pybind11::init( [] ( pybind11::buffer x ) {
          pybind11::buffer_info info = x.request();
          if( info.format != pybind11::format_descriptor< K >::format() )
            throw pybind11::value_error( "Incompatible buffer format." );
          if( info.ndim != 1 )
            throw pybind11::value_error( "Only one-dimensional buffers can be converted into FieldVector." );
          const ssize_t stride = info.strides[ 0 ] / sizeof( K );
          const ssize_t sz = std::min( static_cast< ssize_t >( size ), info.shape[ 0 ] );

          FV *self = new FV( K( 0 ) );
          for( ssize_t i = 0; i < sz; ++i )
            (*self)[ i ] = static_cast< K * >( info.ptr )[ i*stride ];
          return self;
        } ), "x"_a );

      cls.def( pybind11::init( [] ( pybind11::tuple x ) {
          FV *self = new FV( K( 0 ) );
          const std::size_t sz = std::min( static_cast< std::size_t >( size ), x.size() );
          // should this fail in case the sizes do not match?
          for( std::size_t i = 0; i < sz; ++i )
            (*self)[ i ] = x[ i ].template cast< K >();
          return self;
        } ), "x"_a );

      cls.def( pybind11::init( [] ( pybind11::list x ) {
            FV *self = new FV( K( 0 ) );
            const std::size_t sz = std::min( static_cast< std::size_t >( size ), x.size() );
            // should this fail in case the sizes do not match?
            for( std::size_t i = 0; i < sz; ++i )
              (*self)[ i ] = x[ i ].template cast< K >();
            return self;
          } ), "x"_a );

      cls.def( pybind11::init( [] ( pybind11::args args ) {
            FV *self = new FV( K( 0 ) );
            const std::size_t sz = std::min( static_cast< std::size_t >( size ), args.size() );
            // should this fail in case the sizes do not match?
            for( std::size_t i = 0; i < sz; ++i )
              (*self)[ i ] = args[ i ].template cast< K >();
            return self;
          } ) );

      pybind11::implicitly_convertible< pybind11::args, FV >();
      pybind11::implicitly_convertible< pybind11::buffer, FV >();

      cls.def("copy", [](FV& , pybind11::args l) {
            FV v(K(0));
            const std::size_t sz = std::min(v.size(), l.size());
            // should this fail in case the sizes do not match?
            for (std::size_t i = 0; i < sz; ++i)
              v[i] = l[i].template cast<K>();
            return v;
          });

      cls.def( "__str__", [] ( const FV &self ) { return to_string( self ); } );
      cls.def( "__repr__", [] ( const FV &self ) {
          return "Dune::FieldVector<"+to_string(size)+">"+to_string(self);
          } );

      cls.def_buffer( [] ( FV &self ) -> pybind11::buffer_info {
          return pybind11::buffer_info(
              &self[ 0 ],                                         /* Pointer to buffer */
              sizeof( K ),                                        /* Size of one scalar */
              pybind11::format_descriptor< K >::format(),         /* Python struct-style format descriptor */
              1,                                                  /* Number of dimensions */
              { size },                                           /* Buffer dimensions */
              /* Strides (in bytes) for each index */
              {
                static_cast< std::size_t >( reinterpret_cast< char * >( &self[ 1 ] ) - reinterpret_cast< char * >( &self[ 0 ] ) ) } );
              }
        );

      registerDenseVector< FV >( cls );
    }

    template< class K, int size >
    void registerFieldVector ( pybind11::handle scope, std::integral_constant< int, size > = {} )
    {
      typedef Dune::FieldVector<K, size> FV;

      std::string fname = className<K>();
      auto entry = insertClass<FV>(scope, "FieldVector_"+fname+"_"+std::to_string(size), pybind11::buffer_protocol(),
        GenerateTypeName("Dune::FieldVector",MetaType<K>(),size),IncludeFiles{"dune/common/fvector.hh"}
        );
      if (!entry.second)
        return;
      registerFieldVector(scope, entry.first);
    }

    template<class K, int... size>
    void registerFieldVector(pybind11::handle scope, std::integer_sequence<int, size...>)
    {
      std::ignore = std::make_tuple((registerFieldVector<K>(scope, std::integral_constant<int, size>()), size)...);
    }

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_FVECTOR_HH
