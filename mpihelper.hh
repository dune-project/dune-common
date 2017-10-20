// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_PYTHON_COMMON_MPIHELPER_HH
#define DUNE_PYTHON_COMMON_MPIHELPER_HH

#include <config.h>

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/collectivecommunication.hh>

#include <dune/python/pybind11/pybind11.h>

namespace Dune
{

  namespace CorePy
  {

    namespace py = pybind11;

    void registerCollectiveCommunication(py::handle scope)
    {
      typedef Dune::CollectiveCommunication<Dune::MPIHelper::MPICommunicator> Comm;

      py::class_<Comm> cls(scope, "CollectiveCommunication");

      cls.def_property_readonly("rank", &Comm::rank);
      cls.def_property_readonly("size", &Comm::size);

      cls.def("barrier", &Comm::barrier);

      cls.def("min", [] (const Comm& comm, double x) { return comm.min(x); });

      scope.attr("comm") = py::cast(Dune::MPIHelper::getCollectiveCommunication());
    }

  } // namespace CorePy

} // namespace Dune

#endif // #ifndef DUNE_PYTHON_COMMON_MPIHELPER_HH
