#ifndef DUNE_COMMON_TEST_PARAMETERIZEDOBJECTFACTORYSINGLETON_HH
#define DUNE_COMMON_TEST_PARAMETERIZEDOBJECTFACTORYSINGLETON_HH

#include <dune/common/parameterizedobject.hh>
#include <dune/common/singleton.hh>
#include <string>

#define DefineImplementation(IF,T,PARAM...)     \
    struct T : public IF {                      \
        T(PARAM) {}                          \
        std::string info() override {            \
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

template<typename Interface>
Dune::ParameterizedObjectFactory<std::unique_ptr<Interface>(int)> &
globalPtrFactory()
{
    return Dune::Singleton<Dune::ParameterizedObjectFactory<std::unique_ptr<Interface>(int)>>::instance();
}

#endif //#ifndef DUNE_COMMON_TEST_PARAMETERIZEDOBJECTFACTORYSINGLETON_HH
