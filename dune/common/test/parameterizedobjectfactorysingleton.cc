// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include "config.h"
#include <iostream>
#include <cassert>
#include <memory>
#include <tuple>
#include <dune/common/parameterizedobject.hh>
#include <dune/common/parametertree.hh>
#include <dune/common/shared_ptr.hh>

#include "parameterizedobjectfactorysingleton.hh"

DefineImplementation(InterfaceA, Aix, int);
DefineImplementation(InterfaceA, Bix, int);

int init_Factory()
{
    globalPtrFactory<InterfaceA>().define<Aix>("Aix");
    globalPtrFactory<InterfaceA>().define("Bix", [](int i) { return std::make_unique<Bix>(i); });
    return 0;
}

[[maybe_unused]] static const int init = init_Factory();
