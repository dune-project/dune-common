#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdlib>

#include <dune/common/simd/loop.hh>
#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/looptest.hh>

int main()
{
  Dune::Simd::UnitTest test;


  test.checkVector<Dune::LoopSIMD<char              ,5>>();
  test.checkVector<Dune::LoopSIMD<unsigned char     ,5>>();
  test.checkVector<Dune::LoopSIMD<signed char       ,5>>();

  test.checkVector<Dune::LoopSIMD<short             ,5>>();
  test.checkVector<Dune::LoopSIMD<unsigned short    ,5>>();
  test.checkVector<Dune::LoopSIMD<int               ,5>>();
  test.checkVector<Dune::LoopSIMD<unsigned          ,5>>();
  test.checkVector<Dune::LoopSIMD<long              ,5>>();
  test.checkVector<Dune::LoopSIMD<unsigned long     ,5>>();
  test.checkVector<Dune::LoopSIMD<long long         ,5>>();
  test.checkVector<Dune::LoopSIMD<unsigned long long,5>>();

  test.checkVector<Dune::LoopSIMD<bool              ,5>>();

  test.checkVector<Dune::LoopSIMD<float             ,5>>();
  test.checkVector<Dune::LoopSIMD<double            ,5>>();
  test.checkVector<Dune::LoopSIMD<long double       ,5>>();

  test.checkMask<Dune::LoopSIMD<bool                ,5>>();

  return test.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}
