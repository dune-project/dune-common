// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <complex>
#include <iostream>

#include <dune/common/classname.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>


using Dune::FieldVector;
using std::complex;

int main()
{
  try {
    std::cout << "First three simple class names extracted from variables:" << std::endl;
    FieldVector<int, 3> xi;
    std::cout << className(xi) << std::endl;
    FieldVector<double, 1> xd;
    std::cout << className(xd) << std::endl;
    FieldVector<complex<double>, 10> xcd;
    std::cout << className(xcd) << std::endl;
    std::cout << std::endl;

    std::cout << "Adding const:" << std::endl;
    const FieldVector<int, 3> cxi;
    std::cout << className(cxi) << std::endl;
    std::cout << std::endl;

    std::cout << "If a variable is a reference can not be extracted (needs decltype as used below): " << std::endl;
    FieldVector<double, 1> &rxd = xd;
    std::cout << className(rxd) << std::endl;
    std::cout << std::endl;

    std::cout << "Extracting the class name using a type directly - "
              << "also extractes references correctly: " << std::endl;
    std::cout << Dune::className<decltype(rxd)>() << std::endl;
    const FieldVector<double, 1> &rcxd = xd;
    std::cout << Dune::className<decltype(rcxd)>() << std::endl;
    const FieldVector<int, 3> &rcxi = cxi;
    std::cout << Dune::className<decltype(rcxi)>() << std::endl;
    std::cout << std::endl;

    std::cout << "Test some further types:" << std::endl;
    std::cout << Dune::className< volatile FieldVector<complex<double>, 10>& >() << std::endl;
    std::cout << Dune::className< FieldVector<complex<double>, 10>&& >() << std::endl;
    std::cout << std::endl;

  } catch (Dune::Exception& e) {
    throw;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }
}
