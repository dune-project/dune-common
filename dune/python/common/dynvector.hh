// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifndef DUNE_PYTHON_COMMON_DYNVECTOR_HH
#define DUNE_PYTHON_COMMON_DYNVECTOR_HH

#include <string>
#include <tuple>
#include <utility>

#include <dune/common/dynvector.hh>

#include <dune/python/common/typeregistry.hh>
#include <dune/python/common/densevector.hh>
#include <dune/python/pybind11/pybind11.h>
#include <dune/python/pybind11/operators.h>

namespace Dune
{

  namespace Python
  {

    template< class K >
    void registerDynamicVector ( pybind11::handle scope )
    {
      using pybind11::operator""_a;

      typedef Dune::DynamicVector< K > DV;

      auto cls = insertClass< DV >( scope, "DynamicVector",
          GenerateTypeName("Dune::DynamicVector",MetaType<K>()),
          IncludeFiles{"dune/common/dynvector.hh"} ).first;

      cls.def( pybind11::init( [] () { return new DV(); } ) );

      cls.def( pybind11::init( [] ( pybind11::list x ) {
            std::size_t size = x.size();
            DV *self = new DV( size, K( 0 ) );
            for( std::size_t i = 0; i < size; ++i )
              (*self)[ i ] = x[ i ].template cast< K >();
            return self;
          } ), "x"_a );

      cls.def("__repr__",
          [] (const DV &v) {
            std::string repr = "Dune::DynamicVector: (";

            for (std::size_t i = 0; i < v.size(); ++i)
              repr += (i > 0 ? ", " : "") + std::to_string(v[i]);

            repr += ")";

            return repr;
          });

      registerDenseVector<DV>(cls);
    }

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_DYNVECTOR_HH
