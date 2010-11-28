// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef MISC_HH
#define MISC_HH

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

namespace Dune {

  /** \brief Map an integer value to a type

      This comes in handy if one tries to emulate member function specialization.
      The idea how to do it is presented in "Modern C++ Design" by Alexandrescu.

      \deprecated This class is deprecated as of Dune 2.0+1 and will be removed
                  in Dune 2.0+2.  Please use integral_constant from
                  <dune/common/typetraits.hh> instead.

      Migration path: The most common use of Int2Type is a replacement of the
      disallowed partial specialization of functions: to select a particular
      function from an overloaded set, give it a dummy argument of type
      Int2Type.
      \code
     template<int N> void foo(const Int2Type<N>&, ...);
     void foo(const Int2Type<0>&, ...); // "specialization" for N=0
      \endcode
      Since Int2Type<...> is now derived from integral_constant<int,...>, the
      above can safely be replaced by
      \code
     template<int N> void foo(const integral_constant<int,N>&, ...);
     void foo(const integral_constant<int,0>&, ...); // "specialization" for N=0
      \endcode
      Compatibility overloads with Int2Type instead of
      integral_constant<int,...> are needed.  However, the call to such
      functions cannot be migrated that easily:
      \code
     // works for declaration foo(const Int2Type<dim>&, ...)
     // as well as foo(const integral_constant<int,dim>&, ...)
     foo(Int2Type<dim>(), args);
     // works for declaration foo(const integral_constant<int,dim>&, ...) only
     foo(integral_constant<int,dim>(), args);
      \endcode
      So, before a call is migrated, the declaration of all functions that are
      possibly called have to be migrated.
   */
  template <int N>
  struct DUNE_DEPRECATED Int2Type :
    public integral_constant<int,N>
  {
    /** \brief Export template parameter
     *
     * \deprecated This member is deprecated to make warnings more likely since
     *             deprecation of the class itself ins unreliable.
     */
    static const int DUNE_DEPRECATED value = N;

    /** \brief Default constructor
     *
     * \deprecated This member is deprecated to make warnings more likely since
     *             deprecation of the class itself is unreliable.
     */
    DUNE_DEPRECATED
    Int2Type() { }
  };

  // need definition here in case sombody passes "Int2Type<N>::value" as a
  // function argument or similar (otherwise the linker will barf)
  template <int N> const int Int2Type<N>::value;

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
  /**
   * \code
   *#include <dune/common/misc.hh>
   * \endcode
   */
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
    name << fn << std::setw(precision) << std::setfill('0') << ntime;

    // Return the string corresponding to the stringstream
    return name.str();
  }


  //********************************************************************
  //
  //  check whether a given container has a prefix/suffix
  //
  //********************************************************************

  //! check whether a character container has a given prefix
  /**
   * The container must support the the begin() and size() methods.
   */
  template<typename C>
  bool hasPrefix(const C& c, const char* prefix) {
    std::size_t len = std::strlen(prefix);
    return c.size() >= len &&
           std::equal(prefix, prefix+len, c.begin());
  }

  //! check whether a character container has a given suffix
  /**
   * The container must support the the begin() and size() methods and the
   * const_iterator member type.
   *
   * \note This is slow for containers which don't have random access iterators.
   *       In the case of containers with bidirectional iterators, this
   *       slow-ness is unnecessary.
   */
  template<typename C>
  bool hasSuffix(const C& c, const char* suffix) {
    std::size_t len = std::strlen(suffix);
    if(c.size() < len) return false;
    typename C::const_iterator it = c.begin();
    std::advance(it, c.size() - len);
    return std::equal(suffix, suffix+len, it);
  }
  /** @} */

}


#endif
