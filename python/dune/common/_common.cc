// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <utility>

#include <dune/common/hybridutilities.hh>
#include <dune/python/common/dynmatrix.hh>
#include <dune/python/common/dynvector.hh>
#include <dune/python/common/fmatrix.hh>
#include <dune/python/common/fvector.hh>
#include <dune/python/common/mpihelper.hh>

#include <dune/python/pybind11/pybind11.h>
#include <dune/python/pybind11/stl.h>

PYBIND11_MODULE( _common, module )
{
  Dune::Python::addToTypeRegistry<double>(Dune::Python::GenerateTypeName("double"));
  Dune::Python::addToTypeRegistry<int>(Dune::Python::GenerateTypeName("int"));
  Dune::Python::addToTypeRegistry<std::size_t>(Dune::Python::GenerateTypeName("std::size_t"));

  Dune::Python::registerDynamicVector<double>(module);
  Dune::Python::registerDynamicMatrix<double>(module);

  int argc = 0;
  char **argv = NULL;
  Dune::MPIHelper::instance(argc,argv);
  Dune::Python::registerCollectiveCommunication(module);
}
