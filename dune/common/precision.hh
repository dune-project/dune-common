// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_PRECISION_HH
#define DUNE_PRECISION_HH

/** \file
 * \brief Various precision settings for calculations with FieldMatrix and FieldVector
 */

#include <dune/common/typetraits.hh>

namespace Dune {

  /**
      @addtogroup DenseMatVec
      @{
   */

  /**
   * @brief Precisions for calculations with FieldMatrix and FieldVector.
   */
  template <class ctype = double>
  class FMatrixPrecision {
  public:
    typedef typename FieldTraits<ctype>::real_type real_type;

    //! return threshold to declare matrix singular
    static real_type singular_limit ()
    {
      return _singular;
    }

    //! set singular threshold
    static void set_singular_limit (real_type singthres)
    {
      _singular = singthres;
    }

    //! return threshold to declare matrix singular
    static real_type absolute_limit ()
    {
      return _absolute;
    }

    //! set singular threshold
    static void set_absolute_limit (real_type absthres)
    {
      _absolute = absthres;
    }

  private:
    // just to demonstrate some state information
    static real_type _singular;
    static real_type _absolute;
  };

  template <class ctype>
  typename FMatrixPrecision<ctype>::real_type FMatrixPrecision<ctype>::_singular = 1E-14;
  template <class ctype>
  typename FMatrixPrecision<ctype>::real_type FMatrixPrecision<ctype>::_absolute = 1E-80;

  /** @} end documentation */

} // end namespace

#endif
