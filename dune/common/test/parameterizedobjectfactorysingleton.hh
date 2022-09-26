// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_TEST_PARAMETERIZEDOBJECTFACTORYSINGLETON_HH
#define DUNE_COMMON_TEST_PARAMETERIZEDOBJECTFACTORYSINGLETON_HH

#include <dune/common/parameterizedobject.hh>
#include <dune/common/singleton.hh>
#include <string>

#define DefineImplementation2(IF,T)             \
    struct T : public IF {                      \
        T() {}                                  \
        std::string info() override {           \
            return #T;                          \
        }                                       \
    }

#define DefineImplementation(IF,T,...)          \
    struct T : public IF {                      \
        T(__VA_ARGS__) {}                       \
        std::string info() override {           \
            return #T;                          \
        }                                       \
    }

struct InterfaceA
{
    virtual std::string info() = 0;
    virtual ~InterfaceA() = default;
};

struct InterfaceB
{
    virtual std::string info() = 0;
    virtual ~InterfaceB() = default;
};

template<typename Interface>
Dune::ParameterizedObjectFactory<std::unique_ptr<Interface>(int)> &
globalPtrFactory()
{
    return Dune::Singleton<Dune::ParameterizedObjectFactory<std::unique_ptr<Interface>(int)>>::instance();
}

#endif //#ifndef DUNE_COMMON_TEST_PARAMETERIZEDOBJECTFACTORYSINGLETON_HH
