// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_ARRAY_HH
#define DUNE_ARRAY_HH

/** \file
    \brief Fallback implementation of the std::array class (a static array)
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
      fill(t);
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
    if (N == 0)
    {
      s << "[]";
      return s;
    }

    s << "[";
    for (size_t i=0; i<N-1; i++) s << e[i] << ",";
    s << e[N-1] << "]";
    return s;
  }

#ifndef DOXYGEN
  template<class T>
  array<T, 1> make_array(const T &t0) {
    array<T, 1> result = { {t0} };
    return result;
  }

  template<class T>
  array<T, 2> make_array(const T &t0, const T &t1) {
    array<T, 2> result = { {t0, t1} };
    return result;
  }

  template<class T>
  array<T, 3> make_array(const T &t0, const T &t1, const T &t2) {
    array<T, 3> result = { {t0, t1, t2} };
    return result;
  }

  template<class T>
  array<T, 4> make_array(const T &t0, const T &t1, const T &t2, const T &t3) {
    array<T, 4> result = { {t0, t1, t2, t3} };
    return result;
  }

  template<class T>
  array<T, 5> make_array(const T &t0, const T &t1, const T &t2, const T &t3,
                         const T &t4)
  {
    array<T, 5> result = { {t0, t1, t2, t3, t4} };
    return result;
  }

  template<class T>
  array<T, 6> make_array(const T &t0, const T &t1, const T &t2, const T &t3,
                         const T &t4, const T &t5)
  {
    array<T, 6> result = { {t0, t1, t2, t3, t4, t5} };
    return result;
  }

  template<class T>
  array<T, 7> make_array(const T &t0, const T &t1, const T &t2, const T &t3,
                         const T &t4, const T &t5, const T &t6)
  {
    array<T, 7> result = { {t0, t1, t2, t3, t4, t5, t6} };
    return result;
  }

  template<class T>
  array<T, 8> make_array(const T &t0, const T &t1, const T &t2, const T &t3,
                         const T &t4, const T &t5, const T &t6, const T &t7)
  {
    array<T, 8> result = { {t0, t1, t2, t3, t4, t5, t6, t7} };
    return result;
  }

  template<class T>
  array<T, 9> make_array(const T &t0, const T &t1, const T &t2, const T &t3,
                         const T &t4, const T &t5, const T &t6, const T &t7,
                         const T &t8)
  {
    array<T, 9> result = { {t0, t1, t2, t3, t4, t5, t6, t7, t8} };
    return result;
  }
#endif // !DOXYGEN

  //! create an initialize an array
  /**
   * \note There are overloads for this method which take fewer arguments
   *       (minimum 1).  The current maximum of 10 arguments is arbitrary and
   *       can be raised on demand.
   * \note This method is Dune-specific and not part of any C++-standard.
   */
  template<class T>
  array<T, 10> make_array(const T &t0, const T &t1, const T &t2, const T &t3,
                          const T &t4, const T &t5, const T &t6, const T &t7,
                          const T &t8, const T &t9)
  {
    array<T, 10> result = { t0, t1, t2, t3, t4, t5, t6, t7, t8, t9 };
    return result;
  }

  /** @} */

} // end namespace Dune

#endif
