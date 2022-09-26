// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_DYNVECTOR_HH
#define DUNE_DYNVECTOR_HH

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <complex>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <utility>

#include "boundschecking.hh"
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

  template< class K, class Allocator > class DynamicVector;
  template< class K, class Allocator >
  struct DenseMatVecTraits< DynamicVector< K, Allocator > >
  {
    typedef DynamicVector< K, Allocator > derived_type;
    typedef std::vector< K, Allocator > container_type;
    typedef K value_type;
    typedef typename container_type::size_type size_type;
  };

  template< class K, class Allocator >
  struct FieldTraits< DynamicVector< K, Allocator > >
  {
    typedef typename FieldTraits< K >::field_type field_type;
    typedef typename FieldTraits< K >::real_type real_type;
  };

  /** \brief Construct a vector with a dynamic size.
   *
   * \tparam K is the field type (use float, double, complex, etc)
   * \tparam Allocator type of allocator object used to define the storage allocation model,
   *                default Allocator = std::allocator< K >.
   */
  template< class K, class Allocator = std::allocator< K > >
  class DynamicVector : public DenseVector< DynamicVector< K, Allocator > >
  {
    std::vector< K, Allocator > _data;

    typedef DenseVector< DynamicVector< K, Allocator > > Base;
  public:
    typedef typename Base::size_type size_type;
    typedef typename Base::value_type value_type;

    typedef std::vector< K, Allocator > container_type;

    typedef Allocator allocator_type;

    //! Constructor making uninitialized vector
    explicit DynamicVector(const allocator_type &a = allocator_type() ) :
      _data( a )
    {}

    explicit DynamicVector(size_type n, const allocator_type &a = allocator_type() ) :
      _data( n, value_type(), a )
    {}

    //! Constructor making vector with identical coordinates
    DynamicVector( size_type n, value_type c, const allocator_type &a = allocator_type() ) :
      _data( n, c, a )
    {}

    /** \brief Construct from a std::initializer_list */
    DynamicVector (std::initializer_list<K> const &l) :
      _data(l)
    {}

    //! Constructor making vector with identical coordinates
    DynamicVector(const DynamicVector & x) :
      Base(), _data(x._data)
    {}

    //! Move constructor
    DynamicVector(DynamicVector && x) :
      _data(std::move(x._data))
    {}

    template< class T >
    DynamicVector(const DynamicVector< T, Allocator > & x) :
      _data(x.begin(), x.end(), x.get_allocator())
    {}

    //! Copy constructor from another DenseVector
    template< class X >
    DynamicVector(const DenseVector< X > & x, const allocator_type &a = allocator_type() ) :
      _data(a)
    {
      const size_type n = x.size();
      _data.reserve(n);
      for( size_type i =0; i<n ;++i)
        _data.push_back( x[ i ] );
    }

    using Base::operator=;

    //! Copy assignment operator
    DynamicVector &operator=(const DynamicVector &other)
    {
      _data = other._data;
      return *this;
    }

    //! Move assignment operator
    DynamicVector &operator=(DynamicVector &&other)
    {
      _data = std::move(other._data);
      return *this;
    }

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
    size_type size() const { return _data.size(); }
    K & operator[](size_type i) {
      DUNE_ASSERT_BOUNDS(i < size());
      return _data[i];
    }
    const K & operator[](size_type i) const {
      DUNE_ASSERT_BOUNDS(i < size());
      return _data[i];
    }

    //! return pointer to underlying array
    K* data() noexcept
    {
      return _data.data();
    }

    //! return pointer to underlying array
    const K* data() const noexcept
    {
      return _data.data();
    }

    const container_type &container () const { return _data; }
    container_type &container () { return _data; }
  };

  /** \brief Read a DynamicVector from an input stream
   *  \relates DynamicVector
   *
   *  \note This operator is STL compliant, i.e., the content of v is only
   *        changed if the read operation is successful.
   *
   *  \param[in]  in  std :: istream to read from
   *  \param[out] v   DynamicVector to be read
   *
   *  \returns the input stream (in)
   */
  template< class K, class Allocator >
  inline std::istream &operator>> ( std::istream &in,
                                    DynamicVector< K, Allocator > &v )
  {
    DynamicVector< K, Allocator > w(v);
    for( typename DynamicVector< K, Allocator >::size_type i = 0; i < w.size(); ++i )
      in >> w[ i ];
    if(in)
      v = std::move(w);
    return in;
  }

  /** @} end documentation */

} // end namespace

#endif
