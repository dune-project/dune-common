// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_GMPFIELD_HH
#define DUNE_GMPFIELD_HH

/** \file
 * \brief Wrapper for the GNU multiprecision (GMP) library
 */

#include <iostream>
#include <string>
#include <type_traits>

#if HAVE_GMP || DOXYGEN

#include <gmpxx.h>

#include <dune/common/promotiontraits.hh>
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

  template< unsigned int precision1, unsigned int precision2 >
  struct PromotionTraits<GMPField<precision1>, GMPField<precision2>>
  {
    typedef GMPField<(precision1 > precision2 ? precision1 : precision2)> PromotedType;
  };

  template< unsigned int precision >
  struct PromotionTraits<GMPField<precision>,GMPField<precision>>
  {
    typedef GMPField<precision> PromotedType;
  };

  template< unsigned int precision, class T >
  struct PromotionTraits<GMPField<precision>, T>
  {
    typedef GMPField<precision> PromotedType;
  };

  template< class T, unsigned int precision >
  struct PromotionTraits<T, GMPField<precision>>
  {
    typedef GMPField<precision> PromotedType;
  };
}

#endif // HAVE_GMP

#endif // #ifndef DUNE_GMPFIELD_HH
