// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_NULLPTR_HH
#define DUNE_NULLPTR_HH

/** \file
 * \brief Fallback implementation of the nullptr object in C++0x
 */

#if ! HAVE_NULLPTR

/**
   \brief Fallback implementation of nullptr

   see C++ proposal
   http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2431.pdf
 */
const                        // this is a const object...
class dune_nullptr_t {            // of type nullptr_t
public:
  template<class T>          // convertible to any type
  operator T*() const        // of null non-member
  { return 0; }              // pointer...
  template<class C, class T> // or any type of null
  operator T C::*() const    // member pointer...
  { return 0; }
private:
  void operator&() const;    // whose address can't be taken
} nullptr = {};              // and whose name is nullptr

namespace Dune {
  typedef dune_nullptr_t nullptr_t;
}

template<class T>
bool operator == (T* t, dune_nullptr_t)
{
  return (t == static_cast<T*>(nullptr));
}

template<class T>
bool operator == (dune_nullptr_t, T* t)
{
  return (t == static_cast<T*>(nullptr));
}

#else

#include <cstddef>

namespace Dune {
  using std::nullptr_t;
}

#endif // HAVE_NULLPTR

#endif // DUNE_NULLPTR_HH
