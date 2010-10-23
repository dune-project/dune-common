// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ARRAY_HH
#define DUNE_ARRAY_HH

/** \file
    \brief implementation of the stl array class (a static array)
 */

#include <iostream>
#include <iomanip>
#include <string>

// Include system implementation of array class if present
#ifdef HAVE_ARRAY
#include <array>
#else
#include <algorithm>
#endif

#include "deprecated.hh"

namespace Dune
{
  /** @addtogroup Common

     @{
   */

#ifdef HAVE_ARRAY
  using std::array;
#else

  /** \brief Simple fixed size array class.  This replaces std::array,
   * if that is not available.
   *
   */
  template<class T, size_t N>
  class array {
  public:

    //! Remember the storage type
    typedef T value_type;

    /** \brief Reference to an object */
    typedef value_type&                             reference;

    /** \brief Const reference to an object */
    typedef const value_type&                       const_reference;

    /** \brief Iterator type */
    typedef value_type*                             iterator;

    /** \brief Const iterator type */
    typedef const value_type*                       const_iterator;

    /** \brief Type used for array indices */
    typedef std::size_t size_type;

    /** \brief Difference type */
    typedef std::ptrdiff_t difference_type;

    /** \brief Reverse iterator type */
    typedef std::reverse_iterator<iterator>         reverse_iterator;

    /** \brief Const reverse iterator type */
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;

    /** \brief Return array size */
    size_type size() const {return N;}

    //! Assign value to all entries
    array<T,N>& operator= (const T& t)
    {
      for (size_type i=0; i<N; i++) a[i]=t;
      return (*this);
    }

    //! \brief Assign value to all entries (according to C++0x the fill method is to be prefered)
    void assign(const T& t) DUNE_DEPRECATED
    {
      fill(t);
    }

    //! \brief Assign value to all entries (according to C++0x the fill method is to be prefered)
    void fill(const T& t)
    {
      for (size_type i=0; i<N; i++) a[i]=t;
    }

    //! Component access
    reference operator[] (size_type i)
    {
      return a[i];
    }

    //! Const component access
    const_reference operator[] (size_type i) const
    {
      return a[i];
    }

    iterator begin ()
    {
      return a;
    }

    const_iterator begin () const
    {
      return a;
    }

    iterator end ()
    {
      return a + N;
    }

    const_iterator end () const
    {
      return a + N;
    }

    T a[(N > 0) ? N : 1];
  };



  // Comparison Operators (see [lib.container.requirements])
  // -------------------------------------------------------

  template< class T, size_t N >
  inline bool operator< ( const array< T, N > &a, const array< T, N > &b )
  {
    return std::lexicographical_compare( a.begin(), a.end(), b.begin(), b.end() );
  }

  template< class T, size_t N >
  inline bool operator> ( const array< T, N > &a, const array< T, N > &b )
  {
    return b < a;
  }

  template< class T, size_t N >
  inline bool operator<= ( const array< T, N > &a, const array< T, N > &b )
  {
    return !(a > b);
  }

  template< class T, size_t N >
  inline bool operator>= ( const array< T, N > &a, const array< T, N > &b )
  {
    return !(a < b);
  }
#endif

  //! Output operator for array
  template < class T, size_t N >
  inline std::ostream& operator<< (std::ostream& s, const array<T,N>& e)
  {
    s << "[";
    for (size_t i=0; i<N-1; i++) s << e[i] << ",";
    s << e[N-1] << "]";
    return s;
  }

  /** @} */

} // end namespace Dune

#endif
