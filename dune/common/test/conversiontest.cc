// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <dune/common/typetraits.hh>

int main ()
{
  std :: cout << "Conversion from int to double exists? "
              << Dune::Conversion< int, double > :: exists << std :: endl;

  return 0;
}
