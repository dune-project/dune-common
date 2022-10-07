// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>
#include <dune/common/fvector.hh>

int main(int argc, char * argv[])
{

  Dune::FieldVector<double,DIM> one(1);
  Dune::FieldVector<float,2> two(2);

  one=two;

  return 0;
}
