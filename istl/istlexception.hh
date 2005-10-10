// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ISTLEXC_HH
#define DUNE_ISTLEXC_HH

#include <stdlib.h>

#include "dune/common/exceptions.hh"

namespace Dune {

  /**
              @addtogroup ISTL
              @{
   */

  //! derive error class from the base class in common
  class ISTLError : public Exception {};


  /** @} end documentation */

} // end namespace

#endif
