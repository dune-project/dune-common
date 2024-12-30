// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <dune/common/std/compare.hh>

int main(int argc, char** argv)
{
  using namespace Dune;

  static_assert(Std::three_way_comparable<double>);
  static_assert(Std::three_way_comparable_with<double,double>);
  static_assert(Std::three_way_comparable_with<double,float>);
  static_assert(Std::three_way_comparable_with<double,int>);

  static_assert(not Std::three_way_comparable_with<unsigned int,int>);
}
