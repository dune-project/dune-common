// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#include <regex>

#include <dune/common/exceptions.hh>
#include <dune/common/test/testsuite.hh>

constexpr int throwOn42(int i)
{
  if (i==42)
    DUNE_THROW(Dune::RangeError, "Calling throwOn42(42)");
  return i;
}

struct TestClass
{
  static const bool a = true;
  static const int b = 2;
  static const std::size_t c = 3;
};

int main()
{
  Dune::TestSuite test;

  // Check for correct exception type
  {
    test.checkThrow<Dune::RangeError>([](){
      DUNE_THROW(Dune::RangeError, "foo" << "123");
    }) << "DUNE_THROW did not create expected exception type";
  }

  // Check if piping additional values after DUNE_THROW works
  {
    test.checkThrow<Dune::RangeError>([](){
      DUNE_THROW(Dune::RangeError, "foo" << "1") << "2" << 3;
    }) << "DUNE_THROW did not create expected exception type";
  }

  // Check if only piping values after DUNE_THROW works
  {
    test.checkThrow<Dune::RangeError>([](){
      DUNE_THROW(Dune::RangeError) << "foo" << "1" << "2" << 3;
    }) << "DUNE_THROW did not create expected exception type";
  }

  // Check for correct exception value with catch by reference (only piping inside DUNE_THROW)
  {
    std::string message;
    try {
      DUNE_THROW(Dune::RangeError, "foo" << "1" << "2" << 3);
    }
    catch(Dune::RangeError& e) {
      message = e.what();
    }
    test.check(std::regex_match(message, std::regex(".*\\]: foo123")))
      << "DUNE_THROW did not create expected message but '" << message << "'";
  }

  // Check for correct exception value with catch by reference (only piping outside DUNE_THROW)
  {
    std::string message;
    try {
      DUNE_THROW(Dune::RangeError) << "foo" << "1" << "2" << 3;
    }
    catch(Dune::RangeError& e) {
      message = e.what();
    }
    test.check(std::regex_match(message, std::regex(".*\\]: foo123")))
      << "DUNE_THROW did not create expected message but '" << message << "'";
  }

  // Check for correct exception value with catch by reference (piping in- and outside DUNE_THROW)
  {
    std::string message;
    try {
      DUNE_THROW(Dune::RangeError, "foo" << "1") << "2" << 3;
    }
    catch(Dune::RangeError& e) {
      message = e.what();
    }
    test.check(std::regex_match(message, std::regex(".*\\]: foo123")))
      << "DUNE_THROW did not create expected message but '" << message << "'";
  }

  // Check for correct exception value with catch by value
  {
    std::string message;
    try {
      DUNE_THROW(Dune::RangeError, "foo" << "1") << "2" << 3;
    }
    catch(Dune::RangeError e) {
      message = e.what();
    }
    test.check(std::regex_match(message, std::regex(".*\\]: foo123")))
      << "DUNE_THROW did not create expected message but '" << message << "'";
  }

  // Check if passing undefined static const int members works
  {
    std::string message;
    try {
      DUNE_THROW(Dune::RangeError, "foo" << TestClass::a << TestClass::b << TestClass::c);
    }
    catch(Dune::RangeError& e) {
      message = e.what();
    }
    test.check(std::regex_match(message, std::regex(".*\\]: foo123")))
      << "DUNE_THROW did not create expected message but '" << message << "'";
  }

  // Check if throwing from constexpr function works
  {
    test.checkThrow<Dune::RangeError>([](){
      throwOn42(42);
    }) << "DUNE_THROW did not create expected exception type in constexpr context";
  }

  // Check if DUNE_THROW is allowed in constexpr context
  {
    test.check(std::integral_constant<int, throwOn42(23)>()==23)
      << "DUNE_THROW cannot be used in constexpr context";
  }

  return test.exit();
}
