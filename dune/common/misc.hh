// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef MISC_HH
#define MISC_HH

/** \file
    \brief Miscellaneous helper stuff
 */

#include <iostream>
#include <sstream>
#include <complex>
#include "exceptions.hh"

namespace Dune {

  /** \brief Map an integer value to a type

      This comes in handy if one tries to emulate member function specialization.
      The idea how to do it is presented in "Modern C++ Design" by Alexandrescu.
   */
  template <int N>
  struct Int2Type {
    enum { value = N };
  };

  /** @addtogroup Common

          @{
   */

  //! compute conjugate complex of x
  // conjugate complex does nothing for non-complex types
  template<class K>
  inline K conjugateComplex (const K& x)
  {
    return x;
  }

#ifndef DOXYGEN
  // specialization for complex
  template<class K>
  inline std::complex<K> conjugateComplex (const std::complex<K>& c)
  {
    return std::complex<K>(c.real(),-c.imag());
  }
#endif

  //! Return the sign of the value
  template <class T>
  int sign(const T& val)
  {
    return (val < 0 ? -1 : 1);
  }

  /** \brief Compute the square of T */
  template<class T>
  T SQR (T t)
  {
    return t*t;
  }

  //! Calculates m^p at compile time
  template <int m, int p>
  struct Power_m_p
  {
    // power stores m^p
    enum { power = (m * Power_m_p<m,p-1>::power ) };
  };

  //! end of recursion via specialization
  template <int m>
  struct Power_m_p< m , 0>
  {
    // m^0 = 1
    enum { power = 1 };
  };

  //! Calculates the factorial of m at compile time
  template <int m>
  struct Factorial
  {
    //! factorial stores m!
    enum { factorial = m * Factorial<m-1>::factorial };
  };

  //! end of recursion of factorial via specialization
  template <>
  struct Factorial<0>
  {
    // 0! = 1
    enum { factorial = 1 };
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
    name << fn;

    char cp[256];
    switch(precision)
    {
    case 2  : { sprintf(cp, "%02d", ntime); break; }
    case 3  : { sprintf(cp, "%03d", ntime); break; }
    case 4  : { sprintf(cp, "%04d", ntime); break; }
    case 5  : { sprintf(cp, "%05d", ntime); break; }
    case 6  : { sprintf(cp, "%06d", ntime); break; }
    case 7  : { sprintf(cp, "%07d", ntime); break; }
    case 8  : { sprintf(cp, "%08d", ntime); break; }
    case 9  : { sprintf(cp, "%09d", ntime); break; }
    case 10 : { sprintf(cp, "%010d", ntime); break; }
    default :
    {
      DUNE_THROW(Exception, "Couldn't gernerate filename with precision = "<<precision);
    }
    }
    name << cp;

    // here implicitly a string is generated
    return name.str();
  }

  /** @} */

}


#endif
