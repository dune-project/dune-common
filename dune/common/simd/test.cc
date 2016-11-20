#include <config.h>

#include <ostream>

#include <dune/common/simd/test.hh>

void Dune::Simd::UnitTest::complain(const char *file, int line,
                                    const char *func, const char *expr)
{
  log_ << file << ":" << line << ": In " << func  << ": Error: check (" << expr
       << ") failed" << std::endl;
  good_ = false;
}
