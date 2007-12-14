// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "config.h"
#include <iostream>
#include <dune/common/fvector.hh>
#include <dune/common/fassign.hh>

int main ()
{
  try
  {
    static const int sz = _SIZE;
    Dune::FieldVector<int,sz> v;

    v <<= _VALUES;

    for (int i=0; i<sz; i++)
      std::cout << "value[" << i << "] = " << v[i] << "\n";

    return 0;
  }
  catch (Dune::MathError &e)
  {
    std::cerr << "Dune reported MathError: " << e << std::endl;
    return 1;
  }
  catch (Dune::Exception &e)
  {
    std::cerr << "Dune reported error: " << e << std::endl;
    return 1;
  }
  catch (...)
  {
    std::cerr << "Unknown exception thrown!" << std::endl;
    return 1;
  }
}
