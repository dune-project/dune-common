// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <utility>

#include <dune-common-config.hh>

#include <dune/python/common/dynmatrix.hh>
#include <dune/python/common/dynvector.hh>
#include <dune/python/common/fmatrix.hh>
#include <dune/python/common/fvector.hh>
#include <dune/python/common/mpihelper.hh>

#include <dune/python/pybind11/pybind11.h>
#include <dune/python/pybind11/stl.h>

#ifdef DUNE_ENABLE_PYTHONMODULE_PRECOMPILE
#include "registerfvector.hh"
#endif

PYBIND11_MODULE( _common, module )
{
  Dune::Python::addToTypeRegistry<double>(Dune::Python::GenerateTypeName("double"));
  Dune::Python::addToTypeRegistry<float>(Dune::Python::GenerateTypeName("float"));
  Dune::Python::addToTypeRegistry<int>(Dune::Python::GenerateTypeName("int"));
  Dune::Python::addToTypeRegistry<std::size_t>(Dune::Python::GenerateTypeName("std::size_t"));

  Dune::Python::registerDynamicVector<double>(module);
  Dune::Python::registerDynamicMatrix<double>(module);

  Dune::MPIHelper::instance();
  Dune::Python::registerCommunication(module);

#ifdef DUNE_ENABLE_PYTHONMODULE_PRECOMPILE
  registerFieldVectorToModule<0>(module);
  registerFieldVectorToModule<1>(module);
  registerFieldVectorToModule<2>(module);
  registerFieldVectorToModule<3>(module);
  registerFieldVectorToModule<4>(module);
#endif
}
