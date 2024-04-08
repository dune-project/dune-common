// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#pragma once

#include <utility>

#include <dune/python/common/fmatrix.hh>
#include <dune/python/common/fvector.hh>

#include <dune/python/pybind11/pybind11.h>
#include <dune/python/pybind11/stl.h>

template <int s>
void registerFieldVectorToModule( pybind11::module module )
#ifdef INCLUDE_PRECOMP_INLINE
{
  static const int a = s*3;
  Dune::Python::registerFieldVector<double, a>(module);
  Dune::Python::registerFieldVector<double, a+1>(module);
  Dune::Python::registerFieldVector<double, a+2>(module);
}
#else
;
#endif
