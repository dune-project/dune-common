// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/test/iteratorfacadetest.hh>
#include <dune/common/test/iteratortest.hh>

int main(){
  // Test the TestIterator;
  typedef TestContainer<double, Dune::BidirectionalIteratorFacade> Container;
  Container bidicontainer;

  Container::const_iterator cit = bidicontainer.begin();
  //This should fail since making a mutable iterator from a const iterator
  //discard qualifiers
  [[maybe_unused]] Container::iterator it;
  it = cit;
}
