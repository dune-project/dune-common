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

#include <dune/common/deprecated.hh>

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

    /** \brief Initialize all components with same entry
        \deprecated Deprecated because the stl implementation of array doesn't have it
     */
    array (const T& t) DUNE_DEPRECATED
    {
      for (int i=0; i<N; i++) a[i]=t;
    }

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

  /** \brief Simple fixed size array class
   *  \deprecated Replaced by array
   */
  template<class T, int N>
  class FixedArray {
  public:

    //! Remember the storage type
    typedef T MemberType;

    //! The actual number of elements that gets allocated.
    //! It's always at least 1.
    enum { n = (N > 0) ? N : 1 };

    //! Know your own length
    enum { dimension = N };

    //! Create uninitialized array
    FixedArray () {}

    //! Initialize all components with same size
    FixedArray (T t) DUNE_DEPRECATED
    {
      for (int i=0; i<N; i++) a[i]=t;
    }

    /** \brief Return array size */
    int size() const {return N;}

    /** \brief Assign value to all entries
     * @deprecated Use assign instead.
     */
    FixedArray<T,N>& operator= (const T& t) DUNE_DEPRECATED
    {
      assign(t);
      return (*this);
    }

    //! \brief Assign value to all entries
    void assign(const T& t)
    {
      for (int i=0; i<N; i++) a[i]=t;
    }

    //! Component access
    T& operator[] (int i)
    {
      return a[i];
    }

    //! Const component access
    const T& operator[] (int i) const
    {
      return a[i];
    }

    //! \todo Please doc me!
    FixedArray<T,N-1> shrink (int comp)
    {
      FixedArray<T,N-1> x;
      for (int i=0; i<comp; i++) x[i] = a[i];
      for (int i=comp+1; i<N; i++) x[i-1] = a[i];
      return x;
    }

    //! \todo Please doc me!
    FixedArray<T,N+1> expand (int comp, T value)
    {
      FixedArray<T,N+1> x;
      for (int i=0; i<comp; i++) x[i] = a[i];
      x[comp] = value;
      for (int i=comp+1; i<N+1; i++) x[i] = a[i-1];
      return x;
    }

  protected:
    T a[n];
  } DUNE_DEPRECATED;

  //! Output operator for FixedArray
  template <class T, int N>
  inline std::ostream& operator<< (std::ostream& s, FixedArray<T,N> e)
  {
    s << "[";
    for (int i=0; i<N-1; i++) s << e[i] << ",";
    s << e[N-1] << "]";
    return s;
  }

  /** @} */

} // end namespace Dune

#endif
