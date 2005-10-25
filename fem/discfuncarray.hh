// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DISCFUNCARRAY_HH
#define DUNE_DISCFUNCARRAY_HH

#include <dune/common/array.hh>

#include "common/discretefunction.hh"
#include "common/fastbase.hh"
#include "common/localfunction.hh"
#include "common/dofiterator.hh"

#include <fstream>
#include <rpc/xdr.h>

namespace Dune {

  template <class DiscreteFunctionSpaceType > class DiscFuncArray;
  template <class DiscreteFunctionSpaceType > class LocalFunctionArray;
  template <class DofType>                    class DofIteratorArray;


  template <class DiscreteFunctionSpaceImp>
  struct DiscFuncArrayTraits {
    typedef DiscreteFunctionSpaceImp DiscreteFunctionSpaceType;
    typedef DiscFuncArray<DiscreteFunctionSpaceImp> DiscreteFunctionType;
    typedef LocalFunctionArray<DiscreteFunctionSpaceImp> LocalFunctionImp;
    typedef LocalFunctionWrapper<DiscreteFunctionType> LocalFunctionType;
    typedef DofIteratorArray<typename DiscreteFunctionSpaceImp::RangeFieldType> DofIteratorType;
    typedef ConstDofIteratorDefault<DofIteratorType> ConstDofIteratorType;
  };


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
    : public DiscreteFunctionDefault <DiscFuncArrayTraits<DiscreteFunctionSpaceType> >
  {
    typedef DiscreteFunctionDefault<DiscFuncArrayTraits <DiscreteFunctionSpaceType > >
    DiscreteFunctionDefaultType;

    friend class DiscreteFunctionDefault< DiscFuncArrayTraits <DiscreteFunctionSpaceType > > ;

    enum { myId_ = 0};
  public:

    //! my type
    typedef DiscFuncArray < DiscreteFunctionSpaceType > DiscreteFunctionType;

    //! Type of the range field
    typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;

    /** \brief For ISTL-compatibility */
    typedef FieldVector<RangeFieldType,1> block_type;

    //! Type of the grid
    typedef typename DiscreteFunctionSpaceType::GridType GridType;

    //! the MyType
    typedef DiscFuncArray <DiscreteFunctionSpaceType> DiscreteFunctionType;

    //! the local function implementation e
    typedef LocalFunctionArray<DiscreteFunctionSpaceType> LocalFunctionImp;

    //! LocalFunctionType is the exported lf type
    typedef LocalFunctionWrapper < DiscreteFunctionType > LocalFunctionType;

    // the storage of the local functions
    typedef LocalFunctionStorage< DiscreteFunctionType > LocalFunctionStorageType;

    //! the dof iterator type of this function
    typedef DofIteratorArray <typename DiscreteFunctionSpaceType::RangeFieldType> DofIteratorType;
    typedef ConstDofIteratorDefault<DofIteratorType> ConstDofIteratorType;

    //! The associated discrete function space
    typedef DiscreteFunctionSpaceType FunctionSpaceType;

    //! our traits, like DofIterator etc.
    typedef DiscFuncArrayTraits <DiscreteFunctionSpaceType > Traits;

    //! the type of the unknowns
    typedef RangeFieldType DofType;

    //! Constructor makes Discrete Function
    DiscFuncArray ( const DiscreteFunctionSpaceType & f ) ;

    //! Constructor makes Discrete Function with name
    DiscFuncArray ( const char * name, const DiscreteFunctionSpaceType & f ) ;

    //! Constructor makes Discrete Function from copy
    DiscFuncArray (const DiscFuncArray <DiscreteFunctionSpaceType> & df);

    //! delete stack of free local functions belonging to this discrete
    //! function
    ~DiscFuncArray ();

    // ***********  Interface  *************************
    //! return object of type LocalFunctionType
    LocalFunctionType newLocalFunction () DUNE_DEPRECATED;

    //! return local function for given entity
    template <class EntityType>
    LocalFunctionType localFunction(const EntityType& en) const;

    //! update LocalFunction to given Entity en
    template <class EntityType>
    void localFunction ( const EntityType &en, LocalFunctionType & lf) DUNE_DEPRECATED;

    //! return reference to this
    //! this methods is only to fullfill the interface as parameter classes
    DiscreteFunctionType & argument    () { return *this; }

    //! return reference to this
    //! this methods is only to fullfill the interface as parameter classes
    const DiscreteFunctionType & argument () const { return *this; }

    //! return reference to this
    //! this methods is only to fullfill the interface as parameter classes
    DiscreteFunctionType & destination () { return *this; }

    //! we use the default implementation
    DofIteratorType dbegin ();

    //! points behind the last dof of type cc
    DofIteratorType dend   ();

    //! the const versions
    //! we use the default implementation
    ConstDofIteratorType dbegin () const;

    //! points behind the last dof of type cc
    ConstDofIteratorType dend   () const;

    //! Return the name of the discrete function
    const std::string& name() const {return name_;}

    //! return size of this discrete function
    int size() const { return dofVec_.size(); }

    //! set all dofs to zero
    void clear( );

    //! set all dof to value x
    void set( RangeFieldType x );

    //! add g * scalar to discrete function
    void addScaled ( const DiscFuncArray <DiscreteFunctionSpaceType> & g,
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
    void print(std::ostream& s) const;

    //! write data of discrete function to file filename
    //! with xdr methods
    bool write_xdr( const char *filename );

    //! write data of discrete function to file filename
    //! with xdr methods
    bool read_xdr( const char *filename );

    //! write function data to file filename in ascii Format
    bool write_ascii(const char *filename);

    //! read function data from file filename in ascii Format
    bool read_ascii(const char *filename);

    //! write function data in pgm fromat file
    bool write_pgm(const char *filename) ;

    //! read function data from pgm fromat file
    bool read_pgm(const char *filename);

    //! return pointer to internal array for use of BLAS routines
    DofType * leakPointer () { return &(dofVec_[0]);  };
    //! return pointer to internal array for use of BLAS routines
    const DofType * leakPointer () const { return &(dofVec_[0]); };

  private:
    //! return object pointer of type LocalFunctionImp
    LocalFunctionImp * newLocalFunctionObject () const;

    // get memory for discrete function
    void getMemory();

    //! the name of the function
    std::string name_;

    //! true if memory was allocated
    bool built_;

    //! hold one object for addLocal and setLocal and so on
    LocalFunctionImp localFunc_;

    //! the dofs stored in an array
    Array < RangeFieldType > dofVec_;
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
    typedef typename DiscreteFunctionSpaceType::DomainType DomainType;
    typedef typename DiscreteFunctionSpaceType::RangeType RangeType;
    typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename DiscreteFunctionSpaceType::JacobianRangeType JacobianRangeType;

    friend class DiscFuncArray <DiscreteFunctionSpaceType>;
    friend class LocalFunctionWrapper < DiscFuncArray <DiscreteFunctionSpaceType> >;
  public:
    //! Constructor
    LocalFunctionArray ( const DiscreteFunctionSpaceType &f , Array < RangeFieldType >  & dofVec );

    //! Destructor
    ~LocalFunctionArray ();

    //! access to dof number num, all dofs of the dof entity
    RangeFieldType & operator [] (int num);

    //! access to dof number num, all dofs of the dof entity
    const RangeFieldType & operator [] (int num) const;

    //! return number of degrees of freedom
    int numberOfDofs () const DUNE_DEPRECATED;

    //! return number of degrees of freedom
    int numDofs () const;

    //! sum over all local base functions
    template <class EntityType>
    void evaluate (EntityType &en, const DomainType & x, RangeType & ret) const ;

    //! sum over all local base functions evaluated on given quadrature point
    template <class EntityType, class QuadratureType>
    void evaluate (EntityType &en, QuadratureType &quad, int quadPoint , RangeType & ret) const;

  protected:
    //! update local function for given Entity
    template <class EntityType >
    void init ( const EntityType &en ) const;

    //! the corresponding function space which provides the base function set
    const DiscreteFunctionSpaceType &fSpace_;

    //! dofVec from all levels of the discrete function
    Array < RangeFieldType > & dofVec_;

    //! Array holding pointers to the local dofs
    mutable Array < RangeFieldType * > values_;

    //! diffVar for evaluate, is empty
    const DiffVariable<0>::Type diffVar;

    //! number of all dofs
    mutable int numOfDof_;

    //! for example number of corners for linear elements
    mutable int numOfDifferentDofs_;

    //! do we have the same base function set for all elements
    bool uniform_;

    //! is it initialised?
    mutable bool init_;

  }; // end LocalFunctionArray


  //***********************************************************************
  //
  //  --DofIteratorArray
  //
  //***********************************************************************
  /** \brief Iterator over an array of dofs
      \todo Please doc me!
   */
  template < class DofImp >
  class DofIteratorArray : public
                           DofIteratorDefault < DofImp , DofIteratorArray < DofImp > >
  {
  public:
    typedef DofImp DofType;

    //! Default constructor
    DofIteratorArray() :
      dofArray_ (0) ,
      count_() {}

    //! Constructor (const)
    DofIteratorArray ( const Array < DofType > & dofArray , int count )
      :  dofArray_ (const_cast<DofArrayType*>(&dofArray)) ,
        count_ (count) {}

    //! Constructor
    DofIteratorArray(Array<DofType>& dofArray, int count)
      : dofArray_(&dofArray),
        count_(count) {}

    //! Copy Constructor
    DofIteratorArray (const DofIteratorArray<DofImp>& other);

    //! Assignment operator
    DofIteratorArray<DofImp>& operator=(const DofIteratorArray<DofImp>& other);

    //! return dof
    DofType & operator *();

    //! return dof read only
    const DofType & operator * () const;

    //! go next dof
    DofIteratorArray<DofType> & operator++ ();

    //! random access
    DofType& operator[] (int i);

    //! random access read only
    const DofType& operator[] (int i) const;

    //! compare
    bool operator == (const DofIteratorArray<DofType> & I ) const;

    //! compare
    bool operator != (const DofIteratorArray<DofType> & I ) const;

    //! return actual index
    int index () const;

    //! set dof iterator back to begin , for const and not const Iterators
    void reset () ;

  private:
    typedef Array<DofType> DofArrayType;
    //! the array holding the dofs
    Array < DofType >* dofArray_;

    //! index
    mutable int count_;

  }; // end DofIteratorArray


} // end namespace Dune

#include "discfuncarray/discfuncarray.cc"

#endif
