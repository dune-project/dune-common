// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdlib>
#include <iostream>
#include <locale>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/parametertree.hh>

// This assert macro does not depend on the value of NDEBUG
#define check_assert(expr)                                          \
  do                                                                \
  {                                                                 \
    if(!(expr))                                                     \
    {                                                               \
      std::cerr << __FILE__ << ":" << __LINE__ << ": check_assert(" \
                << #expr << ") failed" << std::endl;                \
      std::abort();                                                 \
    }                                                               \
  } while(false)

// Check that the given expression throws the given exception
#define check_throw(expr, except)                               \
  do {                                                          \
    try {                                                       \
      expr;                                                     \
      std::cerr << __FILE__ << ":" << __LINE__ << ": " << #expr \
                << " should throw " << #except << std::endl;    \
      std::abort();                                             \
    }                                                           \
    catch(const except&) {}                                            \
    catch(...) {                                                \
      std::cerr << __FILE__ << ":" << __LINE__ << ": " << #expr \
                << " should throw " << #except << std::endl;    \
      std::abort();                                             \
    }                                                           \
  } while(false)

// globally set a locale that uses "," as the decimal separator.
// return false if no such locale is installed on the system
bool setCommaLocale()
{
  static char const* const commaLocales[] = {
    "de", "de@euro", "de.UTF-8",
    "de_AT", "de_AT@euro", "de_AT.UTF-8",
    "de_BE", "de_BE@euro", "de_BE.UTF-8",
    "de_CH", "de_CH@euro", "de_CH.UTF-8",
    "de_DE", "de_DE@euro", "de_DE.UTF-8",
    "de_LI", "de_LI@euro", "de_LI.UTF-8",
    "de_LU", "de_LU@euro", "de_LU.UTF-8",
    NULL
  };
  for(char const* const* loc = commaLocales; *loc; ++loc)
  {
    try {
      std::locale::global(std::locale(*loc));
      std::cout << "Using comma-locale " << std::locale().name() << std::endl;
      return true;
    }
    catch(const std::runtime_error&) { }
  }

  std::cout << "No comma-using locale found on system, tried the following:";
  std::string sep = " ";
  for(char const* const* loc = commaLocales; *loc; ++loc)
  {
    std::cout << sep << *loc;
    sep = ", ";
  }
  std::cout << std::endl;
  return false;
}

int main()
{
  if(!setCommaLocale())
  {
    std::cerr << "No locale using comma as decimal separator found on system"
              << std::endl;
    return 77;
  }
  { // Try with comma
    Dune::ParameterTree ptree;
    check_throw((ptree["setting"] = "42,42",
                 ptree.get<double>("setting")),
                Dune::RangeError);
    check_throw((ptree["setting"] = "42 2,5",
                 ptree.get<Dune::FieldVector<double, 2> >("setting")),
                Dune::RangeError);
    check_throw((ptree["setting"] = "42 2,5",
                 ptree.get<std::vector<double> >("setting")),
                Dune::RangeError);
  }
  { // Try with point
    Dune::ParameterTree ptree;
    check_assert((ptree["setting"] = "42.42",
                  ptree.get<double>("setting") == 42.42));
    check_assert((ptree["setting"] = "42 2.5",
                  ptree.get<Dune::FieldVector<double, 2> >("setting")
                                  == Dune::FieldVector<double, 2>{42.0, 2.5}));
    check_assert((ptree["setting"] = "42 2.5",
                  ptree.get<std::vector<double> >("setting")
                                           == std::vector<double>{42.0, 2.5}));
  }
}
