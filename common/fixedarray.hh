// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_FIXEDARRAY_HH
#define DUNE_FIXEDARRAY_HH

/** \file
    \brief implementation of the stl array class (a static array)
    and its deprecated ancestor FixedArray
 */

#include <iostream>
#include <iomanip>
#include <string>

// Include system implementation of array class if present
#ifdef HAVE_ARRAY
#include <array>
#endif
#ifdef HAVE_TR1_ARRAY
#include <tr1/array>
#endif


namespace Dune
{
  /** @addtogroup Common

     @{
   */

#ifdef HAVE_ARRAY
  using std::array;
#elif defined HAVE_TR1_ARRAY
  using std::tr1::array;
#else

  /** \brief Simple fixed size array class
   *
   */
  template<class T, int N>
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

    //! Create uninitialized array
    array () {}

    /** \brief Return array size */
    size_type size() const {return N;}

    //! Assign value to all entries
    array<T,N>& operator= (const T& t)
    {
      for (int i=0; i<N; i++) a[i]=t;
      return (*this);
    }

    //! \brief Assign value to all entries
    void assign(const T& t)
    {
      for (int i=0; i<N; i++) a[i]=t;
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

  protected:
    T a[(N > 0) ? N : 1];
  };
#endif
  //! Output operator for array
  template <class T, int N>
  inline std::ostream& operator<< (std::ostream& s, array<T,N> e)
  {
    s << "[";
    for (int i=0; i<N-1; i++) s << e[i] << ",";
    s << e[N-1] << "]";
    return s;
  }

  /** @} */

} // end namespace Dune

#endif
