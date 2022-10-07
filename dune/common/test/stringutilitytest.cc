// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <string>

#include <dune/common/stringutility.hh>

namespace {
const std::string hello_world("hello world");
} /* namespace */

bool test_hasPrefix()
{
  bool pass = true;

  using Dune::hasPrefix;
  pass &= hasPrefix(hello_world, "hello");
  pass &= !hasPrefix(hello_world, "world");

  return pass;
}

bool test_hasSuffix()
{
  bool pass = true;

  using Dune::hasSuffix;
  pass &= hasSuffix(hello_world, "world");
  pass &= !hasSuffix(hello_world, "hello");

  return pass;
}

bool test_formatString()
{
  bool pass = true;
  const int one = 1;
  const static std::string format("hello %i");
  const static std::string expected("hello 1");

  using Dune::formatString;
  const std::string s = formatString(format, one);
  pass &= (s == expected);

  return pass;
}

int main()
{
  bool pass = true;

  pass &= test_hasPrefix();
  pass &= test_hasSuffix();
  pass &= test_formatString();

  return pass ? 0 : 1;
}
