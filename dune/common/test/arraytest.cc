// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <array>
#include <iostream>

#include <dune/common/std/make_array.hh>
#include <dune/common/classname.hh>
#include <dune/common/deprecated.hh>
#include <dune/common/fvector.hh>
#include <dune/common/streamoperators.hh>

template<class T, std::size_t n>
void f(const std::array<T, n> &a)
{
  using Dune::operator<<;
  std::cout << "Got a " << Dune::className(a) << " with elements " << a << std::endl;
}

int main() {
  DUNE_NO_DEPRECATED_BEGIN

  // check that make_array works
  f(Dune::Std::make_array(1, 2));
  f(Dune::Std::make_array(1, 2, 3));
  f(Dune::Std::make_array(1, 2, 3, 4));
  f(Dune::Std::make_array(1, 2, 3, 4, 5));
  f(Dune::Std::make_array(1, 2, 3, 4, 5, 6));
  f(Dune::Std::make_array(1, 2, 3, 4, 5, 6, 7));
  f(Dune::Std::make_array(1, 2, 3, 4, 5, 6, 7, 8));
  f(Dune::Std::make_array(1, 2, 3, 4, 5, 6, 7, 8, 9));
  f(Dune::Std::make_array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10));

  Dune::FieldVector<double, 2> x(0);
  f(Dune::Std::make_array(x, x));

  DUNE_NO_DEPRECATED_END
}
