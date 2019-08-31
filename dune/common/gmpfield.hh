// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GMPFIELD_HH
#define DUNE_GMPFIELD_HH

/** \file
 * \brief Wrapper for the GNU MPFR multiprecision floating point library
 */

#include <iostream>
#include <string>
#include <type_traits>

#if HAVE_MPFR || DOXYGEN

#include <mpreal.h>

#include <dune/common/typetraits.hh>

namespace Dune
{

  /**
   * \ingroup Numbers
   * \brief Number class for high precision floating point number using the MPFR library mpreal implementation
   */
  template< unsigned int precision >
  class GMPField
    : public mpfr::mpreal
  {
    typedef mpfr::mpreal Base;

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

    /**  \brief initialize from from mpreal value
     */
    GMPField ( const Base &v )
      : Base( v )
    {}

    /** \brief initialize from a compatible scalar type
     */
    template< class T,
              typename EnableIf = typename std::enable_if<
                std::is_convertible<T, mpfr::mpreal>::value>::type
              >
    GMPField ( const T &v )
      : Base( v,precision )
    {}

    // type conversion operators
    operator double () const
    {
      return this->toDouble();
    }

  };

  template <unsigned int precision>
  struct IsNumber<GMPField<precision>>
    : public std::integral_constant<bool, true> {
  };

}

namespace std
{
  /// Specialization of numeric_limits for known precision width
  template <unsigned int precision>
  class numeric_limits<Dune::GMPField<precision>>
      : public numeric_limits<mpfr::mpreal>
  {
    using type = Dune::GMPField<precision>;

  public:
    inline static type min () { return mpfr::minval(precision); }
    inline static type max () {  return  mpfr::maxval(precision); }
    inline static type lowest () { return -mpfr::maxval(precision); }
    inline static type epsilon () { return  mpfr::machine_epsilon(precision); }

    inline static type round_error ()
    {
      mp_rnd_t r = mpfr::mpreal::get_default_rnd();

      if(r == GMP_RNDN)  return mpfr::mpreal(0.5, precision);
      else               return mpfr::mpreal(1.0, precision);
    }

    inline static int digits () { return int(precision); }
    inline static int digits10 () { return mpfr::bits2digits(precision); }
    inline static int max_digits10 () { return mpfr::bits2digits(precision); }
  };

} // end namespace std

#endif // HAVE_MPFR

#endif // #ifndef DUNE_GMPFIELD_HH
