// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCFUNCARRAY_HH__
#define __DUNE_DISCFUNCARRAY_HH__

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

    //! ???
    typedef typename DiscreteFunctionSpaceType::RangeField RangeFieldType;
    //! ???
    typedef typename DiscreteFunctionSpaceType::GridType GridType;


    //! ???
    typedef DiscFuncArray <DiscreteFunctionSpaceType> DiscreteFunctionType;
    //! ???
    typedef LocalFunctionArray < DiscreteFunctionSpaceType > LocalFunctionType;
    //! ???
    typedef DofIteratorArray < typename DiscreteFunctionSpaceType::RangeField > DofIteratorType;

    //! ???
    typedef DiscreteFunctionSpaceType FunctionSpaceType;

    //! Constructor make empty DiscFuncArray
    DiscFuncArray ( const char * name , DiscreteFunctionSpaceType & f );

    //! Constructor make Discrete Function for all or leaf level
    DiscFuncArray ( DiscreteFunctionSpaceType & f,
                    int level , int codim , bool leaf ) ;

    //! Constructor make Discrete Function for all or leaf level
    DiscFuncArray ( const char * name, DiscreteFunctionSpaceType & f,
                    int level , int codim , bool leaf ) ;

    //! Constructor make Discrete Function for all or leaf level
    DiscFuncArray (const DiscFuncArray <DiscreteFunctionSpaceType> & df);

    //! delete stack of free local functions belonging to this discrete
    //! function
    ~DiscFuncArray ();

    // ***********  Interface  *************************
    //! return object of type LocalFunctionType
    LocalFunctionArray<DiscreteFunctionSpaceType> newLocalFunction ( );

    //! return reference to this
    //! this methods is only to fullfill the interface as parameter classes
    DiscreteFunctionType & argument    () { return *this; }

    //! return reference to this
    //! this methods is only to fullfill the interface as parameter classes
    const DiscreteFunctionType & argument () const { return *this; }

    //! return reference to this
    //! this methods is only to fullfill the interface as parameter classes
    DiscreteFunctionType & destination () { return *this; }

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

    //! Return the name of the discrete function
    const std::string& name() const {return name_;}

    //! set all dofs to zero
    void clearLevel( int level );

    //! set all dofs to zero
    void clear( );

    //! set all dof to value x
    void set( RangeFieldType x );

    //! set all dof to value x
    void setLevel( RangeFieldType x, int level );

    /** \todo Please to me! */
    void addScaled (int level, const DiscFuncArray <DiscreteFunctionSpaceType> & g,
                    const RangeFieldType &scalar);

    /** \todo Please to me! */
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

    /** \todo Please to me! */
    template <class GridIteratorType>
    void setLocal (GridIteratorType &it, const RangeFieldType &scalar);

    //! print all dofs
    void print(std::ostream& s, int level) const;

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

  private:

    // get memory for discrete function
    void getMemory()
    {
      // for all grid levels we have at least a vector with length 0
      int numLevel = this->functionSpace_.getGrid().maxlevel() +1;
      dofVec_.resize(numLevel);

      // this is done only if levOcu_ > 1
      for(int i=0; i<levOcu_-1; i++)
      {
        int length = this->functionSpace_.size( i );
        (dofVec_[i]).resize( length );
        for( int j=0; j<length; j++)
          (dofVec_[i])[j] = 0.0;
      }

      // the last level is done always
      int length = this->functionSpace_.size( level_ );
      (dofVec_[level_]).resize( length );
      for( int j=0; j<length; j++) (dofVec_[level_])[j] = 0.0;
    }

    //! the name of the function
    std::string name_;

    //! true if memory was allocated
    bool built_;

    //! occupied levels
    int levOcu_;

    //! maxlevel which is occupied
    int level_;

    //! false if only leaf level is allocated
    bool allLevels_;

    //! pointer to next free local function object
    //! if this pointer is NULL, new object is created at the class of
    //! localFunction
    LocalFunctionType * freeLocalFunc_;

    //! hold one object for addLocal and setLocal and so on
    LocalFunctionType localFunc_;

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
    typedef typename DiscreteFunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;
    typedef LocalFunctionArray < DiscreteFunctionSpaceType > MyType;
    typedef DiscFuncArray <DiscreteFunctionSpaceType> DiscFuncType;

    enum { dimrange = DiscreteFunctionSpaceType::DimRange };
    typedef typename DiscreteFunctionSpaceType::Domain DomainType;
    typedef typename DiscreteFunctionSpaceType::Range RangeType;
    typedef typename DiscreteFunctionSpaceType::RangeField RangeFieldType;
    typedef typename DiscreteFunctionSpaceType::JacobianRange JacobianRangeType;

    friend class DiscFuncArray <DiscreteFunctionSpaceType>;
  public:
    //! Constructor
    LocalFunctionArray ( const DiscreteFunctionSpaceType &f ,
                         std::vector < Array < RangeFieldType > > & dofVec );

    //! Destructor
    ~LocalFunctionArray ();

    //! access to dof number num, all dofs of the dof entity
    RangeFieldType & operator [] (int num);

    //! access to dof number num, all dofs of the dof entity
    const RangeFieldType & read (int num) const;

    //! return number of degrees of freedom
    int numberOfDofs () const;

    //! sum over all local base functions
    template <class EntityType>
    void evaluate (EntityType &en, const DomainType & x, RangeType & ret) const ;

    //! sum over all local base functions evaluated on given quadrature point
    template <class EntityType, class QuadratureType>
    void evaluate (EntityType &en, QuadratureType &quad, int quadPoint , RangeType & ret) const;

  protected:
    //! update local function for given Entity
    template <class EntityType > bool init ( EntityType &en ) const;

    //! get pointer to next LocalFunction
    MyType * getNext() const;

    //! set pointer to next LocalFunction
    void setNext (MyType * n);

    //! the corresponding function space which provides the base function set
    const DiscreteFunctionSpaceType &fSpace_;

    //! dofVec from all levels of the discrete function
    typename std::vector < Array < RangeFieldType > > & dofVec_;

    //! Array holding pointers to the local dofs
    mutable Array < RangeFieldType * > values_;

    //! remember pointer to next LocalFunction
    MyType * next_;

    //! needed once
    mutable RangeType tmp_;
    mutable DomainType xtmp_;

    //! needed once
    JacobianRangeType tmpGrad_;

    //! diffVar for evaluate, is empty
    const DiffVariable<0>::Type diffVar;

    //! number of all dofs
    mutable int numOfDof_;

    //! for example number of corners for linear elements
    mutable int numOfDifferentDofs_;

    //! the corresponding base function set
    mutable const BaseFunctionSetType *baseFuncSet_;

    //! do we have the same base function set for all elements
    bool uniform_;

  }; // end LocalFunctionArray


  //***********************************************************************
  //
  //  --DofIteratorArray
  //
  //***********************************************************************
  /** \brief ??? */
  template < class DofType >
  class DofIteratorArray : public
                           DofIteratorDefault < DofType , DofIteratorArray < DofType > >
  {
  public:
    DofIteratorArray ( Array < DofType > & dofArray , int count )
      :  dofArray_ ( dofArray ) , constArray_ (dofArray) , count_ ( count ) {};

    DofIteratorArray ( const Array < DofType > & dofArray , int count )
      :  dofArray_ ( const_cast <Array < DofType > &> (dofArray) ) ,
        constArray_ ( dofArray ) ,
        count_ ( count ) {};

    //! return dof
    DofType & operator *();

    //! return dof read only
    const DofType & operator * () const;

    //! go next dof
    DofIteratorArray<DofType> & operator++ ();

    //! go next dof
    const DofIteratorArray<DofType> & operator++ () const;

    //! random access
    DofType& operator[] (int i);

    //! random access read only
    const DofType& operator [] (int i) const;

    //! compare
    bool operator == (const DofIteratorArray<DofType> & I ) const;

    //! compare
    bool operator != (const DofIteratorArray<DofType> & I ) const;

    //! return actual index
    int index () const;

    //! set dof iterator back to begin , for const and not const Iterators
    void reset () const;

  private:
    //! the array holding the dofs
    Array < DofType > &dofArray_;

    //! the array holding the dofs , only const reference
    const Array < DofType > &constArray_;

    //! index
    mutable int count_;

  }; // end DofIteratorArray


} // end namespace Dune

#include "discfuncarray/discfuncarray.cc"

#endif
