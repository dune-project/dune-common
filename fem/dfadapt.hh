// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DFADAPT_HH
#define DUNE_DFADAPT_HH

#include "dune/common/array.hh"
#include "common/discretefunction.hh"
#include "common/fastbase.hh"
#include "common/localfunction.hh"
#include "common/dofiterator.hh"
#include "dofmanager.hh"

#include <fstream>
#include <rpc/xdr.h>

namespace Dune {

  template <class DiscreteFunctionSpaceImp>    class LocalFunctionAdapt;
  template <class DofType, class DofArrayType>  class DofIteratorAdapt;
  template <class DiscreteFunctionSpaceImp> class DFAdapt;

  template <class DiscreteFunctionSpaceImp>
  struct DFAdaptTraits {
    typedef DiscreteFunctionSpaceImp DiscreteFunctionSpaceType;

    typedef DFAdapt<DiscreteFunctionSpaceImp> DiscreteFunctionType;
    typedef LocalFunctionAdapt<DiscreteFunctionSpaceImp> LocalFunctionImp;
    typedef LocalFunctionWrapper< DiscreteFunctionType > LocalFunctionType;

    typedef typename DofArray<
        typename DiscreteFunctionSpaceImp::RangeFieldType
        >::DofIteratorType DofIteratorType;
    typedef typename DofArray<
        typename DiscreteFunctionSpaceImp::RangeFieldType
        >::ConstDofIteratorType ConstDofIteratorType;
  };

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
    : public DiscreteFunctionDefault<
          DFAdaptTraits<DiscreteFunctionSpaceType>
          >
  {
  public:
    typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;
    typedef DofArray< RangeFieldType > DofArrayType;

  private:
    typedef DiscreteFunctionDefault<
        DFAdaptTraits<DiscreteFunctionSpaceType>
        > DiscreteFunctionDefaultType;
    friend class DiscreteFunctionDefault< DFAdaptTraits<DiscreteFunctionSpaceType> >;

    typedef typename DiscreteFunctionSpaceType::GridType GridType;

    typedef DofManager<GridType> DofManagerType;
    typedef DofManagerFactory<DofManagerType> DofManagerFactoryType;

    enum { myId_ = 0};

  public:
    typedef typename DiscreteFunctionSpaceType::Traits::MapperType MapperType;
    typedef typename DiscreteFunctionSpaceType::Traits::RangeFieldType DofType;
    typedef typename DofArrayType::DofIteratorType DofIteratorType;
    typedef typename DofArrayType::ConstDofIteratorType ConstDofIteratorType;

    typedef DofArrayType DofStorageType;
    typedef MemObjectInterface MemObjectInterfaceType;

    //! type of this class
    typedef DFAdapt <DiscreteFunctionSpaceType> DiscreteFunctionType;
    //! LocalFunctionImp is the implementation
    typedef LocalFunctionAdapt < DiscreteFunctionSpaceType > LocalFunctionImp;

    //! LocalFunctionType is the exported lf type
    typedef LocalFunctionWrapper < DiscreteFunctionType > LocalFunctionType;

    typedef DiscreteFunctionSpaceType FunctionSpaceType;
    typedef DFAdaptTraits<DiscreteFunctionSpaceType> Traits;

    /** \brief For ISTL-compatibility */
    typedef FieldVector<DofType,1> block_type;

    typedef LocalFunctionStorage< DiscreteFunctionType > LocalFunctionStorageType;

  public:

    //! Constructor make Discrete Function
    DFAdapt(DiscreteFunctionSpaceType& f);

    //! Constructor creating discrete functions with name name
    //! for given functions space f
    DFAdapt (std::string name, DiscreteFunctionSpaceType & f ) ;

    //! Constructor creating discrete functions with name name
    //! for given functions space f and using given double * as vector
    //! VectorPointerType should be of the underlying array pointer type
    template <class VectorPointerType>
    DFAdapt (std::string name, DiscreteFunctionSpaceType & f , VectorPointerType * vec ) ;

    //! Constructor make Discrete Function
    DFAdapt (const DFAdapt <DiscreteFunctionSpaceType> & df);

    //! delete stack of free local functions belonging to this discrete
    //! function
    ~DFAdapt ();

    DiscreteFunctionType & argument    () { return *this; }
    const DiscreteFunctionType & argument () const { return *this; }
    DiscreteFunctionType & destination () { return *this; }

    // ***********  Interface  *************************
    //! return empty object of a local fucntion
    //! old function, will be removed soon
    LocalFunctionType newLocalFunction () DUNE_DEPRECATED;

    //! return local function for given entity
    template <class EntityType>
    LocalFunctionType localFunction(const EntityType& en) const;

    //! update LocalFunction to given Entity en
    //! old function, will be removed soon
    template <class EntityType>
    void localFunction ( const EntityType &en, LocalFunctionType & lf) DUNE_DEPRECATED;

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
    void subtractLocal (EntityType &it,
                        const DFAdapt <DiscreteFunctionSpaceType> & g);

    //! \todo Please do me!
    template <class EntityType>
    void setLocal (EntityType &it, const RangeFieldType &scalar);

    //! print all dofs
    void print(std::ostream& s) const;

    //! write data of discrete function to file filename|timestep
    //! with xdr methods
    bool write_xdr(std::string filename);

    //! write data of discrete function to file filename|timestep
    //! with xdr methods
    bool read_xdr(std::string filename);

    //! write function data to file filename|timestep in ascii Format
    bool write_ascii(std::string filename);

    //! read function data from file filename|timestep in ascii Format
    bool read_ascii(std::string filename);

    //! write function data in pgm fromat file
    bool write_pgm(std::string filename);

    //! read function data from pgm fromat file
    bool read_pgm(std::string filename);

    //! return name of this discrete function
    std::string name () const { return name_; }

    //! return siz fo this discrete function
    int size() const { return dofVec_.size(); }

    //! return pointer to internal array for use of BLAS routines
    DofType * leakPointer () { return dofVec_.leakPointer();  };
    //! return pointer to internal array for use of BLAS routines
    const DofType * leakPointer () const { return dofVec_.leakPointer(); };

  private:
    //! return object pointer of type LocalFunctionImp
    LocalFunctionImp * newLocalFunctionObject () const;

    // name of this func
    std::string name_;

    // DofManager
    DofManager<GridType>& dm_;

    // MemObject that manages the memory for the dofs of this function
    std::pair<MemObjectInterface*, DofStorageType*> memPair_;

    //! array containing the dof of this function, see dofmanager.hh
    //! the array is stored within the mem object
    DofArrayType & dofVec_;

    // one local function
    LocalFunctionType localFunc_;

  }; // end class DFAdapt


