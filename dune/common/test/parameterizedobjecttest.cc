#include "config.h"
#include <iostream>
#include <cassert>
#include <tuple>
#include <dune/common/parameterizedobject.hh>
#include <dune/common/parametertree.hh>
#include <dune/common/shared_ptr.hh>

#define DefineImplementation(IF,T,PARAM...)     \
    struct T : public IF {                      \
        T(PARAM) {}                          \
        virtual std::string info() {            \
            return #T;                          \
        }                                       \
    }

struct InterfaceA
{
    virtual std::string info() = 0;
};

struct InterfaceB
{
    virtual std::string info() = 0;
};

DefineImplementation(InterfaceA, Ai, int);
DefineImplementation(InterfaceA, Bi, int);
DefineImplementation(InterfaceA, Ax);
DefineImplementation(InterfaceA, Bx);
DefineImplementation(InterfaceA, Ad, const Dune::ParameterTree&);
DefineImplementation(InterfaceA, Bd, Dune::ParameterTree);
DefineImplementation(InterfaceB, Ais, int, std::string);
DefineImplementation(InterfaceB, Bis, int, std::string);

#define CheckInstance(F,T,PARAM...)             \
    assert(#T == F.create(#T,##PARAM)->info())


int main()
{
    // int as parameter
    Dune::ParameterizedObjectFactory<std::unique_ptr<InterfaceA>, std::string, int> FactoryA;
    FactoryA.define<Ai>("Ai");
    FactoryA.define<Bi>("Bi");
    CheckInstance(FactoryA, Ai, 0);
    CheckInstance(FactoryA, Bi, 1);

    // default constructor
    Dune::ParameterizedObjectFactory<std::shared_ptr<InterfaceA>, std::string> FactoryAd;
    FactoryAd.define<Ax>("Ax");
    FactoryAd.define<Bx>("Bx");
    Dune::ParameterTree param;
    CheckInstance(FactoryAd, Ax);
    CheckInstance(FactoryAd, Bx);
    CheckInstance(FactoryAd, Ai);

    // explicitly request the default constructor
    Dune::ParameterizedObjectFactory<std::unique_ptr<InterfaceA>, std::string> FactoryAx;
    FactoryAx.define<Ax>("Ax");
    FactoryAx.define<Bx>("Bx");
    CheckInstance(FactoryAx, Ax);
    CheckInstance(FactoryAx, Bx);

    // multiple parameters
    Dune::ParameterizedObjectFactory<std::unique_ptr<InterfaceB>, std::string, int, std::string> FactoryB;
    FactoryB.define<Ais>("Ais");
    FactoryB.define<Bis>("Bis");
    CheckInstance(FactoryB, Ais, 0, std::to_string(2));
    CheckInstance(FactoryB, Bis, 1, "Hallo");
}
