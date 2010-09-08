// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id: fvector.hh 6105 2010-08-25 16:06:36Z christi $
#ifndef DUNE_FVECTOR_HH
#define DUNE_FVECTOR_HH

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <complex>
#include <cstring>
#include <limits>

#include "exceptions.hh"
#include "genericiterator.hh"

#include <vector>
#include "densevector.hh"

namespace Dune {

  /** @addtogroup DenseMatVec
      @{
   */

  template< class K >
  class DynamicVector : public DenseVector< std::vector<K> >
  {
  public:
    typedef typename DenseVector< std::vector<K> >::size_type size_type;

    //! Constructor making uninitialized vector
    DynamicVector() {}

    //! Constructor making vector with identical coordinates
    DynamicVector (size_type n, const K& t) :
      DenseVector< std::vector<K> > (n,t)
    {}

    //! Constructor making vector with identical coordinates
    DynamicVector (const DenseVector< std::vector<K> > & x) :
      DenseVector< std::vector<K> > (x)
    {}

    using DenseVector< std::vector<K> >::operator=;
  };

  /** \brief Read a DynamicVector from an input stream
   *  \relates DynamicVector
   *
   *  \note This operator is STL compilant, i.e., the content of v is only
   *        changed if the read operation is successful.
   *
   *  \param[in]  in  std :: istream to read from
   *  \param[out] v   DynamicVector to be read
   *
   *  \returns the input stream (in)
   */
  template<class K>
  inline std::istream &operator>> ( std::istream &in,
                                    DynamicVector<K> &v )
  {
    DynamicVector<K> w(v);
    for( typename DynamicVector<K>::size_type i = 0; i < w.size(); ++i )
      in >> w[ i ];
    if(in)
      v = w;
    return in;
  }

  /** @} end documentation */

} // end namespace

#endif
