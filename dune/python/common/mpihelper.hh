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
      cls.def( "max", [] ( const Comm &self, double x ) { return self.max( x ); }, "x"_a );
      cls.def( "sum", [] ( const Comm &self, double x ) { return self.sum( x ); }, "x"_a );
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
