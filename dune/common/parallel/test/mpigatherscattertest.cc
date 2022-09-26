// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#include <config.h>

#include <iostream>
#include <array>
#include <vector>


#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpicommunication.hh>
#include <dune/common/exceptions.hh>

using namespace Dune;

int main(int argc, char** argv){
  MPIHelper & mpihelper = MPIHelper::instance(argc, argv);
  auto cc = mpihelper.getCommunication();
  int rank = cc.rank();
  int size = cc.size();

  std::array<double, 2> data = {1.0 + rank, 2.0 + rank};

  auto gathered = cc.igather(data, std::vector<double>(rank==0?2*size:0), 0).get();

  for(auto& d : gathered)
    d += 1;

  cc.iscatter(gathered, data, 0).get();
  if(data[0] != 2+rank ||
     data[1] != 3+rank){
    DUNE_THROW(Exception, "Wrong result after gather - scatter");
  }

  return 0;
}
