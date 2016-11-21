#if HAVE_CONFIG_H
#include "config.h"
#endif

#if !HAVE_VC
#error Incosistent buildsystem.  This program should not be built in the \
  absence of Vc.
#endif

#include <cstdlib>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/vc.hh>

int main()
{
  Dune::Simd::UnitTest test;

  test.checkSimdType<Vc::Vector<double> >();

  return test.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}
