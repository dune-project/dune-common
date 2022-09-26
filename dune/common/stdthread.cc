// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdlib>
#include <iostream>
#include <mutex>
#include <ostream>

#include <dune/common/stdthread.hh>

namespace Dune
{

  namespace {

    void printCallOnceError(const char *file, int line, const char *function,
                            const char *msg)
    {
      if(file)
        std::cerr << file << ":" << line << ": ";
      std::cerr << "error: ";
      if(function)
        std::cerr << "(in " << function << "()) ";
      std::cerr << "std::call_once() is broken.\n"
                << "\n"
                << msg << std::endl;
    }

    void setBool(bool *v)
    {
      *v = true;
    }

  } // anonymous namespace

  void doAssertCallOnce(const char *file, int line, const char *function)
  {
    std::once_flag once;
    bool works = false;
    try {
      // pass address to works since call_once passes by value
      std::call_once(once, setBool, &works);
    }
    catch(...) {
      printCallOnceError(file, line, function,
"std::call_once() throws an exception.  This suggests that the program was\n"
"linked without a threading library.  Common ways to link to a threading\n"
"library is to specify one of the following during linking: -pthread, \n"
"-lpthread, or -pthreads.  The build system should have tried various of\n"
"these options, but unfortunately that is only a guess and we cannot verify\n"
"that we found a working configuration until runtime.\n"
"\n"
"Going to rethrow the exception now to give the system library a chance to\n"
"print more information about it, just in case that helps with debugging.\n"
                         );
      throw;
    }
    if(!works)
    {
      printCallOnceError(file, line, function,
"std::call_once() never calls the function.  This suggests that your\n"
"libctdc++ or your gcc built without threading support (--disable-threads,\n"
"see https://gcc.gnu.org/install/configure.html).  This is probably a bug in\n"
"__gthread_once() in /usr/include/c++/4.7/x86_64-linux-gnu/bits/gthr-single.h\n"
"(which should not silently return success without doing anything, but\n"
"apparently does so in some versions).\n"
"\n"
"To fix the issue, either recompile gcc with a working threading\n"
"implementation, or file a bug for gthr-single.h, or file a bug at\n"
"https://dune-project.org/flyspray/ and request a workaround at the dune-side."
                         );
      std::abort();
    }
  }

} // namespace Dune
