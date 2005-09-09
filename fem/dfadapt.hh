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
    typedef LocalFunctionAdapt<DiscreteFunctionSpaceImp> LocalFunctionType;
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

    typedef typename DiscreteFunctionSpaceType::GridType GridType;

    typedef DofManager<GridType> DofManagerType;
    typedef DofManagerFactory<DofManagerType> DofManagerFactoryType;

    enum { myId_ = 0};

  public:
    typedef typename DiscreteFunctionSpaceType::MapperType MapperType;
    typedef typename DiscreteFunctionSpaceType::RangeFieldType DofType;
    typedef typename DofArrayType::DofIteratorType DofIteratorType;
    typedef typename DofArrayType::ConstDofIteratorType ConstDofIteratorType;

    typedef DofArrayType DofStorageType;
    typedef MemObjectInterface MemObjectInterfaceType;

    typedef DFAdapt <DiscreteFunctionSpaceType> DiscreteFunctionType;
    typedef LocalFunctionAdapt < DiscreteFunctionSpaceType> LocalFunctionType;

    typedef DiscreteFunctionSpaceType FunctionSpaceType;
    typedef DFAdaptTraits<DiscreteFunctionSpaceType> Traits;

    /** \brief For ISTL-compatibility */
    typedef FieldVector<DofType,1> block_type;

  public:

    //! Constructor make Discrete Function
    DFAdapt(DiscreteFunctionSpaceType& f);

    DFAdapt (std::string name, DiscreteFunctionSpaceType & f ) ;

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

    template <class EntityType>
    LocalFunctionAdapt<DiscreteFunctionSpaceType> localFunction(EntityType& en);

    //! update LocalFunction to given Entity en
    template <class EntityType>
    void localFunction ( const EntityType &en,
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
    void subtractLocal (EntityType &it,
                        const DFAdapt <DiscreteFunctionSpaceType> & g);

    //! \todo Please do me!
    template <class EntityType>
    void setLocal (EntityType &it, const RangeFieldType &scalar);

    //! print all dofs
    void print(std::ostream& s);

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
    // name of this func
    std::string name_;

    // DofManager
    DofManager<GridType>& dm_;

    // MemObject that manages the memory for the dofs of this function
    std::pair<MemObjectInterface*, DofStorageType*> memPair_;

    //! array containing the dof of this function, see dofmanager.hh
    //! the array is stored within the mem object
    DofArrayType & dofVec_;

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
  template < class DiscreteFunctionSpaceType>
  class LocalFunctionAdapt
    : public LocalFunctionDefault <DiscreteFunctionSpaceType ,
          LocalFunctionAdapt <DiscreteFunctionSpaceType>  >
  {
    typedef typename DiscreteFunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;
    typedef LocalFunctionAdapt<DiscreteFunctionSpaceType> MyType;
    typedef DFAdapt<DiscreteFunctionSpaceType> DiscFuncType;

    enum { dimrange = DiscreteFunctionSpaceType::DimRange };

    friend class DFAdapt <DiscreteFunctionSpaceType>;
    typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename DiscreteFunctionSpaceType::DomainType DomainType;
    typedef typename DiscreteFunctionSpaceType::RangeType RangeType;
    typedef typename DiscreteFunctionSpaceType::JacobianRangeType JacobianRangeType;

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

  protected:
    //! update local function for given Entity
    template <class EntityType > bool init ( const EntityType &en ) const;

    //! Forbidden! Would wreak havoc
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
    //BaseFunctionSetType* baseFuncSet_;

    //! do we have the same base function set for all elements
    bool uniform_;

    //! is it initialised?
    mutable bool init_;
  }; // end LocalFunctionAdapt

} // end namespace Dune

#include "discfuncarray/dfadapt.cc"

#endif
