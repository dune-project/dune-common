// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_LOCALFUNCTIONARRAY_HH__
#define __DUNE_LOCALFUNCTIONARRAY_HH__

#include <vector>
#include "../common/array.hh"

#include "basefunctions.hh"
#include "fastbase.hh"
#include "localfunction.hh"
#include "dofiterator.hh"

namespace Dune {

  //**************************************************************************
  //
  //  --LocalFunctionArray
  //
  //! Implementation of the local functions
  //
  //**************************************************************************
  template < class DiscreteFunctionSpaceType >
  class LocalFunctionArray
    : public LocalFunctionDefault <DiscreteFunctionSpaceType ,
          LocalFunctionArray < DiscreteFunctionSpaceType >  >
  {
    typedef FastBaseFunctionSet < DiscreteFunctionSpaceType > BaseFunctionSetType;
    typedef LocalFunctionArray < DiscreteFunctionSpaceType > MyType;

    typedef typename MemoryProvider <MyType>::ObjectEntity ObjectEntityType;

  public:
    //! Constructor
    LocalFunctionArray ( const DiscreteFunctionSpaceType &f , std::vector < Array < RangeField > > & dofVec )
      : fSpace_ ( f ), dofVec_ ( dofVec )  , next_ (NULL)
    {
      built_ = false;
      baseFuncSet_ = NULL;
    };

    ~LocalFunctionArray ()
    {
      if(next_) delete next_;
    }

    //! access to dof number num, all dofs of the dof entity
    RangeField & operator [] (int num)
    {
      //if (!built_) std::cout << "Warning: LocalFunction not built!\n";
      return (*dofArray_)[ map_ [ num ] ];
    }

    int numberOfDofs ()
    {
      //if (!built_) std::cout << "Warning: LocalFunction not built!\n";
      return numOfDof_;
    };

    //! sum over all local base functions
    template <class EntityType>
    void evaluate (EntityType &en, const Domain & x, Range & ret)
    {
      ret = 0.0;
      for(int i=0; i<numOfDof_; i++)
      {
        baseFuncSet_->evaluate(i,diffVar,x,tmp);
        ret += this->operator [] (i) * tmp;
      }
    }

    //! get pointer to next LocalFunction
    MyType * getNext() const
    {
      return next_;
    }

    //! set pointer to next LocalFunction
    void setNext (MyType * n)
    {
      next_ = n;
    }

    //! methods that do not belong to the interface but have to be public
    //! used like setElInfo and so on
    template <class EntityType >
    void init ( EntityType &en)
    {
      built_  = false;

      dofArray_ = & dofVec_ [ en.level() ];
      baseFuncSet_ = & ( fSpace_.getBaseFunctionSet(en) );
      numOfDof_ = baseFuncSet_->getNumberOfBaseFunctions();

      if(numOfDof_ > map_.size())
        map_.resize( numOfDof_ );

      for(int i=0; i<numOfDof_; i++)
        map_ [i] = fSpace_.mapToGlobal ( en , i);

      built_ = true;
    };

    // print local function
    void print()
    {
      for(int i=0; i<numOfDof_; i ++)
        std::cout << this->operator [] (i) << " Dof "<< i << "\n";
    }

  protected:
    //! needed once
    Range tmp;

    //! remember pointer to next LocalFunction
    MyType * next_;

    //! diffVar for evaluate, is empty
    const DiffVariable<0>::Type diffVar;

    //! true if setup is done
    bool built_;

    //! number of dofs
    int numOfDof_;

    //! the corresponding function space which provides the base function set
    const DiscreteFunctionSpaceType &fSpace_;

    //! dofArray of current level from the discrete function
    Array < RangeField > * dofArray_;

    //! dofVec from all levels of the discrete function
    std::vector < Array < RangeField > > & dofVec_;

    //! lookup table for local to global mapping
    Array < int > map_;

    //! the corresponding base function set
    BaseFunctionSetType *baseFuncSet_;
  }; // end LocalFunctionArray


  //***********************************************************************
  //
  //  --DofIteratorArray
  //
  //***********************************************************************
  template < class DofType >
  class DofIteratorArray : public
                           DofIteratorDefault < DofType , DofIteratorArray < DofType > >
  {
  public:
    DofIteratorArray ( Array < DofType > & dofArray , int count )
      :  dofArray_ ( dofArray ) , count_ ( count ) {};

    DofType & operator *()
    {
      return dofArray_ [ count_ ];
    };

    DofIteratorType & operator++ ()
    {
      count_++;
      return (*this);
    };

    DofIteratorType & operator++ (int i)
    {
      count_ += i;
      return (*this);
    };

    DofType& operator[] (int i) {
      return dofArray_[i];
    }

    bool operator == (const DofIteratorType & I )
    {
      return count_  == I.count_;
    }

    bool operator != (const DofIteratorType & I )
    {
      return count_  != I.count_;
    }

    int index () { return count_; }

    void reset () { count_ = 0; };

  private:
    //! index
    int count_;

    //! the array holding the dofs
    Array < DofType > &dofArray_;

  };

} // end namespace Dune

#endif
