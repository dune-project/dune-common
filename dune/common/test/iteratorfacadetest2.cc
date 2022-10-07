// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "dummyiterator.hh"

int main(){
  // Check that iterator<T> can be compared with iterator<const T> as soon as
  // a conversion from iterator<T> to iterator<const T> exists

  int value = 0;
  dummyiterator<int> mit(value);
  dummyiterator<const int> cit(value);

  [[maybe_unused]] int value2 = *mit;  // Check whether dereferencing works

  bool result = mit == cit;

  if(result) return 0;
  else return 1;
}
