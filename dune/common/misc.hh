// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef MISC_HH
#define MISC_HH

#warning This file is deprecated.  Its contents have moved to stringutility.hh and math.hh, \
         or will disappear completely.

/** \file
    \brief Miscellaneous helper stuff
 */

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <complex>

#include <dune/common/deprecated.hh>
#include "exceptions.hh"
#include <dune/common/typetraits.hh>
#include <dune/common/stringutility.hh>
#include <dune/common/math.hh>

namespace Dune {


  /** @addtogroup Common

          @{
   */

  /** \brief Compute the square of T */
  /**
   * \code
   *#include <dune/common/misc.hh>
   * \endcode
   */
  template<class T>
  T SQR (T t) DUNE_DEPRECATED_MSG("Use method sqr from math.hh instead");

  template<class T>
  T SQR (T t)
  {
    return t*t;
  }

  //! Calculates m^p at compile time
  template <int m, int p>
  struct DUNE_DEPRECATED_MSG ("Use class StaticPower from file power.hh instead")Power_m_p
  {
    // power stores m^p
    enum { power = (m * Power_m_p<m,p-1>::power ) };
  };

  //! end of recursion via specialization
  template <int m>
  struct DUNE_DEPRECATED_MSG ("Use class StaticPower from file power.hh instead")Power_m_p< m , 0>
  {
    // m^0 = 1
    enum { power = 1 };
  };

  //********************************************************************
  //
  // generate filenames with timestep number in it
  //
  //********************************************************************

  /** \brief Generate filenames with timestep number in it */
  inline std::string genFilename(const std::string& path,
                                 const std::string& fn,
                                 int ntime,
                                 int precision = 6)
  {
    std::ostringstream name;

    if(path.size() > 0)
    {
      name << path;
      name << "/";
    }
    name << fn << std::setw(precision) << std::setfill('0') << ntime;

    // Return the string corresponding to the stringstream
    return name.str();
  }


  /** @} */

}


#endif
