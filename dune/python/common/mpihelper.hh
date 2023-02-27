// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_MPIHELPER_HH
#define DUNE_PYTHON_COMMON_MPIHELPER_HH

#include <config.h>

#include <vector>

#include <dune/common/parallel/communication.hh>
#include <dune/common/parallel/mpicommunication.hh>
#include <dune/common/parallel/mpihelper.hh>

#include <dune/python/common/typeregistry.hh>
#include <dune/python/pybind11/pybind11.h>
#include <dune/python/pybind11/operators.h>

#if HAVE_MPI
#include <mpi4py/mpi4py.h>

namespace Dune::Python::Impl
{
  //! \brief helper class to represent internally the MPI_Comm
  struct MPI_Comm_Wrapper {
    MPI_Comm_Wrapper() = default;
    MPI_Comm_Wrapper(MPI_Comm value) : value(value) {}
    operator MPI_Comm () const { return value; }
    MPI_Comm value;
  };
}

// add type caster
namespace PYBIND11_NAMESPACE {
  namespace detail {
    //! \brief tell pybind11 how to case mpi4py.Comm to MPI_Comm (or actually to Impl::MPI_Comm_Wrapper)
    template <> struct type_caster<Dune::Python::Impl::MPI_Comm_Wrapper> {
    public:
      PYBIND11_TYPE_CASTER(Dune::Python::Impl::MPI_Comm_Wrapper, const_name("Dune::Python::Impl::MPI_Comm_Wrapper"));

      // Python -> C++
      bool load(handle src, bool) {
        PyObject *py_src = src.ptr();
        // Check that we have been passed an mpi4py communicator
        if (PyObject_TypeCheck(py_src, &PyMPIComm_Type)) {
          // Convert to regular MPI communicator
          value.value = *PyMPIComm_Get(py_src);
        } else {
          return false;
        }
        return !PyErr_Occurred();
      }

      // C++ -> Python
      static handle cast(const Dune::Python::Impl::MPI_Comm_Wrapper & comm, return_value_policy /* policy */, handle /* parent */) {
        // Create an mpi4py handle
        return PyMPIComm_New(comm.value);
      }
    };
}} // namespace pybind11::detail

#endif

namespace Dune::Python
{

    // registerCommunication
    // -------------------------------

    template< class Comm, class... objects >
    inline static void registerCommunication ( pybind11::class_< Comm, objects... > cls )
    {
      using pybind11::operator""_a;

      // copy constructor
      cls.def( pybind11::init<Comm>() );
      // construct from No_Comm (should always work)
      cls.def( pybind11::init( [](No_Comm comm)
          -> Comm*
          {
            return new Comm(comm);
          }));
#if HAVE_MPI
      // construct from wrapped MPI_Comm (works only with MPI)
      cls.def( pybind11::init( [](Impl::MPI_Comm_Wrapper comm)
          -> Comm*
          {
            static_assert(std::is_same_v<Comm, Dune::Communication<MPI_Comm>>, "trying to compile the constructor taking MPI_Comm for a Communication class without MPI support");
            return new Comm(comm);
          }));
#endif

      cls.def_property_readonly( "rank", &Comm::rank );
      cls.def_property_readonly( "size", &Comm::size );

      // these evaluate via the Communication -> MPI_Comm case
      cls.def(pybind11::self == pybind11::self);
      cls.def(pybind11::self != pybind11::self);

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

      // register No_Comm (eq. to MPI_Comm if MPI is missing)
      auto nocomm = pybind11::class_<No_Comm>(scope, "No_Comm_Type"); // to we have to use insertClass ?
      nocomm.def(pybind11::init<>());
      nocomm.def("__call__", [](No_Comm& self) -> No_Comm& {
        return self;
      });
      nocomm.def(pybind11::self == pybind11::self);
      nocomm.def(pybind11::self != pybind11::self);

      // make No_Comm a singleton
      static No_Comm* No_Comm_Singleton;
      std::once_flag init_No_Comm_Flag;
      auto init_No_Comm_Singleton = [&]()
      {
        No_Comm_Singleton = new No_Comm;
      };
      std::call_once(init_No_Comm_Flag, init_No_Comm_Singleton);

      // register MPI_Comm (or better) MPI_Comm_Wrapper _iff_ MPI is present
#if HAVE_MPI
      // import C symbols of mpi4py (see https://enccs.se/news/2021/03/mpi-hybrid-c-python-code/)
      std::cout << "initialize mpi4py" << std::endl;
      if (import_mpi4py() < 0) {
        throw std::runtime_error("Could not load mpi4py API.");
      }
      // and register the wrapper
      using MPI_Comm_Wrapper = Impl::MPI_Comm_Wrapper;
      auto mpi_comm_wrapper = pybind11::class_<MPI_Comm_Wrapper>(scope, "_MPI_Comm_Wrapper"); // to we have to use insertClass ?
#endif

      // register Dune::Communication type
      auto typeName = GenerateTypeName( "Dune::Communication", "Dune::MPIHelper::MPICommunicator" );
      auto includes = IncludeFiles{ "dune/common/parallel/communication.hh", "dune/common/parallel/mpihelper.hh" };
      auto [ cls, notRegistered ] = insertClass< Comm >( scope, "Communication", typeName, includes );
      if( notRegistered )
        registerCommunication( cls );

      // add attributes for default Communicator and No_Comm
      scope.attr( "comm" ) = pybind11::cast<Comm>( Dune::MPIHelper::getCommunication() );
      scope.attr( "No_Comm" ) = pybind11::cast( *No_Comm_Singleton );

#if HAVE_MPI
      // make sure we can convert the MPI_Comm to Dune::Communication
      pybind11::implicitly_convertible<MPI_Comm_Wrapper, Comm>();
#endif
      pybind11::implicitly_convertible<No_Comm, Comm>();
    }

} // namespace Dune::Python

#endif // #ifndef DUNE_PYTHON_COMMON_MPIHELPER_HH
