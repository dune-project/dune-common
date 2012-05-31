// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_FINITE_STACK_HH
#define DUNE_FINITE_STACK_HH

/** \file
 * \brief Stack class of fixed maximum size (deprecated)
 */

#warning This file is deprecated and will be removed after the release of dune-common-2.2.\
  Please use std::stack<Dune::ReservedVector> instead of FiniteStack.

#include <stack>

#include <dune/common/exceptions.hh>
#include <dune/common/reservedvector.hh>

namespace Dune {

  /*! \addtogroup Common
     @{
   */

  /*! \file

     This file implements a stack classes FiniteStack. It is
     mainly used by the grid iterators where exact knowledge of the stack
     implementation is needed to guarantee efficient execution.
   */

  /** \brief A stack with static memory allocation
   *
     This class implements a very efficient stack where the maximum
     depth is known in advance. Note that no error checking is
     performed!

     \param n Maximum number of stack entries
   */
  template<class T, int n>
  class FiniteStack
    : public std::stack<T, Dune::ReservedVector<T,n> >
  {
  public:

    //! Returns true if the stack is full
    bool full () const
    {
      return this->size()>=n;
    }

    /** Removes and returns the uppermost object from the stack
       \warning This differs from the semantics of std::stack, where pop() returns void
     */
    T pop ()
    {
#ifndef NDEBUG
      if (this->empty())
        DUNE_THROW(Dune::RangeError, "trying to call pop() on an empty FiniteStack");
#endif
      T tmp = this->top();
      this->std::stack<T,Dune::ReservedVector<T,n> >::pop();
      return tmp;
    }

  };

}

//! }@

#endif
