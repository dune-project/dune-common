// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

#include <cstdint>
#include <new>
#include <tuple>
#include <utility>

#include <dune/common/debugalign.hh>
#include <dune/common/hybridutilities.hh>
#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/test/arithmetictestsuite.hh>
#include <dune/common/test/testsuite.hh>

class WithViolatedAlignmentHandler {
  Dune::ViolatedAlignmentHandler oldhandler;
public:
  template<class H>
  WithViolatedAlignmentHandler(H &&newhandler) :
    oldhandler(Dune::violatedAlignmentHandler())
  {
    Dune::violatedAlignmentHandler() = std::forward<H>(newhandler);
  }

  WithViolatedAlignmentHandler(const WithViolatedAlignmentHandler &) = delete;
  WithViolatedAlignmentHandler(WithViolatedAlignmentHandler &&) = delete;

  WithViolatedAlignmentHandler&
  operator=(const WithViolatedAlignmentHandler &) = delete;
  WithViolatedAlignmentHandler&
  operator=(WithViolatedAlignmentHandler &&) = delete;

  ~WithViolatedAlignmentHandler()
  {
    Dune::violatedAlignmentHandler() = oldhandler;
  }
};

// intentionally violate alignment and check that that is detected
template<class T>
void checkAlignmentViolation(Dune::TestSuite &test)
{
  bool misalignmentDetected = false;
  WithViolatedAlignmentHandler
    guard([&](auto&&...){ misalignmentDetected = true; });

  char buffer[alignof(T)+sizeof(T)];

  void* misalignedAddr;
  {
    // a more portable way to ddo this would be to use std::align(), but that
    // isn't supported by g++-4.9 yet
    auto addr = std::uintptr_t( (void*)buffer );
    addr += alignof(T) - 1;
    addr &= -std::uintptr_t(alignof(T));
    addr += 1;
    misalignedAddr = (void*)addr;
  }

  auto ptr = new(misalignedAddr) T;
  test.check(misalignmentDetected, "default construct")
    << "misalignment not detected for " << Dune::className<T>();

  misalignmentDetected = false;

  ptr->~T();
  test.check(misalignmentDetected, "destruct")
    << "misalignment not detected for " << Dune::className<T>();

  misalignmentDetected = false;

  ptr = new(misalignedAddr) T(T(0));
  test.check(misalignmentDetected, "move construct")
    << "misalignment not detected for " << Dune::className<T>();
  ptr->~T(); // ignore any misalignment here

  misalignmentDetected = false;

  T t(0);
  ptr = new(misalignedAddr) T(t);
  test.check(misalignmentDetected, "copy construct")
    << "misalignment not detected for " << Dune::className<T>();
  ptr->~T(); // ignore any misalignment here
}

int main(int argc, char **argv)
{
  Dune::MPIHelper::instance(argc, argv);

  Dune::ArithmeticTestSuite test;

  using ArithmeticTypes = std::tuple<
    bool,
    char, signed char, unsigned char,
    short, unsigned short,
    int, unsigned,
    long, long unsigned,
    long long, long long unsigned,
    wchar_t, char16_t, char32_t,
    float, double, long double>;

  Dune::Hybrid::forEach(ArithmeticTypes(), [&](auto val) {
      using T = decltype(val);
      using Aligned = Dune::AlignedNumber<T>;
      test.checkArithmetic<Aligned, T>();

      checkAlignmentViolation<Aligned>(test);
    });

  return test.exit();
}
