// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISFUNCARRAY_HH__
#define __DUNE_DISFUNCARRAY_HH__

#include <dune/common/array.hh>

#include "common/discretefunction.hh"
#include "common/fastbase.hh"
#include "common/localfunction.hh"
#include "common/dofiterator.hh"

#include <fstream>
#include <rpc/xdr.h>

namespace Dune {

  template <class DiscreteFunctionSpaceType > class LocalFunctionArray;
  template < class DiscreteFunctionType, class GridIteratorType >
  class LocalFunctionArrayIterator;
  template < class DofType >                  class DofIteratorArray;

  //**********************************************************************
  //
  //  --DiscFuncArray
  //
  //! this is one special implementation of a discrete function using an
  //! array for storing the dofs.
  //!
  //**********************************************************************
  template<class DiscreteFunctionSpaceType >
  class DiscFuncArray
    : public DiscreteFunctionDefault < DiscreteFunctionSpaceType,
          DofIteratorArray < typename DiscreteFunctionSpaceType::RangeField > ,
          LocalFunctionArrayIterator ,
          DiscFuncArray <DiscreteFunctionSpaceType> >
  {
    typedef DiscreteFunctionDefault < DiscreteFunctionSpaceType,
        DofIteratorArray < typename DiscreteFunctionSpaceType::RangeField > ,
        LocalFunctionArrayIterator ,
        DiscFuncArray <DiscreteFunctionSpaceType > >
    DiscreteFunctionDefaultType;


    enum { myId_ = 0};
  public:
    typedef DiscFuncArray <DiscreteFunctionSpaceType> DiscreteFunctionType;
    typedef LocalFunctionArray < DiscreteFunctionSpaceType > LocalFunctionType;

    template <class GridIteratorType>
    struct Traits
    {
      typedef LocalFunctionArrayIterator < DiscreteFunctionType,
          GridIteratorType> LocalFunctionIteratorType;
    };

    typedef DiscreteFunctionSpaceType FunctionSpaceType;

    //! Constructor make empty DiscFuncArray
    DiscFuncArray ( DiscreteFunctionSpaceType & f );

    //! Constructor make Discrete Function for all or leaf level
    DiscFuncArray ( DiscreteFunctionSpaceType & f,
                    int level , int codim , bool leaf ) ;

    //! Constructor make Discrete Function for all or leaf level
    DiscFuncArray (const DiscFuncArray <DiscreteFunctionSpaceType> & df);

    //! delete stack of free local functions belonging to this discrete
    //! function
    ~DiscFuncArray ();

    // ***********  Interface  *************************
    //! return object of type LocalFunctionType
    LocalFunctionArray<DiscreteFunctionSpaceType> newLocalFunction ( );

    //! update LocalFunction to given Entity en
    template <class EntityType>
    void localFunction ( EntityType &en,
                         LocalFunctionArray<DiscreteFunctionSpaceType> & lf);

    // we use the default implementation
    DofIteratorType dbegin ( int level );

    //! points behind the last dof of type cc
    DofIteratorType dend   ( int level );

    // the const versions
    // we use the default implementation
    const DofIteratorType dbegin ( int level ) const;

    //! points behind the last dof of type cc
    const DofIteratorType dend   ( int level ) const;

    //! return if allLevels are used
    bool allLevels () { return allLevels_; }

    //! set all dofs to zero
    void clearLevel( int level );
    void clear( );

    //! set all dof to value x
    void set( RangeFieldType x );
    void setLevel( RangeFieldType x, int level );

    void addScaled (int level, const DiscFuncArray <DiscreteFunctionSpaceType> & g,
                    const RangeFieldType &scalar);

    template <class GridIteratorType>
    void addScaledLocal (GridIteratorType &it,
                         const DiscFuncArray <DiscreteFunctionSpaceType> & g,
                         const RangeFieldType &scalar);

    //! add g to this on local entity
    template <class GridIteratorType>
    void addLocal (GridIteratorType &it,
                   const DiscFuncArray <DiscreteFunctionSpaceType> & g);

    //! add g to this on local entity
    template <class GridIteratorType>
    void substractLocal (GridIteratorType &it,
                         const DiscFuncArray <DiscreteFunctionSpaceType> & g);

    template <class GridIteratorType>
    void setLocal (GridIteratorType &it, const RangeFieldType &scalar);

    //! print all dofs
    void print(std::ostream& s, int level);

    //! write leaf data to file in USPM format for Grape
    bool write_USPM(const char *filename , int timestep);

    //! write data of discrete function to file filename|timestep
    //! with xdr methods
    bool write_xdr( const char *filename , int timestep );

    //! write data of discrete function to file filename|timestep
    //! with xdr methods
    bool read_xdr( const char *filename , int timestep );

    //! write function data to file filename|timestep in ascii Format
    bool write_ascii(const char *filename, int timestep);

    //! read function data from file filename|timestep in ascii Format
    bool read_ascii(const char *filename, int timestep);

    //! write function data in pgm fromat file
    bool write_pgm(const char *filename, int timestep) ;

    //! read function data from pgm fromat file
    bool read_pgm(const char *filename, int timestep);

    //***** methods that not belong to the interface ************
    //***** but have to be public *******************************

    //! get local function pointer, if not exists new object is created
    LocalFunctionType * getLocalFunction ();

    //! free access to LocalFunction for next use
    void freeLocalFunction (LocalFunctionType * lf );
  private:

    // get memory for discrete function
    void getMemory()
    {
      // for all grid levels we have at least a vector with length 0
      int numLevel = functionSpace_.getGrid().maxlevel() +1;
      dofVec_.resize(numLevel);
      for(int i=0; i<numLevel; i++)
        dofVec_[i] = NULL;

      // this is done only if levOcu_ > 1
      for(int i=0; i<levOcu_-1; i++)
      {
        int length = functionSpace_.size( i );
        (dofVec_[i]).resize( length );
        for( int j=0; j<length; j++)
          (dofVec_[i])[j] = 0.0;
      }

      // the last level is done always
      int length = functionSpace_.size( level_ );
      (dofVec_[level_]).resize( length );
      for( int j=0; j<length; j++) (dofVec_[level_])[j] = 0.0;
    }

    //! true if memory was allocated
    bool built_;

    //! false if only leaf level is allocated
    bool allLevels_;

    //! occupied levels
    int levOcu_;

    //! maxlevel which is occupied
    int level_;

    //! pointer to next free local function object
    //! if this pointer is NULL, new object is created at the class of
    //! localFunction
    LocalFunctionType * freeLocalFunc_;

    //! for all level an Array < RangeField > , the data
    std::vector < Array < RangeFieldType > > dofVec_;
  }; // end class DiscFuncArray


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
    typedef DiscFuncArray <DiscreteFunctionSpaceType> DiscFuncType;

