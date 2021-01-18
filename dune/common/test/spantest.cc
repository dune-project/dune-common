// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <array>
#include <vector>

#include <dune/common/std/span.hh>

int main(int argc, char** argv){
  {
    std::array<double,2> arr = {3.14159, 42};
    Dune::Std::span<double> sp(arr);
    sp[0] = 0.0;
    if(arr[0] != 0.0)
      return 1;
  }

  {
    std::vector<double> vec = {3.14159, 42};
    Dune::Std::span<double> sp(vec);
    sp[1] = 0.0;
    if(vec[1] != 0.0)
      return 1;
  }

  return 0;
}
