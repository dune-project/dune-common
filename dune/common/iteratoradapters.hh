// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_ITERATORADAPTERS_HH
#define DUNE_COMMON_ITERATORADAPTERS_HH

#include <iterator>

#include <dune/common/typetraits.hh>
#include <dune/common/iteratorfacades.hh>

namespace Dune {

  template<typename It, int stride_>
  class StridedIterator
    : public RandomAccessIteratorFacade<StridedIterator<It,stride_>,
                                        typename std::iterator_traits<It>::value_type,
                                        typename std::iterator_traits<It>::reference,
                                        typename std::iterator_traits<It>::difference_type
                                        >
  {

    static_assert(
      (is_base_of<std::random_access_iterator_tag,typename std::iterator_traits<It>::iterator_category>::value),
      "StridedIterator can only wrap random access iterators."
      );

    typedef RandomAccessIteratorFacade<
      StridedIterator,
      typename std::iterator_traits<It>::value_type,
      typename std::iterator_traits<It>::reference,
      typename std::iterator_traits<It>::difference_type
      > Base;

    friend class RandomAccessIteratorFacade<
      StridedIterator,
      typename std::iterator_traits<It>::value_type,
      typename std::iterator_traits<It>::reference,
      typename std::iterator_traits<It>::difference_type
      >;

  public:

    typedef typename std::iterator_traits<It>::value_type value_type;
    typedef typename std::iterator_traits<It>::pointer pointer;
    typedef typename std::iterator_traits<It>::reference reference;
    typedef typename std::iterator_traits<It>::difference_type difference_type;

    static const int stride = stride_;

    explicit StridedIterator(const It& it)
      : _it(it)
    {}

  private:
    // keep internal methods public for now, as access from the facade is a horrible mess!
  public:

    reference dereference() const
    {
      return *_it;
    }

    reference elementAt(difference_type n) const
    {
      return _it[n * stride];
    }

    bool equals(const StridedIterator& other) const
    {
      return _it == other._it;
    }

    void increment()
    {
      _it += stride;
    }

    void decrement()
    {
      _it -= stride;
    }

    void advance(difference_type n)
    {
      _it += n * stride;
    }

    difference_type distanceTo(const StridedIterator& other) const
    {
      return (other._it - _it) / stride;
    }

    It _it;

  };

} // namespace Dune

#endif // DUNE_ITERATORFACADES_HH
