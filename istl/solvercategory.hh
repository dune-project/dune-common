// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_SOLVERCATEGORY_HH
#define DUNE_SOLVERCATEGORY_HH


namespace Dune {

  /**
              @addtogroup ISTL
              @{
   */

  struct SolverCategory
  {
    enum { sequential, nonoverlapping, overlapping };
  };

  /** @} end documentation */

} // end namespace

#endif
