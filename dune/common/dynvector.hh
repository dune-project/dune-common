// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id: fvector.hh 6105 2010-08-25 16:06:36Z christi $
#ifndef DUNE_DYNVECTOR_HH
#define DUNE_DYNVECTOR_HH

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

  /*! \file
   * \brief This file implements a dense vector with a dynamic size.
   */

  template< class K > class DynamicVector;
  template< class K >
  struct DenseMatVecTraits< DynamicVector<K> >
  {
    typedef DynamicVector<K> derived_type;
    typedef std::vector<K> container_type;
    typedef K value_type;
    typedef typename container_type::size_type size_type;
  };

  template< class K >
  struct FieldTraits< DynamicVector<K> >
  {
    typedef typename FieldTraits<K>::field_type field_type;
    typedef typename FieldTraits<K>::real_type real_type;
  };

  /** \brief Construct a vector with a dynamic size.
   *
   * \tparam K is the field type (use float, double, complex, etc)
   */
  template< class K >
  class DynamicVector : public DenseVector< DynamicVector<K> >
  {
    std::vector<K> _data;

    typedef DenseVector< DynamicVector<K> > Base;
  public:
    typedef typename Base::size_type size_type;
    typedef typename Base::value_type value_type;

    //! Constructor making uninitialized vector
    DynamicVector() {}

    //! Constructor making vector with identical coordinates
    explicit DynamicVector (size_type n, value_type c = value_type() ) :
      _data(n,c)
    {}

    //! Constructor making vector with identical coordinates
    DynamicVector (const DynamicVector & x) :
      _data(x._data)
    {}

    using Base::operator=;

    //==== forward some methods of std::vector
    /** \brief Number of elements for which memory has been allocated.

        capacity() is always greater than or equal to size().
     */
    size_type capacity() const
    {
      return _data.capacity();
    }
    void resize (size_type n, value_type c = value_type() )
    {
      _data.resize(n,c);
    }
    void reserve (size_type n)
    {
      _data.reserve(n);
    }

    //==== make this thing a vector
    size_type vec_size() const { return _data.size(); }
    K & vec_access(size_type i) { return _data[i]; }
    const K & vec_access(size_type i) const { return _data[i]; }
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