  //**************************************************************************
  //
  //  --LocalFunctionAdapt
  //
  //! Implementation of the local functions
  //
  //**************************************************************************
  template < class DiscreteFunctionSpaceType>
  class LocalFunctionAdapt
    : public LocalFunctionDefault <DiscreteFunctionSpaceType ,
          LocalFunctionAdapt <DiscreteFunctionSpaceType>  >
  {
  public:
    typedef typename DiscreteFunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;
    typedef LocalFunctionAdapt<DiscreteFunctionSpaceType> MyType;
    typedef DFAdapt<DiscreteFunctionSpaceType> DiscFuncType;
    friend class LocalFunctionWrapper< DiscFuncType >;

    enum { dimrange = DiscreteFunctionSpaceType::DimRange };

    friend class DFAdapt <DiscreteFunctionSpaceType>;
    typedef typename DiscreteFunctionSpaceType::Traits::RangeFieldType RangeFieldType;
    typedef typename DiscreteFunctionSpaceType::Traits::DomainType DomainType;
    typedef typename DiscreteFunctionSpaceType::Traits::RangeType RangeType;
    typedef typename DiscreteFunctionSpaceType::Traits::JacobianRangeType JacobianRangeType;

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
    int numberOfDofs () const DUNE_DEPRECATED;

    //! return number of degrees of freedom
    int numDofs () const;

    //! sum over all local base functions
    template <class EntityType>
    void evaluate (EntityType &en, const DomainType & x, RangeType & ret) const ;

    template <class EntityType>
    void evaluateLocal(EntityType &en, const DomainType & x, RangeType & ret) const ;
    //! sum over all local base functions evaluated on given quadrature point
    template <class EntityType, class QuadratureType>
    void evaluate (EntityType &en, QuadratureType &quad, int quadPoint , RangeType & ret) const;

    //! sum over all local base functions evaluated on given quadrature point
    template <class EntityType, class QuadratureType>
    void jacobian (EntityType &en, QuadratureType &quad, int quadPoint , JacobianRangeType & ret) const;

    template <class EntityType>
    void jacobianLocal(EntityType& en, const DomainType& x, JacobianRangeType& ret) const ;

    template <class EntityType>
    void jacobian(EntityType& en, const DomainType& x, JacobianRangeType& ret) const;

    void assign(int numDof, const RangeType& dofs);

  protected:
    //! update local function for given Entity
    template <class EntityType >
    void init ( const EntityType &en ) const;

    //! Forbidden! Would wreak havoc
    LocalFunctionAdapt(const LocalFunctionAdapt&);
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

    //! the corresponding function space which provides the base function set
    const DiscreteFunctionSpaceType& fSpace_;

    //! Array holding pointers to the local dofs
    mutable Array < RangeFieldType * > values_;

    //! dofVec from all levels of the discrete function
    DofArrayType & dofVec_;

    //! the corresponding base function set
    //BaseFunctionSetType* baseFuncSet_;

    //! do we have the same base function set for all elements
    bool uniform_;

    //! is it initialised?
    mutable bool init_;
  }; // end LocalFunctionAdapt

} // end namespace Dune

#include "discfuncarray/dfadapt.cc"

#endif
