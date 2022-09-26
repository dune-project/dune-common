// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

#include <ostream>
#include <string>

#include <dune/common/simd/test.hh>

void Dune::Simd::UnitTest::complain(const char *file, int line,
                                    const char *func, const char *expr)
{
  log_ << file << ":" << line << ": In " << func  << ": Error: check (" << expr
       << ") failed" << std::endl;
  good_ = false;
}

void Dune::Simd::UnitTest::
complain(const char *file, int line, const char *func,
         const std::string &opname, const char *expr)
{
  log_ << file << ":" << line << ": In " << func  << ", while testing "
       << opname << ": Error: check (" << expr << ") failed" << std::endl;
  good_ = false;
}
