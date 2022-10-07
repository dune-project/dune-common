// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/enumset.hh>
#include <iostream>
int main()
{
  using namespace Dune;
  std::cout<<Combine<EnumItem<int,1>,EnumItem<int,2>,int>::contains(1)<<
  " "<<Combine<EnumItem<int,1>,EnumItem<int,2>,int>::contains(2)<<
  " "<<Combine<Combine<EnumItem<int,1>,EnumItem<int,2>,int>,EnumItem<int,0>,int>::contains(3)<<
  " "<<EnumRange<int,1,3>::contains(3)<<std::endl;
}
