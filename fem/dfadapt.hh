// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DFADAPT_HH__
#define __DUNE_DFADAPT_HH__

#include "dune/common/array.hh"
#include "common/discretefunction.hh"
#include "common/fastbase.hh"
#include "common/localfunction.hh"
#include "common/dofiterator.hh"

#include <fstream>
#include <rpc/xdr.h>

namespace Dune {

  template <class DiscreteFunctionSpaceType >    class LocalFunctionAdapt;
  template <class DofType, class DofArrayType >  class DofIteratorAdapt;

  //! defined in dofmanager.hh
  template <class T>                             class DofArray;

  //**********************************************************************
  //
  //  --DFAdapt
  //
  //! this is one special implementation of a discrete function using an
  //! array for storing the dofs.
  //!
  //**********************************************************************
  template<class DiscreteFunctionSpaceType>
  class DFAdapt
    : public DiscreteFunctionDefault < DiscreteFunctionSpaceType,
          DofIteratorAdapt < typename DiscreteFunctionSpaceType::RangeField ,
              DofArray< typename DiscreteFunctionSpaceType::RangeField > > ,
          LocalFunctionAdapt< DiscreteFunctionSpaceType > ,
          DFAdapt <DiscreteFunctionSpaceType> >
  {
  public:
    typedef typename DiscreteFunctionSpaceType::RangeField RangeFieldType;
    typedef DofArray< RangeFieldType > DofArrayType;

  private:
    typedef DiscreteFunctionDefault < DiscreteFunctionSpaceType,
        DofIteratorAdapt < typename DiscreteFunctionSpaceType::RangeField,DofArrayType  > ,
        LocalFunctionAdapt<DiscreteFunctionSpaceType> ,
        DFAdapt <DiscreteFunctionSpaceType > >
    DiscreteFunctionDefaultType;

    enum { myId_ = 0};

  public:
    typedef DofIteratorAdapt<typename DiscreteFunctionSpaceType::RangeField,
        DofArrayType > DofIteratorType;
    typedef ConstDofIteratorDefault<DofIteratorType> ConstDofIteratorType;


    typedef typename DiscreteFunctionSpaceType::MemObjectType MemObjectType;

    typedef DFAdapt <DiscreteFunctionSpaceType> DiscreteFunctionType;
    typedef LocalFunctionAdapt < DiscreteFunctionSpaceType > LocalFunctionType;

    typedef DiscreteFunctionSpaceType FunctionSpaceType;

    //! Constructor make Discrete Function
    DFAdapt(DiscreteFunctionSpaceType& f);

    DFAdapt (const char * name, DiscreteFunctionSpaceType & f ) ;

    //! Constructor make Discrete Function
    DFAdapt (const DFAdapt <DiscreteFunctionSpaceType> & df);

    //! delete stack of free local functions belonging to this discrete
    //! function
    ~DFAdapt ();

    DiscreteFunctionType & argument    () { return *this; }
    const DiscreteFunctionType & argument () const { return *this; }
    DiscreteFunctionType & destination () { return *this; }

    // ***********  Interface  *************************
    //! return object of type LocalFunctionType
    LocalFunctionAdapt<DiscreteFunctionSpaceType> newLocalFunction ( );

    //! update LocalFunction to given Entity en
    template <class EntityType>
    void localFunction ( EntityType &en,
                         LocalFunctionAdapt<DiscreteFunctionSpaceType> & lf);

    //! points to the first dof of type cc
    DofIteratorType dbegin ( );

    //! points behind the last dof of type cc
    DofIteratorType dend   ( );

    //! const version of dof iterator
    ConstDofIteratorType dbegin ( ) const;

    //! const version of dof iterator
    ConstDofIteratorType dend   ( ) const;

    //! set all dofs to zero
    void clear( );

    //! set all dof to value x
    void set( RangeFieldType x );

    //! \todo Please do me!
    void addScaled (const DFAdapt <DiscreteFunctionSpaceType> & g,
                    const RangeFieldType &scalar);

    //! \todo Please do me!
    template <class EntityType>
    void addScaledLocal (EntityType &en,
                         const DFAdapt <DiscreteFunctionSpaceType> & g,
                         const RangeFieldType &scalar);

    //! add g to this on local entity
    template <class EntityType>
    void addLocal (EntityType &it,
                   const DFAdapt <DiscreteFunctionSpaceType> & g);

    //! add g to this on local entity
    template <class EntityType>
    void substractLocal (EntityType &it,
                         const DFAdapt <DiscreteFunctionSpaceType> & g);

    //! \todo Please do me!
    template <class EntityType>
    void setLocal (EntityType &it, const RangeFieldType &scalar);

    //! print all dofs
    void print(std::ostream& s);

    //! write data of discrete function to file filename|timestep
    //! with xdr methods
    bool write_xdr( const char *filename );

    //! write data of discrete function to file filename|timestep
    //! with xdr methods
    bool read_xdr( const char *filename );

    //! write function data to file filename|timestep in ascii Format
    bool write_ascii(const char *filename);

    //! read function data from file filename|timestep in ascii Format
    bool read_ascii(const char *filename);

    //! write function data in pgm fromat file
    bool write_pgm(const char *filename );

    //! read function data from pgm fromat file
    bool read_pgm(const char *filename);

    //! return name of this discrete function
    const char * name () const { return name_; }

    //! return to MemObject which holds the memory of this discrete fucntion
    MemObjectType & memObj() { return memObj_; }

  private:
    // name of this func
    const char * name_;

    // MemObject that manages the memory for the dofs of this function
    MemObjectType & memObj_;

    typedef typename DiscreteFunctionSpaceType::GridType GridType;

    //! array containing the dof of this function, see dofmanager.hh
    DofArrayType dofVec_;

    //! hold one object for addLocal and setLocal and so on
    LocalFunctionType localFunc_;
  }; // end class DFAdapt