    enum { dimrange = DiscreteFunctionSpaceType::DimRange };

    friend class DiscFuncArray <DiscreteFunctionSpaceType>;
  public:
    //! Constructor
    LocalFunctionArray ( const DiscreteFunctionSpaceType &f ,
                         std::vector < Array < RangeFieldType > > & dofVec );

    //! Destructor
    ~LocalFunctionArray ();

    //! access to dof number num, all dofs of the dof entity
    RangeFieldType & operator [] (int num);

    //! return number of degrees of freedom
    int numberOfDofs () const;

    //! sum over all local base functions
    template <class EntityType>
    void evaluate (EntityType &en, const DomainType & x, RangeType & ret) const ;

    //! sum over all local base functions evaluated on given quadrature point
    template <class EntityType, class QuadratureType>
    void evaluate (EntityType &en, QuadratureType &quad, int quadPoint , RangeType & ret) const;

    //********* Method that no belong to the interface *************
    //! methods that do not belong to the interface but have to be public
    //! used like setElInfo and so on
    template <class EntityType > bool init ( EntityType &en );
  protected:

    //! get pointer to next LocalFunction
    MyType * getNext() const;

    //! set pointer to next LocalFunction
    void setNext (MyType * n);

    //! needed once
    mutable RangeType tmp;

    //! needed once
    JacobianRangeType tmpGrad_;

    //! remember pointer to next LocalFunction
    MyType * next_;

    //! diffVar for evaluate, is empty
    const DiffVariable<0>::Type diffVar;

    //! number of all dofs
    int numOfDof_;

