// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>
#include <dune/common/dynvector.hh>

int main(int, char **)
{

  Dune::DynamicVector<double> one(1);
  Dune::DynamicVector<double> two(2);

  two = one;

  return 0;
}
