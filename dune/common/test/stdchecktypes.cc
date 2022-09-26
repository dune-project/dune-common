// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tuple>
#include <utility>
#include <sstream>

#include <dune/common/std/type_traits.hh>


struct A
{
  typedef long type;
};

struct B
{
};


template <class Traits>
struct C
{
  template <class T>
  using Fallback = int;

  template <class T>
  using Target = typename T::type;

  using type = Dune::detected_or_fallback_t<Fallback, Target, Traits>;

  void foo() { };
};


int main()
{
  C< A > ca;

  ca.foo();


  // this should produce a compiler warning
  C< B > cb;

  cb.foo();

}