    //! for example number of corners for linear elements
    int numOfDifferentDofs_;

    //! the corresponding function space which provides the base function set
    const DiscreteFunctionSpaceType &fSpace_;

    //! Array holding pointers to the local dofs
    Array < RangeFieldType * > values_;

    //! dofVec from all levels of the discrete function
    typename std::vector < Array < RangeFieldType > > & dofVec_;

    //! do we have the same base function set for all elements
    bool uniform_;

    //! the corresponding base function set
    const BaseFunctionSetType *baseFuncSet_;
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
      :  dofArray_ ( dofArray ) , constArray_ (dofArray) , count_ ( count ) {};

    DofIteratorArray ( const Array < DofType > & dofArray , int count )
      :  constArray_ ( dofArray ) ,
        dofArray_ ( const_cast <Array < DofType > &> (dofArray) ) ,
        count_ ( count ) {};

    //! return dof
    DofType & operator *();

    //! return dof read only
    const DofType & read () const;

    //! go next dof
    DofIteratorArray<DofType> & operator++ ();

    //! go next dof
    const DofIteratorArray<DofType> & operator++ () const;

    //! go next i steps
    DofIteratorArray<DofType> & operator++ (int i);

    //! go next i steps
    const DofIteratorArray<DofType> & operator++ (int i) const;

    //! random access
    DofType& operator[] (int i);

    //! random access read only
    const DofType& read (int i) const;

    //! compare
    bool operator == (const DofIteratorArray<DofType> & I ) const;

    //! compare
    bool operator != (const DofIteratorArray<DofType> & I ) const;

    //! return actual index
    int index () const;

    //! set dof iterator back to begin , for const and not const Iterators
    void reset () const;

  private:
    //! index
    mutable int count_;

    //! the array holding the dofs
    Array < DofType > &dofArray_;

    //! the array holding the dofs , only const reference
    const Array < DofType > &constArray_;

  }; // end DofIteratorArray

  //**************************************************************************
  //
  //  --LocalFunctionArrayIterator
  //
  //! Iterator to navigate through the local functions
  //! The Storage of the dofs is implemented via an array
  //
  //**************************************************************************
  template < class DiscFunctionType , class GridIteratorType >
  class LocalFunctionArrayIterator
    : public LocalFunctionIteratorDefault <
          LocalFunctionArray < typename DiscFunctionType::FunctionSpace > ,
          LocalFunctionArrayIterator < DiscFunctionType, GridIteratorType >
          >

  {
    typedef LocalFunctionArray < typename DiscFunctionType::FunctionSpace >
    LocalFunctionType;

    // just for readability
    typedef LocalFunctionArrayIterator < DiscFunctionType , GridIteratorType >
    LocalFunctionArrayIteratorType;

    typedef typename GridIteratorType::Traits::Entity EntityType;

  public:
    //! Constructor
    LocalFunctionArrayIterator ( DiscFunctionType &df , GridIteratorType & it );

    //! Copy Constructor
    LocalFunctionArrayIterator ( const LocalFunctionArrayIteratorType & copy);

    //! Desctructor
    ~LocalFunctionArrayIterator ();

    //! we use localFunc_ as Interface
    LocalFunctionType & operator *();

    //! we use localFunc_ as Interface
    LocalFunctionType * operator ->();

    //! go next local function, means go netx grid entity an map to dofs
    LocalFunctionArrayIteratorType& operator++ ();

    //! go next i steps
    LocalFunctionArrayIteratorType& operator++ (int i);

    //! compare LocalFucntionIterator
    bool operator == (const LocalFunctionArrayIteratorType & I ) const;

    //! compare LocalFucntionIterator
    bool operator != (const LocalFunctionArrayIteratorType & I ) const;

    int index () const;

    void update ( GridIteratorType & it );

  private:
    //! true if local function init was called already
    bool built_;

    //! GridIteratorType can be LevelIterator, HierarchicIterator or
    //! ItersectionIterator or LeafIterator
    GridIteratorType & it_;

    //! needed for access of local functions
    DiscFunctionType &df_;

    //! pointer to the local function
    LocalFunctionType *lf_;
  }; // end LocalFunctionArrayIterator

} // end namespace Dune

#include "discfuncarray/discfuncarray.cc"

#endif
