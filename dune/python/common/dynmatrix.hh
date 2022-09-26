// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifndef DUNE_PYTHON_COMMON_DYNMATRIX_HH
#define DUNE_PYTHON_COMMON_DYNMATRIX_HH

#include <string>
#include <tuple>
#include <utility>

#include <dune/common/dynmatrix.hh>

#include <dune/python/common/typeregistry.hh>
#include <dune/python/common/densematrix.hh>
#include <dune/python/pybind11/pybind11.h>
#include <dune/python/pybind11/operators.h>

namespace Dune
{

  namespace Python
  {

    template< class K >
    static void registerDynamicMatrix ( pybind11::handle scope )
    {
      typedef Dune::DynamicMatrix< K > DM;

      auto cls = insertClass< DM >( scope, "DynamicMatrix",
          GenerateTypeName("Dune::DynamicMatrix",MetaType<K>()),
          IncludeFiles{"dune/common/dynmatrix.hh"} ).first;

      cls.def( pybind11::init( [] () { return new DM(); } ) );

      cls.def( pybind11::init( [] ( pybind11::list rows ) {
            std::size_t numRows = rows.size();
            std::size_t numCols = (numRows > 0 ? rows[ 0 ].cast< pybind11::list >().size() : 0);

            DM *self = new DM( numRows, numCols, K( 0 ) );
            for( std::size_t i = 0; i < numRows; ++i )
            {
              pybind11::list row = rows[ i ].cast< pybind11::list >();
              std::size_t numCol = row.size();
              assert(numCols >= numCol); // dense matrix constructed with list having entries of different length
              for( std::size_t j = 0; j < numCol; ++j )
                (*self)[ i ][ j ] = row[ j ].template cast< K >();
            }
            return self;
          } ) );

      cls.def("__repr__",
          [] (const DM& m) {
            std::string repr = "Dune::DynamicMatrix:\n(";

            for(unsigned int r = 0; r < m.rows(); r++)
            {
              repr += "(";
              for (unsigned int c = 0; c < m.cols(); c++)
                repr += (c > 0 ? ", " : "") + std::to_string(m[r][c]);
              repr += std::string(")") + (r < m.rows() - 1 ? "\n" : "");
            }

            repr += ")";

            return repr;
          });

      registerDenseMatrix<DM>(cls);
    }

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_DYNMATRIX_HH
