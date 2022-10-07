// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifndef DUNE_COMMON_STDTHREAD_HH
#define DUNE_COMMON_STDTHREAD_HH

namespace Dune
{

  // used internally by assertCallOnce for the actual check
  void doAssertCallOnce(const char *file, int line, const char *function);

  //! \brief Make sure call_once() works and provide a helpful error message
  //!        otherwise.
  /**
   * For call_once() to work, certain versions of libstdc++ need to be
   * _linked_ with -pthread or similar flags.  If that is not the case,
   * call_once() will throw an exception.  This function checks that
   * call_once() can indeed be used, i.e. that it does not throw an exception
   * when it should not, and that the code does indeed get executed.  If
   * call_once() cannot be used, assertCallOnce() aborts the program with a
   * helpful error message.
   *
   * The check is only actually executed the first time assertCallOnce() is
   * called.
   *
   * The arguments \c file and \c line specify the filename and line number
   * that should appear in the error message.  They are ignored if \c file is
   * 0.  The argument \c function specifies the name of the function to appear
   * in the error message.  It is ignored if \c function is 0.
   */

  inline void assertCallOnce(const char *file = nullptr, int line = -1,
                             const char *function = nullptr)
  {
    // make sure to call this only the first time this function is invoked
    [[maybe_unused]] static const bool works
      = (doAssertCallOnce(file, line, function), true);
  }

  //! \brief Make sure call_once() works and provide a helpful error message
  //!        otherwise.
  /**
   * This calls assertCallOnce() and automatically provides information about
   * the caller in the error message.
   */
#define DUNE_ASSERT_CALL_ONCE()                         \
  ::Dune::assertCallOnce(__FILE__, __LINE__, __func__)

} // namespace Dune

#endif // DUNE_COMMON_STDTHREAD_HH
