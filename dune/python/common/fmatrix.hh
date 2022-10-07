// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_FMATRIX_HH
#define DUNE_PYTHON_COMMON_FMATRIX_HH

#include <cstddef>

#include <algorithm>
#include <string>
#include <tuple>
#include <utility>

#include <dune/common/fmatrix.hh>
#include <dune/common/classname.hh>

#include <dune/python/common/typeregistry.hh>
#include <dune/python/common/fmatrix.hh>
#include <dune/python/common/densematrix.hh>
#include <dune/python/common/string.hh>
#include <dune/python/pybind11/pybind11.h>
#include <dune/python/pybind11/operators.h>

namespace Dune
{

  namespace Python
  {

    // registerFieldMatrix
    // -------------------

    template< class K, int m, int n, class ...options >
    void registerFieldMatrix ( pybind11::handle scope, pybind11::class_<Dune::FieldMatrix<K,m,n>, options...> cls )
    {
      typedef Dune::FieldMatrix< K, m, n > FM;
      using pybind11::operator""_a;

      if( (m == 1) && (n == 1) )
      {
        cls.def( pybind11::init( [] ( int a ) { return new FM( K( a ) ); } ) );
        cls.def( pybind11::init( [] ( K a ) { return new FM( a ); } ) );
        cls.def( "__float__", [] ( const FM &self ) { return self[ 0 ][ 0 ]; } );
        pybind11::implicitly_convertible< int, FM >();
        pybind11::implicitly_convertible< K, FM >();
      }

      cls.def( pybind11::init( [] () { return new FM( K( 0 ) ); } ) );

      cls.def( pybind11::init( [] ( pybind11::list rows ) {
          FM *self = new FM( K( 0 ) );

          const std::size_t numRows = std::min( static_cast< std::size_t >( m ), rows.size() );
          for( std::size_t i = 0; i < numRows; ++i )
            (*self)[ i ] = pybind11::cast< Dune::FieldVector< K, n > >( rows[ i ] );
          return self;
        } ) );

      pybind11::implicitly_convertible< pybind11::list, FM >();

      cls.def( "__str__", [] ( const FM &self ) {
            std::string s = "(";
            for( int i = 0; i < m; ++i )
            {
              s += (i > 0 ? "\n(" : "(");
              for( int j = 0; j < n; ++j )
                s += (j > 0 ? ", " : "") + std::to_string( self[ i ][ j ] );
              s += std::string( ") ");
            }
            return s += ")";
          });
      cls.def( "__repr__", [] ( const FM &self ) {
          return "Dune::FieldMatrix<"+to_string(m)+","+to_string(n)+">(...)";
          } );

      cls.def_buffer( [] ( FM &self ) -> pybind11::buffer_info {
          return pybind11::buffer_info(
              &self[ 0 ][ 0 ],                          /* Pointer to buffer */
              sizeof( K ),                              /* Size of one scalar */
              pybind11::format_descriptor< K >::value,  /* Python struct-style format descriptor */
              2,                                        /* Number of dimensions */
              { m, n },                                 /* Buffer dimensions */
              /* Strides (in bytes) for each index */
              {
                static_cast< std::size_t >( reinterpret_cast< char * >( &self[ 1 ][ 0 ] ) - reinterpret_cast< char * >( &self[ 0 ][ 0 ] ) ),
                static_cast< std::size_t >( reinterpret_cast< char * >( &self[ 0 ][ 1 ] ) - reinterpret_cast< char * >( &self[ 0 ][ 0 ] ) )
              }
            );
        } );

      registerDenseMatrix< FM >( cls );
    }

    template< class K, int m, int n >
    inline static void registerFieldMatrix ( pybind11::handle scope )
    {
      typedef Dune::FieldMatrix< K, m, n > FM;

      std::string fname = className<K>();
      auto entry = insertClass<FM>( scope, "FieldMatrix_"+fname+"_"+std::to_string(m)+"_"+std::to_string(n), pybind11::buffer_protocol(),
          GenerateTypeName("Dune::FieldMatrix",Dune::MetaType<K>(),m,n), IncludeFiles{"dune/common/fmatrix.hh"}
        );
      if (!entry.second)
        return;
      registerFieldMatrix( scope, entry.first );
    }
  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_FMATRIX_HH
