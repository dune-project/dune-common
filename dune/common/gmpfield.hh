// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GMPFIELD_HH
#define DUNE_GMPFIELD_HH

/** \file
 * \brief Wrapper for the GNU multiprecision (GMP) library
 */

#include <iostream>
#include <string>

#if HAVE_GMP || DOXYGEN

#include <gmpxx.h>

#include <dune/common/typetraits.hh>

namespace Dune
{

  /**
   * \ingroup Numbers
   * \brief Number class for high precision floating point number using the GMP library mpf_class implementation
   */
  template< unsigned int precision >
  class GMPField
    : public mpf_class
  {
    typedef mpf_class Base;

  public:
    /** default constructor, initialize to zero */
    GMPField ()
      : Base(0,precision)
    {}

    /** \brief initialize from a string
        \note this is the only reliable way to initialize with higher precision values
     */
    GMPField ( const char* str )
      : Base(str,precision)
    {}

    /** \brief initialize from a string
        \note this is the only reliable way to initialize with higher precision values
     */
    GMPField ( const std::string& str )
      : Base(str,precision)
    {}

    /** \brief initialize from a compatible scalar type
     */
    template< class T,
              typename EnableIf = typename std::enable_if<
                std::is_convertible<T, mpf_class>::value>::type
              >
    GMPField ( const T &v )
      : Base( v,precision )
    {}

    // type conversion operators
    operator double () const
    {
      return this->get_d();
    }

  };

  template <unsigned int precision>
  struct IsNumber<GMPField<precision>>
    : public std::integral_constant<bool, true> {
  };
}

#endif // HAVE_GMP

#endif // #ifndef DUNE_GMPFIELD_HH
