// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_FIXEDARRAY_HH
#define DUNE_FIXEDARRAY_HH


//***********************************************************************
//
//  implementation of peter array
//
//***********************************************************************

#include <iostream>
#include <iomanip>
#include <string>

namespace Dune
{
  /** @addtogroup Common

     @{
   */

  /** \brief Simple fixed size array class
   *
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
    FixedArray (T t)
    {
      for (int i=0; i<N; i++) a[i]=t;
    }

    //! Assign value to all entries
    FixedArray<T,N>& operator= (const T& t)
    {
      for (int i=0; i<N; i++) a[i]=t;
      return (*this);
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
  };

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
