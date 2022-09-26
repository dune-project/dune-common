// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include "config.h"
#include <iostream>
#include <cassert>
#include <memory>
#include <tuple>
#include <dune/common/parametertree.hh>
#include <dune/common/shared_ptr.hh>

#include "parameterizedobjectfactorysingleton.hh"

DefineImplementation(InterfaceA, Ai, int);
DefineImplementation(InterfaceA, Bi, int);
DefineImplementation2(InterfaceA, Ax);
DefineImplementation2(InterfaceA, Bx);
DefineImplementation(InterfaceA, Ad, const Dune::ParameterTree&);
DefineImplementation(InterfaceA, Bd, Dune::ParameterTree);
DefineImplementation(InterfaceB, Ais, int, std::string);
DefineImplementation(InterfaceB, Bis, int, std::string);

#define CheckInstance2(F,T)                       \
    assert(#T == F.create(#T)->info())

#define CheckInstance(F,T,...)                    \
    assert(#T == F.create(#T,##__VA_ARGS__)->info())

struct AImp : public InterfaceA
{
    AImp(std::string s) :
        s_(s)
    {}

    AImp(const AImp& /*other*/) :
        s_("copied")
    {}

    std::string info() override
    {
        return s_;
    }
    std::string s_;
};

int main()
{
    // int as parameter
    // Dune::ParameterizedObjectFactory<std::unique_ptr<InterfaceA>(int)> FactoryA;
    globalPtrFactory<InterfaceA>().define<Ai>("Ai");
    globalPtrFactory<InterfaceA>().define<Bi>("Bi");
    globalPtrFactory<InterfaceA>().define("Ax", [](int /*i*/) { return std::make_unique<Ax>(); });
    CheckInstance(globalPtrFactory<InterfaceA>(), Ai, 0);
    CheckInstance(globalPtrFactory<InterfaceA>(), Bi, 1);
    CheckInstance(globalPtrFactory<InterfaceA>(), Ax, 1);
    // int as parameter for external factory
    CheckInstance(globalPtrFactory<InterfaceA>(), Aix, 0);
    CheckInstance(globalPtrFactory<InterfaceA>(), Bix, 1);

    // default constructor
    Dune::ParameterizedObjectFactory<std::shared_ptr<InterfaceA>()> FactoryAd;
    FactoryAd.define<Ax>("Ax");
    FactoryAd.define<Bx>("Bx");
    FactoryAd.define("Ai", []() { return std::make_shared<Ai>(0); });
    AImp aimp("onStack");
    FactoryAd.define("AImp", [&]() { return Dune::stackobject_to_shared_ptr<AImp>(aimp); });
    FactoryAd.define("AImp2", Dune::stackobject_to_shared_ptr<AImp>(aimp));
    FactoryAd.define("AImp3", std::make_shared<AImp>("shared"));
    Dune::ParameterTree param;
    CheckInstance2(FactoryAd, Ax);
    CheckInstance2(FactoryAd, Bx);
    CheckInstance2(FactoryAd, Ai);
    std::cout << FactoryAd.create("AImp")->info() << std::endl;
    std::cout << FactoryAd.create("AImp2")->info() << std::endl;
    std::cout << FactoryAd.create("AImp3")->info() << std::endl;

    // explicitly request the default constructor
    Dune::ParameterizedObjectFactory<std::unique_ptr<InterfaceA>()> FactoryAx;
    FactoryAx.define<Ax>("Ax");
    FactoryAx.define<Bx>("Bx");
    CheckInstance2(FactoryAx, Ax);
    CheckInstance2(FactoryAx, Bx);

    // multiple parameters
    Dune::ParameterizedObjectFactory<std::unique_ptr<InterfaceB>(int, std::string)> FactoryB;
    FactoryB.define<Ais>("Ais");
    FactoryB.define<Bis>("Bis");
    CheckInstance(FactoryB, Ais, 0, std::to_string(2));
    CheckInstance(FactoryB, Bis, 1, "Hallo");

    // check for ambiguous overloads
    Dune::ParameterizedObjectFactory<bool()> FactoryBool;
    FactoryBool.define("true",true);
    FactoryBool.define("false",[](){return false;});

    // value semantics
    Dune::ParameterizedObjectFactory<std::function<double(double)>(int)> FactoryC;
    FactoryC.define("fi", [](int i) {
        return [=](double x) { return x+i;};
    });
    FactoryC.define("fi1", [](int i) {
        return [=](double x) { return x+i+1;};
    });
    assert(FactoryC.create("fi", 42)(0) == 42);
    assert(FactoryC.create("fi1", 42)(0) == 43);

}
