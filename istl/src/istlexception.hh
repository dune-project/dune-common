// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ISTLEXC_HH__
#define __DUNE_ISTLEXC_HH__

#include <stdlib.h>

#include "../../common/exceptions.hh"

namespace Dune {

  /** @defgroup ISTL Iterative Solvers Template Library
              @addtogroup ISTL
              @{
   */

  //! derive error class from the base class in common
  class ISTLError : public Exception {};


  /** @} end documentation */

} // end namespace

#endif
