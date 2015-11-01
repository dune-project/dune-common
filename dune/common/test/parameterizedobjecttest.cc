#include "config.h"
#include <iostream>
#include <cassert>
#include <tuple>
#include <dune/common/parameterizedobject.hh>

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
    assert(#T == F::create(#T,##PARAM)->info())


int main()
{
    // int as parameter
    using FactoryA = Dune::ParameterizedObjectFactory<InterfaceA(int)>;
    FactoryA::define<Ai>("Ai");
    FactoryA::define<Bi>("Bi");
    CheckInstance(FactoryA, Ai, 0);
    CheckInstance(FactoryA, Bi, 1);

    // default constructor
    using FactoryAd = Dune::ParameterizedObjectFactory<InterfaceA>;
    FactoryAd::define<Ax>("Ax");
    FactoryAd::define<Bx>("Bx");
    Dune::ParameterTree param;
    CheckInstance(FactoryAd, Ax);
    CheckInstance(FactoryAd, Bx);

    // explicitly request the default constructor
    using FactoryAx = Dune::ParameterizedObjectFactory<InterfaceA()>;
    FactoryAx::define<Ax>("Ax");
    FactoryAx::define<Bx>("Bx");
    CheckInstance(FactoryAx, Ax);
    CheckInstance(FactoryAx, Bx);

    // multiple parameters
    using FactoryB = Dune::ParameterizedObjectFactory<InterfaceB(int, std::string)>;
    FactoryB::define<Ais>("Ais");
    FactoryB::define<Bis>("Bis");
    CheckInstance(FactoryB, Ais, 0, std::to_string(2));
    CheckInstance(FactoryB, Bis, 1, "Hallo");
}
