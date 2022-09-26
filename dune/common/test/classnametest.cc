// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include "config.h"

#include <bitset>
#include <complex>
#include <iostream>
#include <regex>
#include <string>

#include <dune/common/classname.hh>
#include <dune/common/fvector.hh>
#include <dune/common/test/testsuite.hh>


using CVRef = std::bitset<4>;
constexpr CVRef is_const            = 1;
constexpr CVRef is_volatile         = 2;
constexpr CVRef is_lvalue_reference = 4;
constexpr CVRef is_rvalue_reference = 8;
constexpr CVRef is_reference        = 12;

void checkname(Dune::TestSuite &t, const std::string &name, CVRef cvref,
               const std::string &pattern)
{
  const auto npos = std::string::npos;

  std::cout << name << std::endl;

  t.check(std::regex_search(name, std::regex{pattern}))
    << '`' << name << "` does not look like `" << pattern << '`';

  static const std::regex const_pattern{ R"(\bconst\b)" };
  bool found_const = std::regex_search(name, const_pattern);
  if((cvref & is_const) == is_const)
    t.check(found_const) << '`' << name << "` contains `const`";
  else
    t.check(!found_const) << '`' << name << "` does not contain `const`";

  static const std::regex volatile_pattern{ R"(\bvolatile\b)" };
  bool found_volatile = std::regex_search(name, volatile_pattern);
  if((cvref & is_volatile) == is_volatile)
    t.check(found_volatile) << '`' << name << "` contains `volatile`";
  else
    t.check(!found_volatile) << '`' << name << "` does not contain `volatile`";

  bool found_reference = name.find('&') != npos;
  bool found_rvalue_reference = name.find("&&") != npos;
  if((cvref & is_reference) == is_reference)
    t.check(found_reference)
      << '`' << name << "` does not contain `&` or `&&`";
  else if((cvref & is_lvalue_reference) == is_lvalue_reference)
    t.check(found_reference && !found_rvalue_reference)
      << '`' << name << "` contains `&&` or does not contain `&`";
  else if((cvref & is_rvalue_reference) == is_rvalue_reference)
    t.check(found_rvalue_reference)
      << '`' << name << "` does not contain `&&`";
  else
    t.check(!found_reference)
      << '`' << name << "` contains `&` or `&&`";
}

struct Base {
  virtual ~Base() = default;
};

struct Derived : Base {};

int main()
{
  Dune::TestSuite t("className()");

  std::cout << "First three simple class names extracted from variables:"
            << std::endl;
  Dune::FieldVector<int, 3> xi;
  checkname(t, Dune::className(xi), {},
            R"(\bFieldVector\s*<\s*int\s*,\s*3\s*>)");
  Dune::FieldVector<double, 1> xd;
  checkname(t, Dune::className(xd), {},
            R"(\bFieldVector\s*<\s*double\s*,\s*1\s*>)");
  Dune::FieldVector<std::complex<double>, 10> xcd;
  checkname(t, Dune::className(xcd), {},
            R"(\bFieldVector\s*<.*\bcomplex\s*<\s*double\s*>\s*,\s*10\s*>)");
  std::cout << std::endl;

  std::cout << "Adding const:" << std::endl;
  const Dune::FieldVector<int, 3> cxi;
  checkname(t, Dune::className(cxi), is_const,
            R"(\bFieldVector\s*<\s*int\s*,\s*3\s*>)");
  std::cout << std::endl;

  std::cout << "If a variable is a reference that can not be extracted (needs "
            << "decltype as used below): " << std::endl;
  Dune::FieldVector<double, 1> &rxd = xd;
  checkname(t, Dune::className(rxd), {},
            R"(\bFieldVector\s*<\s*double\s*,\s*1\s*>)");
  std::cout << std::endl;

  std::cout << "Extracting the class name using a type directly - "
            << "also extractes references correctly: " << std::endl;
  checkname(t, Dune::className<decltype(rxd)>(), is_lvalue_reference,
            R"(\bFieldVector\s*<\s*double\s*,\s*1\s*>)");
  const Dune::FieldVector<double, 1> &rcxd = xd;
  checkname(t, Dune::className<decltype(rcxd)>(), is_const|is_lvalue_reference,
            R"(\bFieldVector\s*<\s*double\s*,\s*1\s*>)");
  const Dune::FieldVector<int, 3> &rcxi = cxi;
  checkname(t, Dune::className<decltype(rcxi)>(), is_const|is_lvalue_reference,
            R"(\bFieldVector\s*<\s*int\s*,\s*3\s*>)");
  std::cout << std::endl;

  std::cout << "Test some further types:" << std::endl;
  using RVXCD = volatile Dune::FieldVector<std::complex<double>, 10>&;
  checkname(t, Dune::className<RVXCD>(), is_volatile|is_lvalue_reference,
            R"(\bFieldVector\s*<.*\bcomplex\s*<\s*double\s*>\s*,\s*10\s*>)");
  using RRXCD = Dune::FieldVector<std::complex<double>, 10>&&;
  checkname(t, Dune::className<RRXCD>(), is_rvalue_reference,
            R"(\bFieldVector\s*<.*\bcomplex\s*<\s*double\s*>\s*,\s*10\s*>)");
  std::cout << std::endl;

  std::cout << "Test printing dynamic vs. static types:" << std::endl;
  Derived d{};
  Base &b = d;
  checkname(t, Dune::className(b), {}, R"(\bDerived\b)");
  checkname(t, Dune::className<decltype(b)>(), is_lvalue_reference,
            R"(\bBase\b)");
  t.check(Dune::className<Derived>() == Dune::className(b))
    << "dynamic type of base reference should match derived type";
  std::cout << std::endl;

  std::cout << "Test rvalue argument to className(expr):" << std::endl;
  checkname(t, Dune::className(Base{}), {}, R"(\bBase\b)");
  std::cout << std::endl;

  #if !HAVE_CXA_DEMANGLE
  // in this case we only make sure that no segfault or similar happens
  return 0;
  #else
  return t.exit();
  #endif
}
