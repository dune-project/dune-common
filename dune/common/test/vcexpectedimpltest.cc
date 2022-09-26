// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include "config.h"

#if !HAVE_VC
#error Inconsistent buildsystem.  This program should not be built in the \
  absence of Vc.
#endif

#include <cstdlib>
#include <map>
#include <iostream>
#include <string>

#include <dune/common/exceptions.hh>
#include <dune/common/vc.hh>

const std::map<Vc::Implementation, std::string> impl_names = {
  {Vc::ScalarImpl, "Scalar" },
  {Vc::SSE2Impl,   "SSE2"   },
  {Vc::SSE3Impl,   "SSE3"   },
  {Vc::SSSE3Impl,  "SSSE3"  },
  {Vc::SSE41Impl,  "SSE41"  },
  {Vc::SSE42Impl,  "SSE42"  },
  {Vc::AVXImpl,    "AVX"    },
  {Vc::AVX2Impl,   "AVX2"   },
  {Vc::MICImpl,    "MIC"    },
};

const std::string expected_var = "DUNE_TEST_EXPECTED_VC_IMPLEMENTATION";

int main()
{

  auto p = impl_names.find(Vc::CurrentImplementation::current());
  if(p == impl_names.end())
    DUNE_THROW(Dune::NotImplemented, "Unexpected current implementation value "
               << Vc::CurrentImplementation::current());
  auto current_impl = p->second;

  std::cout << "The current Vc implementation is " << current_impl
            << std::endl;

  std::string expected_impl;
  if(auto env_impl = std::getenv(expected_var.c_str()))
    expected_impl = env_impl;

  if(expected_impl.empty())
  {
    std::cerr << "No expected vc implementation provided, skipping test\n"
              << "Please set " << expected_var
              << " environment variable to one of the following values:";
    for(const auto &item : impl_names)
      std::cerr << ' ' << item.second;
    std::cerr << std::endl;
    return 77;
  }

  std::cout << "The expected Vc implementation is " << expected_impl
            << std::endl;

  if(current_impl == expected_impl) {
    std::cout << "OK: Current and expected Vc implementation match"
              << std::endl;
    return 0;
  }
  else {
    std::cout << "Error: Current Vc implementation does not match expected"
              << std::endl;
    return 1;
  }

}
