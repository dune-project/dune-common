// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

#include <functional>

#include <dune/common/parallel/mpihelper.hh>

#include <dune/common/referencehelper.hh>

#include <dune/common/test/testsuite.hh>



class CopyCounter
{
public:
  CopyCounter() : count_(0) {}
  CopyCounter(std::size_t count) : count_(count) {}
  CopyCounter(const CopyCounter& other) :
    count_(other.count_ + 1)
  {}

  auto& getCount() {
    return count_;
  }

  const auto& getCount() const {
    return count_;
  }

  void setCount(std::size_t count) {
    count_ = count;
  }

private:
  mutable std::size_t count_;
};


int main (int argc, char *argv[]) try
{
  // Set up MPI, if available
  Dune::MPIHelper::instance(argc, argv);

  Dune::TestSuite suite;

  suite.subTest([]() {
    Dune::TestSuite subSuite("Checking with mutable l-value");
    CopyCounter c;
    Dune::resolveRef(c).setCount(42);
    Dune::resolveRef(c).getCount();
    subSuite.check(Dune::resolveRef(c).getCount() == 42, "Checking resolveRef");
    subSuite.check(not Dune::IsReferenceWrapper_v<decltype(c)>, "Checking IsReferenceWrapper_v");
    return subSuite;
  }());

  suite.subTest([]() {
    Dune::TestSuite subSuite("Checking with const l-value");
    const CopyCounter c(42);
    Dune::resolveRef(c).getCount();
    subSuite.check(Dune::resolveRef(c).getCount() == 42, "Checking resolveRef");
    subSuite.check(not Dune::IsReferenceWrapper_v<decltype(c)>, "Checking IsReferenceWrapper_v");
    return subSuite;
  }());

  suite.subTest([]() {
    Dune::TestSuite subSuite("Checking with mutable reference_wrapper of mutable l-value");
    CopyCounter c;
    auto c_ref = std::ref(c);
    Dune::resolveRef(c_ref).setCount(42);
    Dune::resolveRef(c_ref).getCount();
    subSuite.check(Dune::resolveRef(c_ref).getCount() == 42, "Checking resolveRef");
    subSuite.check(Dune::IsReferenceWrapper_v<decltype(c_ref)>, "Checking IsReferenceWrapper_v");
    return subSuite;
  }());

  suite.subTest([]() {
    Dune::TestSuite subSuite("Checking with const reference_wrapper of mutable l-value");
    CopyCounter c;
    const auto c_ref = std::ref(c);
    Dune::resolveRef(c_ref).setCount(42);
    Dune::resolveRef(c_ref).getCount();
    subSuite.check(Dune::resolveRef(c_ref).getCount() == 42, "Checking resolveRef");
    subSuite.check(Dune::IsReferenceWrapper_v<decltype(c_ref)>, "Checking IsReferenceWrapper_v");
    return subSuite;
  }());

  suite.subTest([]() {
    Dune::TestSuite subSuite("Checking with mutable reference_wrapper of const l-value");
    const CopyCounter c(42);
    auto c_ref = std::ref(c);
    Dune::resolveRef(c_ref).getCount();
    subSuite.check(Dune::resolveRef(c_ref).getCount() == 42, "Checking resolveRef");
    subSuite.check(Dune::IsReferenceWrapper_v<decltype(c_ref)>, "Checking IsReferenceWrapper_v");
    return subSuite;
  }());

  suite.subTest([]() {
    Dune::TestSuite subSuite("Checking with const reference_wrapper of const l-value");
    const CopyCounter c(42);
    const auto c_ref = std::ref(c);
    Dune::resolveRef(c_ref).getCount();
    subSuite.check(Dune::resolveRef(c_ref).getCount() == 42, "Checking resolveRef");
    subSuite.check(Dune::IsReferenceWrapper_v<decltype(c_ref)>, "Checking IsReferenceWrapper_v");
    return subSuite;
  }());

  suite.subTest([]() {
    Dune::TestSuite subSuite("Checking with const reference_wrapper of const l-value (via std::cref)");
    CopyCounter c(42);
    auto c_ref = std::cref(c);
    Dune::resolveRef(c_ref).getCount();
    subSuite.check(Dune::resolveRef(c_ref).getCount() == 42, "Checking resolveRef");
    subSuite.check(Dune::IsReferenceWrapper_v<decltype(c_ref)>, "Checking IsReferenceWrapper_v");
    return subSuite;
  }());

  suite.subTest([]() {
    Dune::TestSuite subSuite("Checking with const reference_wrapper r-value of mutable l-value");
    CopyCounter c;
    Dune::resolveRef(std::ref(c)).setCount(42);
    Dune::resolveRef(std::ref(c)).getCount();
    subSuite.check(Dune::resolveRef(std::ref(c)).getCount() == 42, "Checking resolveRef");
    subSuite.check(Dune::IsReferenceWrapper_v<decltype(std::ref(c))>, "Checking IsReferenceWrapper_v");
    return subSuite;
  }());

  return suite.exit();
}
catch (std::exception& e) {
  std::cout << e.what() << std::endl;
  return 1;
}
