// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_DUMMYITERATOR_HH
#define DUNE_COMMON_DUMMYITERATOR_HH

#include <cstddef>

#include <dune/common/iteratorfacades.hh>
#include <dune/common/typetraits.hh>

template<typename T>
class dummyiterator
  : public Dune::BidirectionalIteratorFacade<dummyiterator<T>, T, T&,
        std::ptrdiff_t>
{
  friend class dummyiterator<const typename Dune::remove_const<T>::type>;

  T *value;

public:
  dummyiterator(T& value_)
    : value(&value_)
  {}

  template<typename T2>
  dummyiterator
    ( const dummyiterator<T2>& o,
    typename Dune::enable_if<Dune::Conversion<T2&, T&>::exists>::type* = 0)
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
