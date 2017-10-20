// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>


#include <dune/python/common/typeregistry.hh>


PYBIND11_MODULE( _typeregistry, module )
{
  Dune::Python::registerTypeRegistry(module);
}
