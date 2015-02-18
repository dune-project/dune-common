// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_ITERATORRANGE_HH
#define DUNE_COMMON_ITERATORRANGE_HH

namespace Dune {

  //! Simple range between a begin and an end iterator.
  /**
   * IteratorRange is mainly useful as a lightweight adaptor
   * class when adding support for range-based for loops to
   * existing containers that lack a standard begin(), end()
   * pair of member functions.
   *
   * \tparam Iterator  The type of iterator
   */
  template<typename Iterator>
  class IteratorRange
  {

  public:

    //! The iterator belonging to this range.
    typedef Iterator iterator;

    //! Constructs an iterator range on [begin,end).
    IteratorRange(const Iterator& begin, const Iterator& end)
      : _begin(begin)
      , _end(end)
    {}

    //! Returns an iterator pointing to the begin of the range.
    iterator begin() const
    {
      return _begin;
    }

    //! Returns an iterator pointing past the end of the range.
    iterator end() const
    {
      return _end;
    }

  private:

    const Iterator _begin;
    const Iterator _end;

  };

}

#endif // DUNE_COMMON_ITERATORRANGE_HH
