#include "config.h"
#include <iostream>
#include <cassert>
#include <tuple>
#include <dune/common/parameterizedobject.hh>

#define DefineImplementation(IF,T,PARAM...)     \
    struct T : public IF {                      \
        T(PARAM) {}                             \
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
DefineImplementation(InterfaceA, Aid, int, double);
DefineImplementation(InterfaceA, Bid, int, double);

using FactoryA = Dune::ParameterizedObjectFactory<InterfaceA, int>;
// using FactoryB = Dune::ParameterizedObjectFactory<InterfaceB(int)>;

#define CheckInstance(F,T,PARAM...)             \
    assert(#T == F::create(#T,PARAM)->info())


int main()
{
    FactoryA::define<Ai>("Ai");
    FactoryA::define<Bi>("Bi");

    CheckInstance(FactoryA, Ai, 0);
    CheckInstance(FactoryA, Bi, 1);
    // CheckInstance(FactoryB, Ai, 0, 2);
    // CheckInstance(FactoryB, Bi, 1, "Hallo");
}
