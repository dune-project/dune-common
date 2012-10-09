// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_POWER_HH
#define DUNE_COMMON_POWER_HH

/** \file
    \brief Various implementations of the power function for run-time and static arguments
 */

namespace Dune {

  /** @addtogroup Common

       @{
   */

  //! Calculates m^p at compile time
  template <int m, int p>
  struct Power_m_p
  {
    /** \brief power stores m^p */
    enum { power = (m * Power_m_p<m,p-1>::power ) };
  };

  //! end of recursion via specialization
  template <int m>
  struct Power_m_p< m , 0>
  {
    /** \brief m^0 = 1 */
    enum { power = 1 };
  };

}

#endif
