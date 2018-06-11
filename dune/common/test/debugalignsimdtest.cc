#include <config.h>

#include <cstdlib>

#include <dune/common/debugalign.hh>
#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/simd/test.hh>

int main(int argc, char **argv)
{
  Dune::MPIHelper::instance(argc, argv);

  Dune::Simd::UnitTest test;

  test.checkVector<Dune::AlignedNumber<double> >();

  return test.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}
