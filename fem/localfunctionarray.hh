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
  public:
    //! Constructor
    LocalFunctionArray ( const DiscreteFunctionSpaceType &f , std::vector < Array < RangeField > > & dofVec )
      : fSpace_ ( f ), dofVec_ ( dofVec )
    {
      built_ = false;
      baseFuncSet_ = NULL;
    };

    //! access to dof number num, all dofs of the dof entity
    RangeField & operator [] (int num)
    {
      if (!built_) std::cout << "Warning: LocalFunction not built!\n";
      return (*dofArray_)[ map_ [ num ] ];
    }

    int numberOfDofs ()
    {
      if (!built_) std::cout << "Warning: LocalFunction nout built!\n";
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

    // methods that do not belong to the interface but have to be public
    // used like setElInfo and so on
    template <class EntityType >
    void init ( EntityType &en )
    {
      built_  = false;
      //std::cout << "Init LocalFunctionArray! \n";
      dofArray_ = & dofVec_ [ en.level() ];
      baseFuncSet_ = & ( fSpace_.getBaseFunctionSet(en) );
      numOfDof_ = baseFuncSet_->getNumberOfBaseFunctions();
      if(numOfDof_ > map_.size())
        map_.resize( numOfDof_ );
      for(int i=0; i<numOfDof_; i++)
        map_ [i] = fSpace_.mapToGlobal ( en , i);
      built_ = true;
    };

  protected:
    //! needed once
    Range tmp;

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


  //**************************************************************************
  //
  //  --LocalFunctionArrayIterator
  //
  //! Iterator to navigate through the local functions
  //! The Storage of the dofs is implemented via an array
  //
  //**************************************************************************
  template < int codim , class DiscFunctionSpaceType >
  class LocalFunctionArrayIterator
    : public LocalFunctionIteratorDefault < DiscFunctionSpaceType ,
          LocalFunctionArrayIterator < codim , DiscFunctionSpaceType > ,
          LocalFunctionArray < DiscFunctionSpaceType >
          >

  {
    typedef typename DiscFunctionSpaceType::GridType GridType;
    //! we use the Grid LevelIterator
    typedef typename GridType::Traits<codim>::LevelIterator LevelIterator;
    typedef typename DiscFunctionSpaceType::RangeField RangeField;

    // just for readability
    typedef LocalFunctionArrayIterator < codim , DiscFunctionSpaceType >
    LocalFunctionArrayIteratorType;
  public:

    //! Constructor
    LocalFunctionArrayIterator ( const DiscFunctionSpaceType &f ,
                                 const LevelIterator & it, std::vector < Array < RangeField > > & dofVec )
      : localFunc_ ( f , dofVec ) , levIt_ ( it )
    {
      localFunc_.init ( *levIt_ );
    };

    //! we use localFunc_ as Interface
    LocalFunctionType & operator *()
    {
      return localFunc_;
    };

    //! go next local function, means go netx grid entity an map to dofs
    LocalFunctionArrayIteratorType& operator++ ()
    {
      ++levIt_;
      localFunc_.init(*levIt_);
      return (*this);
    };

    //! go next i steps
    LocalFunctionArrayIteratorType& operator++ (int i)
    {
      ++levIt_.operator ++ (i);
      localFunc_.init(*levIt_);
      return (*this);
    };

    //! compare LocalFucntionIterator
    bool operator == (const LocalFunctionArrayIteratorType & I )
    {
      return levIt_ == I.levIt_;
    }

    //! compare LocalFucntionIterator
    bool operator != (const LocalFunctionArrayIteratorType & I )
    {
      return levIt_ != I.levIt_;
    }

    int index ()
    {
      return (levIt_->index());
    }

  private:
    //! the level iterator for grid iteration
    LevelIterator levIt_;

    //! the local function
    LocalFunctionType localFunc_;
  }; // end LocalFunctionArrayIterator

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
