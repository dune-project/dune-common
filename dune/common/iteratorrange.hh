// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
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
   * \ingroup CxxUtilities
   */
  template<typename Iterator>
  class IteratorRange
  {

  public:

    //! The iterator belonging to this range.
    typedef Iterator iterator;

    //! The iterator belonging to this range.
    /**
     * This typedef is here mainly for compatibility reasons.
     */
    typedef Iterator const_iterator;

    //! Constructs an iterator range on [begin,end).
    IteratorRange(const Iterator& begin, const Iterator& end)
      : _begin(begin)
      , _end(end)
    {}

    //! Default constructor, relies on iterators being default-constructible.
    IteratorRange()
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

    Iterator _begin;
    Iterator _end;

  };

}

#endif // DUNE_COMMON_ITERATORRANGE_HH
