// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <assert.h>
#include <iostream>
#include <dune/common/lru.hh>
#include <dune/common/parallel/mpihelper.hh>

void lru_test()
{
  std::cout << "testing Dune::lru<int,double>\n";

  Dune::lru<int, double> lru;
  lru.insert(10, 1.0);
  assert(lru.front() == lru.back());
  lru.insert(11, 2.0);
  assert(lru.front() == 2.0 && lru.back() == 1.0);
  lru.insert(12, 99);
  lru.insert(13, 1.3);
  lru.insert(14, 12345);
  lru.insert(15, -17);
  assert(lru.front() == -17 && lru.back() == 1.0);
  // update
  lru.insert(10);
  assert(lru.front() == 1.0 && lru.back() == 2.0);
  // update
  lru.touch(13);
  assert(lru.front() == 1.3 && lru.back() == 2.0);
  // remove item
  lru.pop_front();
  assert(lru.front() == 1.0 && lru.back() == 2.0);
  // remove item
  lru.pop_back();
  assert(lru.front() == 1.0 && lru.back() == 99);

  std::cout << "... passed\n";
}

int main (int argc, char** argv)
{
  Dune::MPIHelper::instance(argc,argv);

  lru_test();

  return 0;
}
