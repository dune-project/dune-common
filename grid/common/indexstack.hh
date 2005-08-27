// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_INDEXSTACK_HH
#define DUNE_INDEXSTACK_HH

#include <assert.h>
#include <stack>

#include <dune/common/stack.hh>

namespace Dune {

  //!  IndexStack providing indices via getIndex and freeIndex
  //!  indices that are freed, are put on a stack and get
  template <class T, int length>
  class IndexStack
  {
    typedef typename Dune::FiniteStack<T,length> StackType;
    typedef typename std::stack < StackType * > StackListType;

    StackListType fullStackList_;
    StackListType emptyStackList_;

    //typedef typename StackListType::Iterator DListIteratorType;
    StackType * stack_;

    // current maxIndex
    int maxIndex_;
  public:
    //! Constructor, create new IndexStack
    inline IndexStack();

    //! Destructor, deleting all stacks
    inline ~IndexStack ();

    //! set index as maxIndex if index is bigger than maxIndex
    inline void checkAndSetMax(T index) { if(index > maxIndex_) maxIndex_ = index;}

    //! set index as maxIndex
    inline void setMaxIndex(T index) { maxIndex_ = index; }

    //! return maxIndex which is also the
    inline int getMaxIndex() const { return maxIndex_;  }

    //! return maxIndex which is also the
    inline int size() const { return getMaxIndex(); }

    //! restore index from stack or create new index
    inline T getIndex ();

    //! store index on stack
    inline void freeIndex(T index);

    //! test stack funtcionality
    inline void test ();

    // backup set to out stream
    inline void backupIndexSet ( std::ostream & os );

    // restore from in stream
    inline void restoreIndexSet ( std::istream & is );
  private:
    // no copy constructor allowed
    IndexStack( const IndexStack<T,length> & s) : maxIndex_ (0) , stack_(0) {}

    // no assignment operator allowed
    IndexStack<T,length> & operator = ( const IndexStack<T,length> & s)
    {
      std::cerr << "IndexStack::operator = () not allowed! in: " __FILE__ << " line:" << __LINE__ << "\n";
      abort();
      return *this;
    }

    // clear fullStacks
    void clearStack ();

  }; // end class IndexStack

  //****************************************************************
  // Inline implementation
  // ***************************************************************
  template <class T, int length>
  inline IndexStack<T,length>::IndexStack()
    : stack_ ( new StackType () ) , maxIndex_ (0) {}

  template <class T, int length>
  inline IndexStack<T,length>::~IndexStack ()
  {
    if(stack_) delete stack_;
    stack_ = 0;

    while( !fullStackList_.empty() )
    {
      StackType * st = fullStackList_.top();
      if(st) delete st;
      fullStackList_.pop();
    }
    while( !emptyStackList_.empty() )
    {
      StackType * st = emptyStackList_.top();
      if(st) delete st;
      emptyStackList_.pop();
    }
  }

  template <class T, int length>
  inline T IndexStack<T,length>::getIndex ()
  {
    if((*stack_).empty())
    {
      if( fullStackList_.size() <= 0)
      {
        return maxIndex_++;
      }
      else
      {
        emptyStackList_.push( stack_ );
        stack_ = fullStackList_.top();
        fullStackList_.pop();
      }
    }
    return (*stack_).pop();
  }

  template <class T, int length>
  inline void IndexStack<T,length>::freeIndex ( T index )
  {
    if((*stack_).full())
    {
      fullStackList_.push(  stack_ );
      if(emptyStackList_.size() <= 0)
      {
        stack_ = new StackType ();
      }
      else
      {
        stack_ = emptyStackList_.top();
        emptyStackList_.pop();
      }
    }
    (*stack_).push(index);
  }

  template <class T, int length>
  inline void IndexStack<T,length>::test ()
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

  template <class T, int length>
  inline void IndexStack<T,length>::backupIndexSet ( std::ostream & os )
  {
    // holes are not stored at the moment
    os.write( ((const char *) &maxIndex_ ), sizeof(int) ) ;
    return ;
  }

  template <class T, int length>
  inline void IndexStack<T,length>::restoreIndexSet ( std::istream & is )
  {
    is.read ( ((char *) &maxIndex_), sizeof(int) );
    clearStack ();

    return ;
  }

  template <class T, int length>
  inline void IndexStack<T,length>::clearStack ()
  {
    if(stack_)
    {
      delete stack_;
      stack_ = new StackType();
      assert(stack_);
    }

    while( !fullStackList_.empty() )
    {
      StackType * st = fullStackList_.top();
      if(st) delete st;
      fullStackList_.pop();
    }
    return;
  }

} // end namespace Dune

#endif
