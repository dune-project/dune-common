// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GMPFIELD_HH
#define DUNE_GMPFIELD_HH

/** \file
 * \brief Wrapper for the GNU multiprecision (GMP) library
 */

#include <iostream>

#if HAVE_GMP

#include <gmpxx.h>

namespace Dune
{

  template< unsigned int precision >
  class GMPField
    : public mpf_class
  {
    typedef mpf_class Base;

  public:
    GMPField ()
      : Base(0,precision)
    {}

    template< class T >
    GMPField ( const T &v )
      : Base( v,precision )
    {}

    /*
       GMPField &operator=(const GMPField &other)
       {
       Base(*this) = Base(other);
       return *this;
       }
     */

    // type conversion operators
    operator double () const
    {
      return this->get_d();
    }

    operator float () const
    {
      return this->get_d();
    }
  };



  template< unsigned int precision >
  inline GMPField< precision >
  operator+ ( const GMPField< precision > &a, const GMPField< precision > &b )
  {
    typedef mpf_class F;
    return ((const F &)a + (const F &)b);
  }

  template< unsigned int precision >
  inline GMPField< precision >
  operator- ( const GMPField< precision > &a, const GMPField< precision > &b )
  {
    typedef mpf_class F;
    return ((const F &)a - (const F &)b);
  }

  template< unsigned int precision >
  inline GMPField< precision >
  operator- ( const GMPField< precision > &a )
  {
    typedef mpf_class F;
    return -((const F &)a);
  }

  template< unsigned int precision >
  inline GMPField< precision >
  operator* ( const GMPField< precision > &a, const GMPField< precision > &b )
  {
    typedef mpf_class F;
    return ((const F &)a * (const F &)b);
  }

  template< unsigned int precision >
  inline GMPField< precision >
  operator/ ( const GMPField< precision > &a, const GMPField< precision > &b )
  {
    typedef mpf_class F;
    return ((const F &)a / (const F &)b);
  }



  template< unsigned int precision >
  inline std::ostream &
  operator<< ( std::ostream &out, const GMPField< precision > &value )
  {
    return out << static_cast<const mpf_class&>(value);
  }

}

namespace std
{

  template< unsigned int precision >
  inline Dune::GMPField< precision >
  sqrt ( const Dune::GMPField< precision > &a )
  {
    return Dune::GMPField< precision >(sqrt(static_cast<const mpf_class&>(a)));
  }

  template< unsigned int precision >
  inline Dune::GMPField< precision >
  abs ( const Dune::GMPField< precision > &a )
  {
    return Dune::GMPField< precision >( abs( static_cast< const mpf_class & >( a ) ) );
  }

}

#endif // HAVE_GMP

#endif // #ifndef DUNE_GMPFIELD_HH
