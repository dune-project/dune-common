// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

#ifdef NDEBUG
    #undef NDEBUG
#endif
#ifdef TEST_NDEBUG
    #define NDEBUG TEST_NDEBUG
#endif

#include <cassert>
#include <dune/common/assertandreturn.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/parallel/mpihelper.hh>

struct Foo
{
  static constexpr auto lessAndReturn([[maybe_unused]] int a, [[maybe_unused]] int b, int x)
  {
    return DUNE_ASSERT_AND_RETURN(a<b, x);
  }
};


int main ( int argc, char **argv )
try
{
  using namespace Dune;

  MPIHelper::instance(argc, argv);

  // This should not fail since 0<2
  if (Foo::lessAndReturn(0,2,3) != 3)
    DUNE_THROW(Dune::Exception, "DUNE_ASSERT_AND_RETURN returned incorrect value in dynamic context");

  // This should not fail since 0<2
  if (std::integral_constant<int, Foo::lessAndReturn(0,2,3)>::value != 3)
    DUNE_THROW(Dune::Exception, "DUNE_ASSERT_AND_RETURN returned incorrect value in constexpr context");

// If EXPECT_FAIL would work with failing assertions,
// we could test if the assertion is triggered with
// a target
//
// dune_add_test(NAME assertandreturntest_runtime_fail
//               SOURCES assertandreturntest.cc
//               LINK_LIBRARIES dunecommon
//               COMPILE_DEFINITIONS "TEST_RUNTIME_FAIL"
//               EXPECT_FAIL
//               LABELS quick)
//
// and the following code:
#ifdef TEST_RUNTIME_FAIL
  // This should fail at runtime because 0>-3
  if (Foo::lessAndReturn(0,-1,3) != 3)
    DUNE_THROW(Dune::Exception, "DUNE_ASSERT_AND_RETURN returned incorrect value in dynamic context");
#endif

#ifdef TEST_COMPILETIME_FAIL
  // This should fail at compile time because 0>-3
  if (std::integral_constant<int, Foo::lessAndReturn(0,-1,3)>::value != 3)
    DUNE_THROW(Dune::Exception, "DUNE_ASSERT_AND_RETURN returned incorrect value in constexpr context");
#endif

#ifdef TEST_NDEBUG
  // This should not fail because NDEBUG is set
  if (Foo::lessAndReturn(0,-1,3) != 3)
    DUNE_THROW(Dune::Exception, "DUNE_ASSERT_AND_RETURN returned incorrect value in dynamic context");

  // This should not fail because NDEBUG is set
  if (std::integral_constant<int, Foo::lessAndReturn(0,-1,3)>::value != 3)
    DUNE_THROW(Dune::Exception, "DUNE_ASSERT_AND_RETURN returned incorrect value in constexpr context");
#endif

  return 0;
}
catch( Dune::Exception &e )
{
  std::cerr << "Dune reported error: " << e << std::endl;
  return 1;
}
catch(...)
{
  std::cerr << "Unknown exception thrown!" << std::endl;
  return 1;
}
