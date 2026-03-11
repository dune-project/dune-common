// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#include <dune/common/std/assume.hh>

// implement a function using the DUNE_ASSUME macro
int divide_by_32 (int x) {
  DUNE_ASSUME(x >= 0);
  return x/32;
}

int main()
{
  divide_by_32(7);
}
