// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_TEST_TESTSUITE_HH
#define DUNE_COMMON_TEST_TESTSUITE_HH

#include <iostream>
#include <sstream>
#include <string>

#include <dune/common/exceptions.hh>
#include <dune/common/test/collectorstream.hh>



namespace Dune {



  /**
   * \brief A Simple helper class to organize your test suite
   *
   * Usage: Construct a TestSuite and call check() or require()
   * with the condition to check and probably a name for this check.
   * These methods return a stream such that you can pipe in an
   * explanantion accompanied by respective data to give a reason
   * for a test failure.
   */
  class TestSuite
  {
  public:
    enum ThrowPolicy
    {
      AlwaysThrow,
      ThrowOnRequired
    };

    /**
     * \brief Create TestSuite
     *
     * \param name A name to identify this TestSuite. Defaults to "".
     * \param policy If AlwaysThrow any failing check will throw, otherwise only required checks will do.
     */
    TestSuite(ThrowPolicy policy, std::string name="") :
      name_(name),
      checks_(0),
      failedChecks_(0),
      throwPolicy_(policy==AlwaysThrow)
    {}

    /**
     * \brief Create TestSuite
     *
     * \param name A name to identify this TestSuite. Defaults to "".
     * \param policy If AlwaysThrow any failing check will throw, otherwise only required checks will do. Defaults to ThrowOnRequired
     */
    TestSuite(std::string name="", ThrowPolicy policy=ThrowOnRequired) :
      name_(name),
      checks_(0),
      failedChecks_(0),
      throwPolicy_(policy==AlwaysThrow)
    {}

    /**
     * \brief Check condition
     *
     * This will throw an exception if the check fails and if the AlwaysThrow policy was used on creation.
     *
     * \param condition Checks if this is true and increases the failure counter if not.
     * \param name A name to identify this check. Defaults to ""
     * \returns A CollectorStream that can be used to create a diagnostic message to be printed on failure.
     */
    CollectorStream check(bool condition, std::string name="")
    {
      ++checks_;
      if (not condition)
        ++failedChecks_;

      return CollectorStream([condition, name, this](std::string reason) {
          if (not condition)
            this->announceCheckResult(throwPolicy_, "CHECK  ", name, reason);
        });
    }

    /**
     * \brief Check a required condition
     *
     * This will always throw an exception if the check fails.
     *
     * \param condition Checks if this is true and increases the failure counter if not.
     * \param name A name to identify this check. Defaults to ""
     * \returns A CollectorStream that can be used to create a diagnostic message to be printed on failure.
     */
    CollectorStream require(bool condition, std::string name="")
    {
      ++checks_;
      if (not condition)
        ++failedChecks_;

      return CollectorStream([condition, name, this](std::string reason) {
          if (not condition)
            this->announceCheckResult(true, "REQUIRED CHECK", name, reason);
        });
    }

    /**
     * \brief Collect data from a sub-TestSuite
     *
     * This will incorporate the accumulated results of the sub-TestSuite
     * into this one. If the sub-TestSuite failed, i.e., contained failed
     * checks, a summary will be printed.
     */
    void subTest(const TestSuite& subTest)
    {
      checks_ += subTest.checks_;
      failedChecks_ += subTest.failedChecks_;

      if (not subTest)
        announceCheckResult(throwPolicy_, "SUBTEST", subTest.name(), std::to_string(subTest.failedChecks_)+"/"+std::to_string(subTest.checks_) + " checks failed in this subtest.");
    }

    /**
     * \brief Check if this TestSuite failed
     *
     * \returns False if any of the executed tests failed, otherwise true.
     */
    explicit operator bool () const
    {
      return (failedChecks_==0);
    }

    /**
     * \brief Query name
     *
     * \returns Name of this TestSuite
     */
    std::string name() const
    {
      return name_;
    }

    /**
     * \brief Print a summary of this TestSuite
     *
     * \returns False if any of the executed tests failed, otherwise true.
     */
    bool report() const
    {
      if (failedChecks_>0)
        std::cout << composeMessage("TEST   ", name(), std::to_string(failedChecks_)+"/"+std::to_string(checks_) + " checks failed in this test.") << std::endl;
      return (failedChecks_==0);
    }

    /**
     * \brief Exit the test.
     *
     * This will print a summary of the test and return an integer
     * to be used on program exit.
     *
     * \returns 1 if any of the executed tests failed, otherwise 0.
     */
    int exit() const
    {
      return (report() ? 0: 1);
    }

  protected:

    // Compose a diagnostic message
    static std::string composeMessage(std::string type, std::string name, std::string reason)
    {
      std::ostringstream s;
      s << type << " FAILED";
      if (name!="")
        s << "(" << name << ")";
      s << ": ";
      if (reason!="")
        s << reason;
      return s.str();
    }

    // Announce check results. To be called on failed checks
    static void announceCheckResult(bool throwException, std::string type, std::string name, std::string reason)
    {
      std::string message = composeMessage(type, name, reason);
      std::cout << message << std::endl;
      if (throwException)
      {
        Dune::Exception ex;
        ex.message(message);
        throw ex;
      }
    }

    std::string name_;
    std::size_t checks_;
    std::size_t failedChecks_;
    bool throwPolicy_;
  };



} // namespace Dune



#endif // DUNE_COMMON_TEST_TESTSUITE_HH
