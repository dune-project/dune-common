// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __MISC_HH__
#define __MISC_HH__

#include <iostream>
#include <math.h>

template <bool flag>
class CompileTimeChecker { };

template <> class CompileTimeChecker<true> { };



namespace Dune {
  /** @addtogroup Common

          @{
   */

  template<class T>
  T ABS (T t)
  {
    if (t<0) return -t;
    return t;
  }

  template<class T>
  T MAX (T t1, T t2)
  {
    if (t1<t2) return t2;
    return t1;
  }

  template<class T>
  T MIN (T t1, T t2)
  {
    if (t1<t2) return t1;
    return t2;
  }

  template <class T>
  T* realloc(T* & pointer, int size) {
    if (size==0) {
      std::cerr << "Warning request for realloc with size=0\n";
      size = 1;
    }
    pointer =
      static_cast<T*>(::realloc(static_cast<void*>(pointer),
                                size*sizeof(T)));
    if (pointer == 0)
      throw std::string("Bad realloc");
    return pointer;
  }

  template <class T>
  T* malloc(int size) {
    return static_cast<T*>(::malloc(size*sizeof(T)));
  }

  /** @} */

}


#endif
