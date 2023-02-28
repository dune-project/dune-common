// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_MPIHELPER_HH
#define DUNE_PYTHON_COMMON_MPIHELPER_HH

#include <config.h>

#include <vector>

#include <dune/common/parallel/communication.hh>
#include <dune/common/parallel/mpihelper.hh>

#include <dune/python/common/typeregistry.hh>
#include <dune/python/pybind11/pybind11.h>

namespace Dune
{

  namespace Python
  {

    // registerCommunication
    // -------------------------------

    template< class Comm, class... objects >
    inline static void registerCommunication ( pybind11::class_< Comm, objects... > cls )
    {
      using pybind11::operator""_a;

      cls.def_property_readonly( "rank", &Comm::rank );
      cls.def_property_readonly( "size", &Comm::size );

      cls.def( "barrier", &Comm::barrier );

      cls.def( "min", [] ( const Comm &self, double x ) { return self.min( x ); }, "x"_a );
      cls.def( "min", [] ( const Comm &self, std::vector< double > x ) { self.min( x.data(), x.size() ); return x; }, "x"_a );

      cls.def( "max", [] ( const Comm &self, double x ) { return self.max( x ); }, "x"_a );
      cls.def( "max", [] ( const Comm &self, std::vector<double> x ) { self.max( x.data(), x.size() ); return x; }, "x"_a );

      cls.def( "sum", [] ( const Comm &self, double x ) { return self.sum( x ); }, "x"_a );
      cls.def( "sum", [] ( const Comm &self, std::vector<double> x ) { self.sum( x.data(), x.size() ); return x; }, "x"_a );

      cls.def( "broadcast", [] ( const Comm &self, double x, int root ) { self.broadcast( &x, 1, root); return x; }, "x"_a, "root"_a );
      cls.def( "broadcast", [] ( const Comm &self, std::vector<double> x, int root ) { self.broadcast( x.data(), x.size(), root); return x; }, "x"_a, "root"_a );

      cls.def( "gather", [] ( const Comm &self, double x, int root )
          {
            // result will contain valid values only on rank=root
            std::vector< double > out;
            if( self.rank() == root )
              out.resize( self.size(), x );
            self.gather( &x, out.data(), 1, root);
            return out;
          }, "x"_a, "root"_a );
      cls.def( "scatter", [] ( const Comm &self, double x, int root )
          {
            double out = x;
            self.scatter( &x, &out, 1, root);
            return out;
          }, "x"_a, "root"_a );
    }

    inline static void registerCommunication ( pybind11::handle scope )
    {
      using Comm = Dune::Communication< Dune::MPIHelper::MPICommunicator >;

      auto typeName = GenerateTypeName( "Dune::Communication", "Dune::MPIHelper::MPICommunicator" );
      auto includes = IncludeFiles{ "dune/common/parallel/communication.hh", "dune/common/parallel/mpihelper.hh" };
      auto [ cls, notRegistered ] = insertClass< Comm >( scope, "Communication", typeName, includes );
      if( notRegistered )
        registerCommunication( cls );

      scope.attr( "comm" ) = pybind11::cast( Dune::MPIHelper::getCommunication() );
    }

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_MPIHELPER_HH
