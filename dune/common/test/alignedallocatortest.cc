// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

#include <cstdint>
#include <tuple>
#include <vector>

#include <dune/common/alignedallocator.hh>
#include <dune/common/debugalign.hh>
#include <dune/common/hybridutilities.hh>
#include <dune/common/test/testsuite.hh>


template<class T>
void checkAlignment(Dune::TestSuite &test, unsigned int n)
{
  try {
    std::vector<T, Dune::AlignedAllocator<T>> defaultalignment(4);
    test.check(Dune::isAligned(defaultalignment.data(), std::alignment_of<T>::value), "defaultalignment isAligned")
      << "alignment(" << std::alignment_of<T>::value << ") not detected for " << Dune::className<T>() << ", count " << n;
  }
  catch (std::bad_alloc & e)
  {
    test.check(false, "defaultalignment isAligned")
      << "failed to allocate vector with defaultalignment " << std::alignment_of<T>::value << " for " << Dune::className<T>() << ", count " << n;
  }

  try {
    std::vector<T, Dune::AlignedAllocator<T,16>> alignment16(4);
    test.check(Dune::isAligned(alignment16.data(), 16), "alignment16 isAligned")
      << "alignment(16) not detected for " << Dune::className<T>();
  }
  catch (std::bad_alloc & e)
  {
    test.check(false, "alignment16 isAligned")
      << "failed to allocate vector with alignment 16 for " << Dune::className<T>() << ", count " << n;
  }

}

int main(int argc, char **argv)
{
  Dune::TestSuite test;

  using ArithmeticTypes = std::tuple<
    char, signed char, unsigned char,
    short, unsigned short,
    int, unsigned,
    long, long unsigned,
    long long, long long unsigned,
    wchar_t, char16_t, char32_t,
    float, double, long double>;

  Dune::Hybrid::forEach(ArithmeticTypes(), [&](auto val) {
    using T = decltype(val);
    for (unsigned int n = 1; n < 10; n++)
      checkAlignment<T>(test, n);
  });

  return test.exit();
}
