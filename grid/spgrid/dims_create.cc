// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DIMS_CREATE_CC
#define DIMS_CREATE_CC

#include <limits>

namespace Dune {

  array<3> Dims_create(int P, array<3> & w) {
    int DIM = 3;
    array<3> solution;
    double weight = std::numeric_limits<double>::max();
    array<3> dims;
    for (dims[0]=1; dims[0]<=P; dims[0]++)
      for (dims[1]=1; dims[1]<=P; dims[1]++)
        for (dims[2]=1; dims[2]<=P; dims[2]++)
          if (dims[0]*dims[1]*dims[2]==P) {
            double thisweight = 0;
            for (int d=0; d<DIM; d++) {
              thisweight += (dims[d]*1.0)/(w[d]*1.0);
            }
#if 0
            std::cout << "possible: " << dims
                      << " weight=" << thisweight << std::endl;
#endif
            if (weight > thisweight) {
              solution = dims;
              weight = thisweight;
            }
          }
    return solution;
  }

}

#endif // DIMS_CREATE_CC
