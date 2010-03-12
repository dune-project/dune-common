// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <iostream>
#include <dune/common/fvector.hh>
#include <dune/common/fassign.hh>
#include <dune/common/geometrytype.hh>

using namespace Dune;

int main(int argc, char** argv) try
{
  Dune::GeometryType t(Dune::GeometryType::cube, 4);
  std::cout << t << "\n";

  Dune::FieldVector<double,3> pos;
  pos <<= 1, 0, 0;

  return 0;
}
catch (Exception e) {
  std::cout << e << std::endl;
}
