// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DOFSTORAGEARRAY_HH__
#define __DUNE_DOFSTORAGEARRAY_HH__

#include <vector>
#include "dofstorage.hh"

namespace Dune {

  //**************************************************************************
  //
  //  --LocalDofIteratorArray
  //
  //! The DofStorage class holds the memory with the dofs of a grid function.
  //! The DofStorage class must provide a iterator for global walk through
  //! all dofs and a local walk through all local dofs of a element.
  //! Furthermore there should be a random access to global dof number n and
  //! to local dof number l. Perhaps there should be a mapToGlobal method?
  //
  //**************************************************************************
  template <class DofType>
  class LocalDofIteratorArray : public
                                LocalDofIteratorDefault < DofType , LocalDofIteratorArray < DofType > >
  {
    // just for readability
    typedef LocalDofIteratorArray < DofType > LocalDofIteratorImp;
  public:
    //! Constructor getting the array that holds the dofs
    LocalDofIteratorArray ( Array < DofType > &dofVec , Array < int > & map, int actIndex  )
      : dofVec_ ( dofVec ), map_ ( map ) , count_ ( actIndex )  {};

    //! return local dof
    DofType& operator *()
    {
      return dofVec_ [ map_ [ count_ ] ];
    };

    //! go next dof
    LocalDofIteratorImp& operator++ ()
    {
      count_++;
      return (*this);
    };

    //! go next i steps
    LocalDofIteratorImp& operator++ (int i)
    {
      count_ += i;
      return (*this);
    };

    //! comparison
    bool operator == (const LocalDofIteratorImp & I )
    {
      return count_ == I.count_;
    }

    //! comparison
    bool operator != (const LocalDofIteratorImp & I )
    {
      return count_ != I.count_;
    }

    //! return local dof number
    int index ()
    {
      return count_;
    };

  private:
    //! local dof number
    int count_;

    //! lookup table for local to global mapping
    Array < int > &map_;

    //! array storing the dofs
    Array < DofType > & dofVec_;
  };

  //**************************************************************************
  //
  //  --GlobalDofIteratorArray
  //
  //! The GlobalDofIteratorArray can esayly walk through global dof, because
  //! all dofs are stored in an array. The GlobalDofIteratorArray is more or
  //! less an array iterator.
  //
  //**************************************************************************
  template <class DofType>
  class GlobalDofIteratorArray : public
                                 GlobalDofIteratorDefault < DofType , GlobalDofIteratorArray < DofType > >
  {
    //! just for the programer
    typedef  GlobalDofIteratorArray <DofType> GlobalDofIteratorImp;
  public:

    GlobalDofIteratorArray ( Array < DofType > &dofVec , int actIndex )
      : dofVec_ ( dofVec ) , count_ ( actIndex ) {};

    DofType& operator *()
    {
      return dofVec_ [ count_ ];
    };

    GlobalDofIteratorImp& operator++ ()
    {
      count_++;
      return (*this);
    };

    GlobalDofIteratorImp& operator++ (int i)
    {
      count_ += i;
      return (*this);
    };

    bool operator == (const GlobalDofIteratorImp & I )
    {
      return count_ == I.count_;
    }

    bool operator != (const GlobalDofIteratorImp & I )
    {
      return count_ != I.count_;
    }

    int index () { return count_; };
  private:
    //! the global dof number
    int count_;

    //! the array holding the dofs
    Array < DofType > & dofVec_;
  };


