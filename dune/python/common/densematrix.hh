// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_DENSEMATRIX_HH
#define DUNE_PYTHON_COMMON_DENSEMATRIX_HH

#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include <dune/python/pybind11/extensions.h>
#include <dune/python/pybind11/operators.h>
#include <dune/python/pybind11/pybind11.h>

namespace Dune
{

  namespace Python
  {

    // registerDenseMatrix
    // -------------------

    template< class Matrix >
    void registerDenseMatrix ( pybind11::class_< Matrix > cls )
    {
      typedef typename Matrix::field_type field_type;
      typedef typename Matrix::row_type row_type;
      typedef typename Matrix::row_reference row_reference;

      cls.def( "__getitem__", [] ( Matrix &self, std::size_t i ) -> row_reference {
          if( i < self.mat_rows() )
            return self[ i ];
          else
            throw pybind11::index_error();
        }, (std::is_reference< row_reference >::value ? pybind11::return_value_policy::reference : pybind11::return_value_policy::move), pybind11::keep_alive< 0, 1 >() );

      cls.def( "__setitem__", [] ( Matrix &self, std::size_t i, pybind11::object l ) {
          if( i < self.mat_rows() )
          {
            row_type v = l.cast< row_type >();
            std::size_t size = std::min( self.mat_cols(), v.size() );

            for( std::size_t j = 0; j < size; ++j )
              self[ i ][ j ] = v[ j ];
          }
          else
            throw pybind11::index_error();
        } );

      cls.def( "__len__", [] ( const Matrix &self ) -> std::size_t { return self.size(); } );

      cls.def( "invert", [] ( Matrix &self ) { self.invert(); } );

      cls.def( pybind11::self += pybind11::self );
      cls.def( pybind11::self -= pybind11::self );
      cls.def( pybind11::self *= field_type() );
      cls.def( pybind11::self /= field_type() );

      cls.def( pybind11::self == pybind11::self );
      cls.def( pybind11::self != pybind11::self );

      cls.def_property_readonly( "frobenius_norm", [] ( const Matrix &self ) { return self.frobenius_norm(); } );
      cls.def_property_readonly( "frobenius_norm2", [] ( const Matrix &self ) { return self.frobenius_norm2(); } );
      cls.def_property_readonly( "infinity_norm", [] ( const Matrix &self ) { return self.infinity_norm(); } );
      cls.def_property_readonly( "infinity_norm_real", [] ( const Matrix &self ) { return self.infinity_norm_real(); } );

      cls.def_property_readonly( "rows", [] ( const Matrix &self ) { return self.mat_rows(); } );
      cls.def_property_readonly( "cols", [] ( const Matrix &self ) { return self.mat_cols(); } );
    }

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_DENSEMATRIX_HH