  //**************************************************************************
  //
  //  --LocalFunctionAdapt
  //
  //! Implementation of the local functions
  //
  //**************************************************************************
  template < class DiscreteFunctionSpaceType >
  class LocalFunctionAdapt
    : public LocalFunctionDefault <DiscreteFunctionSpaceType ,
          LocalFunctionAdapt < DiscreteFunctionSpaceType >  >
  {
    typedef typename DiscreteFunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;
    typedef LocalFunctionAdapt < DiscreteFunctionSpaceType > MyType;
    typedef DFAdapt <DiscreteFunctionSpaceType> DiscFuncType;

    enum { dimrange = DiscreteFunctionSpaceType::DimRange };

    friend class DFAdapt <DiscreteFunctionSpaceType>;
    typedef typename DiscreteFunctionSpaceType::RangeField RangeFieldType;
    typedef typename DiscreteFunctionSpaceType::Domain DomainType;
    typedef typename DiscreteFunctionSpaceType::Range RangeType;
    typedef typename DiscreteFunctionSpaceType::JacobianRange JacobianRangeType;

    typedef typename DiscFuncType::DofArrayType DofArrayType;
  public:
    //! Constructor
    LocalFunctionAdapt ( const DiscreteFunctionSpaceType &f ,
                         DofArrayType & dofVec );

    //! Destructor
    ~LocalFunctionAdapt ();

    //! access to dof number num, all dofs of the dof entity
    RangeFieldType & operator [] (int num);

    //! access to dof number num, all dofs of the dof entity
    const RangeFieldType & operator [] (int num) const;

    //! return number of degrees of freedom
    int numberOfDofs () const;

    //! sum over all local base functions
    template <class EntityType>
    void evaluate (EntityType &en, const DomainType & x, RangeType & ret) const ;

    //! sum over all local base functions evaluated on given quadrature point
    template <class EntityType, class QuadratureType>
    void evaluate (EntityType &en, QuadratureType &quad, int quadPoint , RangeType & ret) const;

    //! sum over all local base functions evaluated on given quadrature point
    template <class EntityType, class QuadratureType>
    void jacobian (EntityType &en, QuadratureType &quad, int quadPoint , JacobianRangeType & ret) const;

  protected:
    //! update local function for given Entity
    template <class EntityType > bool init ( EntityType &en ) const;

    //! Forbidden! Would wreck havoc
    MyType& operator= (const MyType& other);

    //! needed once
    mutable RangeType tmp_;
    mutable DomainType xtmp_;

    //! needed once
    mutable JacobianRangeType tmpGrad_;

    //! diffVar for evaluate, is empty
    const DiffVariable<0>::Type diffVar;

    //! number of all dofs
    mutable int numOfDof_;

    //! for example number of corners for linear elements
    mutable int numOfDifferentDofs_;

    //! the corresponding function space which provides the base function set
    const DiscreteFunctionSpaceType &fSpace_;

    //! Array holding pointers to the local dofs
    mutable Array < RangeFieldType * > values_;

    //! dofVec from all levels of the discrete function
    DofArrayType & dofVec_;

    //! the corresponding base function set
    mutable BaseFunctionSetType *baseFuncSet_;

    //! do we have the same base function set for all elements
    bool uniform_;
  }; // end LocalFunctionAdapt


  //***********************************************************************
  //
  //  --DofIteratorAdapt
  //! \todo Please doc me!
  //! I guess this is an iterator over an adaptive space.
  //
  //***********************************************************************
  template < class DofImp, class DofArrayType >
  class DofIteratorAdapt : public
                           DofIteratorDefault < DofImp , DofIteratorAdapt < DofImp, DofArrayType > >
  {
    typedef DofIteratorAdapt<DofImp,DofArrayType> MyType;
  public:
    typedef DofImp DofType;
    //! Default constructor
    DofIteratorAdapt () :
      dofArray_(0),
      count_() {}

    //! Constructor (with const)
    DofIteratorAdapt ( const DofArrayType & dofArray , int count )
      :  dofArray_ ( const_cast<DofArrayType*>(&dofArray) ) ,
        count_ ( count ) {}

    //! Constructor (without const)
    DofIteratorAdapt(DofArrayType& dofArray, int count)
      : dofArray_(&dofArray),
        count_(count) {}

    //! Copy constructor
    DofIteratorAdapt(const DofIteratorAdapt<DofImp, DofArrayType>& other) :
      dofArray_ (other.dofArray_),
      count_ (other.count_) {}

    //! Assignment operator
    DofIteratorAdapt<DofImp, DofArrayType>&
    operator= (const DofIteratorAdapt<DofImp, DofArrayType>& other);

    //! return dof
    DofType & operator *();

    //! return dof read only
    const DofType & operator * () const;

    //! go next dof
    MyType & operator++ ();

    //! random access
    DofType& operator[] (int i);

    //! random access read only
    const DofType& operator [] (int i) const;

    //! compare
    bool operator == (const MyType & I ) const;

    //! compare
    bool operator != (const MyType & I ) const;

    //! return actual index
    int index () const;

    //! set dof iterator back to begin , for const and not const Iterators
    void reset () ;

    DofType * vector() { return dofArray_.vector(); }
    const DofType * vector() const { return dofArray_.vector(); }

  private:
    //! the array holding the dofs
    DofArrayType* dofArray_;

    //! index
    mutable int count_;
  }; // end DofIteratorAdapt

} // end namespace Dune

#include "discfuncarray/dfadapt.cc"

#endif
