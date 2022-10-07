// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <dune/common/exceptions.hh>

namespace Dune {
  /*
    static member of Dune::Exception
  */
  ExceptionHook * Exception::_hook = 0;

  /*
     Implementation of Dune::Exception
   */
  Exception::Exception ()
  {
    // call the hook if necessary
    if (_hook != 0) _hook->operator()();
  }

  void Exception::registerHook (ExceptionHook * hook)
  {
    _hook = hook;
  }

  void Exception::clearHook ()
  {
    _hook = 0;
  }

  void Exception::message(const std::string & msg)
  {
    _message = msg;
  }

  const char* Exception::what() const noexcept
  {
    return _message.data();
  }

}
