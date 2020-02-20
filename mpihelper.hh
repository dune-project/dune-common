// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_PYTHON_COMMON_MPIHELPER_HH
#define DUNE_PYTHON_COMMON_MPIHELPER_HH

#include <config.h>

#include <dune/common/parallel/communication.hh>
#include <dune/common/parallel/mpihelper.hh>

#include <dune/python/common/typeregistry.hh>
#include <dune/python/pybind11/pybind11.h>

namespace Dune
{

  namespace Python
  {

    // registerCollectiveCommunication
    // -------------------------------

    template< class Comm, class... objects >
    inline static void registerCollectiveCommunication ( pybind11::class_< Comm, objects... > cls )
    {
      using pybind11::operator""_a;

      cls.def_property_readonly( "rank", &Comm::rank );
      cls.def_property_readonly( "size", &Comm::size );

      cls.def( "barrier", &Comm::barrier );

      cls.def( "min", [] ( const Comm &self, double x ) { return self.min( x ); }, "x"_a );
      cls.def( "max", [] ( const Comm &self, double x ) { return self.max( x ); }, "x"_a );
      cls.def( "sum", [] ( const Comm &self, double x ) { return self.sum( x ); }, "x"_a );
    }

    inline static void registerCollectiveCommunication ( pybind11::handle scope )
    {
      typedef Dune::CollectiveCommunication< Dune::MPIHelper::MPICommunicator > Comm;

      auto typeName = GenerateTypeName( "Dune::CollectiveCommunication", "Dune::MPIHelper::MPICommunicator" );
      auto includes = IncludeFiles{ "dune/common/parallel/collectivecommunication.hh", "dune/common/parallel/mpihelper.hh" };
      auto clsComm = insertClass< Comm >( scope, "CollectiveCommunication", typeName, includes );
      if( clsComm.second )
        registerCollectiveCommunication( clsComm.first );

      scope.attr( "comm" ) = pybind11::cast( Dune::MPIHelper::getCollectiveCommunication() );
    }

  } // namespace Python

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_MPIHELPER_HH
