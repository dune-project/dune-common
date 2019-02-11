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

static const int DUNE_UNUSED init = init_Factory();
