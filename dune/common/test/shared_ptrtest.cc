// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

// make sure assert works even when not compiling for debugging
#ifdef NDEBUG
#undef NDEBUG
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/deprecated.hh>
#include <dune/common/shared_ptr.hh>

#include <cstdlib>


class A {};
class B : public A {};
class C : A {};


int main(){
  using namespace Dune;
  int ret=0;
  {
    // test shared_ptr for stack allocation
    {
      int i = 10;
      std::shared_ptr<int> pi = stackobject_to_shared_ptr(i);
    }

    // test shared_ptr for stack allocation with down cast
    {
DUNE_NO_DEPRECATED_BEGIN
      B b2;
      std::shared_ptr<A> pa = stackobject_to_shared_ptr<A>(b2);
DUNE_NO_DEPRECATED_END
#ifdef SHARED_PTR_COMPILE_FAIL
      C c;
      pa = stackobject_to_shared_ptr<A>(c);       // A is an inaccessible base of C
#endif
    }
  }
  return (ret);
}