  //************************************************************************
  //
  //  --DofStorageArray
  //
  //! Here the storage of the dofs is done in an array.
  //! Furthermore the functionality of the DofStorageDefault is overloaded
  //! because here this can be done more efficiently.
  //
  //************************************************************************
  template <class DofType , class FunctionSpaceType  >
  class DofStorageArray
    : public DofStorageDefault < DofType, LocalDofIteratorArray <DofType > ,
          GlobalDofIteratorArray < DofType > ,
          DofStorageArray < DofType,  FunctionSpaceType > >
  {
    typedef Traits::LocalDofIterator LocalDofIterator;
    typedef Traits::GlobalDofIterator GlobalDofIterator;
  public:

    //! the dof type , to be revised
    typedef typename FunctionSpaceType::RangeField RangeField;

    //! the dof storage array , a vector of arrays , one for each level
    typedef std::vector < Array<RangeField> > ArrayType;

    //! Constructor
    DofStorageArray ( const FunctionSpaceType &fuSpace , int level , int codim , bool flag ) :
      functionSpace_ ( fuSpace ) , level_ ( level ), codim_ ( codim )
    {
      if(flag)
        levOcu_ = level_+1;
      else
        levOcu_ = 1;

      dofVec_.resize(levOcu_);

      // this is done only if levOcu_ > 1
      for(int i=0; i< levOcu_-1; i++)
      {
        int length = functionSpace_.size( i );
        (dofVec_[i]).realloc( length );
        for( int j=0; j<length; j++)
          (dofVec_[i])[j] = 0.0;
      }

      int length = functionSpace_.size( level_ );
      (dofVec_[levOcu_-1]).realloc( length );
      for( int j=0; j<length; j++) (dofVec_[levOcu_-1])[j] = 0.0;

      // I want a special operator for  (dofVec_[levOcu_-1]) = 0.0;
    };

    //! provide the Interface functionality
    //! with the array storage method this is a bit slow but so what
    template <class EntityType >
    LocalDofIterator beginLocal(EntityType &en)
    {
      int localDofNum =
        functionSpace_.getBaseFunctionSet(en).getNumberOfBaseFunctions();

      if(localDofNum > localMap_.size() )
      {
        localMap_.realloc ( localDofNum );
      }

      // with this DofStorgge the local to global Mapping is given to the
      // LocalDofIterator to provide fast access
      for(int i=0; i<localDofNum; i++)
        localMap_[i] = mapToGlobal (en , i );

      int lev = 0;
      if(levOcu_ > 1) lev = en.level();

      LocalDofIterator tmp ( dofVec_[lev] , localMap_ , 0 );
      return tmp;
    };

    //! provide the Interface functionality
    template <class EntityType >
    LocalDofIterator endLocal(EntityType &en)
    {
      // localDofNum marks the end for LocalDofIterator
      int localDofNum =
        functionSpace_.getBaseFunctionSet(en).getNumberOfBaseFunctions();

      LocalDofIterator tmp ( dofVec_[en.level()], localMap_ , localDofNum );
      return tmp;
    };

    //! provide the Interface functionality
    template <class GridType>
    GlobalDofIterator beginGlobal (GridType &grid , int level )
    {
      // check if more than one level is occupied
      int lev = 0;
      if(levOcu_ > 1)
        lev = level;

      GlobalDofIterator tmp ( dofVec_[lev], 0 );
      return tmp;
    };

    //! provide the Interface functionality
    template <class GridType>
    GlobalDofIterator endGlobal (GridType &grid, int level )
    {
      // check if more than one level is occupied
      int lev = 0;
      if(levOcu_ > 1)
        lev = level;

      GlobalDofIterator tmp ( dofVec_[lev], dofVec_[lev].size() );
      return tmp;
    };

    //! overload the default method because we can do it better
    template <class GridType>
    DofType& global (GridType &grid, int level, int globalNumber)
    {
      return dofVec_ [level ][ globalNumber ] ;
    };

    //! overload the default method because we can do it better
    template <class EntityType>
    DofType& local (EntityType &en, int localNum )
    {
      return dofVec_[en.level()]
             [ mapToGlobal (en , localNum )];
    };

  private:
    //! we know how to map our dofs
    template <class EntityType>
    int mapToGlobal ( EntityType &en , int localNum ) const
    {
      return functionSpace_.mapToGlobal(en, localNum);
    };

    //! know the function space which the dof generating the function
    //! belongs to
    const FunctionSpaceType &functionSpace_;

    //! occupied levels
    int levOcu_;

    //! know the codim
    int codim_;

    //! maxlevel which is occupied
    int level_;

    //! build in array holding local mapping information for LocalIterator
    Array < int > localMap_;

    //! Array holding the dofs
    ArrayType dofVec_;

  }; // end class DofStorageArray


} // end namespace Dune

#endif
