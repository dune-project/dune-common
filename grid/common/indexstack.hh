// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_INDEXSTACK_HH__
#define __DUNE_INDEXSTACK_HH__

//******************************************************
//
//  IndexStack providing indices via getIndex and freeIndex
//  indices that are freed, are put on a stack and get
//
//******************************************************

#include <dune/common/dlist.hh>
#include <dune/common/stack.hh>

namespace Dune {

  template <class T, int length>
  class IndexStack
  {
    typedef typename Dune::FiniteStack<T,length> StackType;
    typedef typename Dune::DoubleLinkedList< StackType * > StackListType;


    StackListType fullStackList_;
    StackListType emptyStackList_;

    typedef typename StackListType::Iterator DListIteratorType;
    StackType * stack_;

    // current maxIndex
    int maxIndex_;
  public:
    //! Constructor, create new IndexStack
    IndexStack() : maxIndex_ (0)
    {
      stack_ = new StackType ();
    }

    //! set index as maxIndex if index is bigger than maxIndex
    void checkAndSetMax(T index)
    {
      if(index > maxIndex_) maxIndex_ = index;
    }

    //! set index as maxIndex
    void setMaxIndex(T index)
    {
      maxIndex_ = index;
    }

    //! restore index from stack or create new index
    T getIndex ()
    {
      if(stack_->isempty())
      {
        if( fullStackList_.size() <= 0)
        {
          return maxIndex_++;
        }
        else
        {
          emptyStackList_.insert_after( emptyStackList_.rbegin(), stack_);
          DListIteratorType it = fullStackList_.begin();
          stack_ = (*it);
          fullStackList_.erase( it );
        }
      }
      return stack_->pop_front();
    }

    //! store index on stack
    void freeIndex(T index)
    {
      if(stack_->isfull())
      {
        fullStackList_.insert_before( fullStackList_.begin() , stack_ );
        if(emptyStackList_.size() <= 0)
        {
          stack_ = new StackType ();
        }
        else
        {
          DListIteratorType it = emptyStackList_.begin();
          stack_ = (*it);
          emptyStackList_.erase( it );
        }
      }
      stack_->push_front(index);
    }

    //! test stack funtcionality
    void test ()
    {
      T vec[2*length];

      for(int i=0; i<2*length; i++)
        vec[i] = getIndex();

      for(int i=0; i<2*length; i++)
        freeIndex(vec[i]);

      for(int i=0; i<2*length; i++)
        vec[i] = getIndex();

      for(int i=0; i<2*length; i++)
        printf(" index [%d] = %d \n",i,vec[i]);
    }
  }; // end class IndexStack

} // end namespace Dune

#endif
