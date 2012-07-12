// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_PROMOTIONTRAITS_HH
#define DUNE_PROMOTIONTRAITS_HH

#include <complex>

namespace Dune {
  /**
   * @file
   * @brief  Provides some promotion traits
   *
   * For example, the promotion traits are used for the implementation of dot products @see <dune/common/dotproduct.hh>
   * @author Matthias Wohlmuth
   */

  /** @addtogroup Common
   *
   * @{
   */

  /**
  ** \brief Class for type promotions. For example, the promotion traits are used for the implementation of dot products @see <dune/common/dotproduct.hh>
  **  \private basic template:
  */
  template <typename T1, typename T2>
  struct PromotionTraits { };

#ifndef DOXYGEN
  // class for type promotion; // the same types are the same:
  template <typename T1>
  struct PromotionTraits<T1,T1> { typedef T1 PromotedType; };

  // promote to complex type
  template <typename T1>
  struct PromotionTraits<std::complex<T1>,T1> { typedef std::complex<T1> PromotedType; };

  // \private promote to complex type
  template <typename T1>
  struct PromotionTraits<T1,std::complex<T1> > { typedef std::complex<T1> PromotedType; };

  // a few specializations for some common cases
  template<> struct PromotionTraits<std::complex<double>,std::complex<double> >   { typedef std::complex<double> PromotedType; };
  template<> struct PromotionTraits<std::complex<double>,double>   { typedef std::complex<double> PromotedType; };
  template<> struct PromotionTraits<double,std::complex<double> >   { typedef std::complex<double> PromotedType; };
  template<> struct PromotionTraits<double,double>   { typedef double PromotedType; };

  template<> struct PromotionTraits<std::complex<float>,float>   { typedef std::complex<float> PromotedType; };
  template<> struct PromotionTraits<float,std::complex<float> >   { typedef std::complex<float> PromotedType; };
  template<> struct PromotionTraits<std::complex<float>,std::complex<float> >   { typedef std::complex<float> PromotedType; };
  template<> struct PromotionTraits<float,float>   { typedef float PromotedType; };

  template<> struct PromotionTraits<std::complex<int>,int>   { typedef std::complex<int> PromotedType; };
  template<> struct PromotionTraits<int,std::complex<int> >   { typedef std::complex<int> PromotedType; };
  template<> struct PromotionTraits<std::complex<int>,std::complex<int> >   { typedef std::complex<int> PromotedType; };
  template<> struct PromotionTraits<int,int>   { typedef int PromotedType; };
#endif // DOXYGEN

  /** @} */
} // end namespace


#endif // DUNE_PROMOTIONTRAITS_HH
