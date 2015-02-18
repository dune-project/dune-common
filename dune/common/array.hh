// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_ARRAY_HH
#define DUNE_ARRAY_HH

/** \file
    \brief Fallback implementation of the std::array class (a static array)
 */

#include <array>
#include <iostream>

namespace Dune
{
  /** @addtogroup Common

     @{
   */

  // pull in default implementation
  using std::array;

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
    array<T, 10> result = { {t0, t1, t2, t3, t4, t5, t6, t7, t8, t9} };
    return result;
  }

  //! Create an array and fill it with copies of the provided value.
  /**
   * \note This method is Dune-specific and not part of any C++ standard.
   */
  template<typename T, std::size_t n>
  array<T,n> fill_array(const T& t)
  {
    array<T,n> r;
    r.fill(t);
    return std::move(r);
  }

  /** @} */

} // end namespace Dune

#endif
