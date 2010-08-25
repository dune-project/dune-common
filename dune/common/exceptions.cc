// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <dune/common/exceptions.hh>

namespace Dune {
  // static member of Dune::Exception
  ExceptionHook * Exception::_hook = 0;
}
