// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#include <dune/common/forceinline.hh>
#include <dune/common/test/testsuite.hh>

template <unsigned int n>
DUNE_FORCE_INLINE int compute_sum ()
{
  if constexpr (n <= 1)
    return n;
  else
    return n + compute_sum<n-1>();
}

int main ()
{
  Dune::TestSuite test;

  test.check(compute_sum<100>() == (100 * 101)/2);

  return test.exit();
}