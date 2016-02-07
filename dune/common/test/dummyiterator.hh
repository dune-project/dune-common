// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_DUMMYITERATOR_HH
#define DUNE_COMMON_DUMMYITERATOR_HH

#include <cstddef>
#include <type_traits>

#include <dune/common/iteratorfacades.hh>

template<typename T>
class dummyiterator
  : public Dune::BidirectionalIteratorFacade<dummyiterator<T>, T, T&,
        std::ptrdiff_t>
{
  friend class dummyiterator<const typename std::remove_const<T>::type>;

  T *value;

public:
  dummyiterator(T& value_)
    : value(&value_)
  {}

  template<typename T2>
  dummyiterator
    ( const dummyiterator<T2>& o,
    typename std::enable_if<std::is_convertible<T2&, T&>::value>::type* = 0)
    : value(o.value)
  {}

  T& derefence() const {
    return *value;
  }

  bool equals(const dummyiterator& o) const {
    return value == o.value;
  }

  void increment() {}
  void decrement() {}
};

#endif // DUNE_COMMON_DUMMYITERATOR_HH
