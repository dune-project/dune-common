// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ADAPTIVEFUNCTIONIMP_HH
#define DUNE_ADAPTIVEFUNCTIONIMP_HH

//- System includes
#include <string>

//- Dune includes

namespace Dune {

  //- Forward declarations
  template <class DiscreteFunctionSpaceImp>
  class AdaptiveDiscreteFunction;
  template <class DiscreteFunctionSpaceImp>
  class AdaptiveLocalFunction;
  template <class DiscreteFunctionSpaceImp>
  class AdaptiveDiscreteFunctionTraits;
  template <class DiscreteFunctionSpaceImp>
  class AdaptiveLocalFunctionTraits;


  template <class DiscreteFunctionSpaceImp>
  class AdaptiveFunctionImplementation {
  public:
    typedef DiscreteFunctionSpaceImp DiscreteFunctionSpaceType;

    typedef AdaptiveDiscreteFunctionTraits<DiscreteFunctionSpaceImp> Traits;
  private:
    typedef AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp> ThisType;

    typedef typename Traits::DofIteratorType DofIteratorType;
    typedef typename Traits::ConstDofIteratorType ConstDofIteratorType;
  public:
    typedef typename Traits::LocalFunctionImp LocalFunctionImp;
    typedef typename Traits::LocalFunctionType LocalFunctionType;
  private:
    typedef typename Traits::MapperType MapperType;

    typedef typename DiscreteFunctionSpaceImp::Traits SpaceTraits;
    typedef typename SpaceTraits::RangeFieldType RangeFieldType;

    typedef typename Traits::DofStorageType DofStorageType;
    typedef typename Traits::GridType GridType;
    typedef DofManager<GridType> DofManagerType;

    typedef typename Traits::DiscreteFunctionType LeafType;

    typedef typename Traits::DofType DofType;
  public:
    std::string name() const;
    int size() const;

    DofIteratorType dbegin();
    DofIteratorType dend();
    ConstDofIteratorType dbegin() const;
    ConstDofIteratorType dend() const;

    LocalFunctionType newLocalFunction() DUNE_DEPRECATED;

    //! update local function to given entity
    template <class EntityType>
    void localFunction(const EntityType& en, LocalFunctionType& lf) DUNE_DEPRECATED;

    template <class EntityType>
    LocalFunctionType localFunction(const EntityType& en) const;

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

    //! return pointer to local function implementation
    LocalFunctionImp * newLocalFunctionObject () const;

    //! return pointer to underlying array
    DofType       * leakPointer ()       { return dofVec_.leakPointer(); }
    //! return pointer to underlying array
    const DofType * leakPointer () const { return dofVec_.leakPointer(); }
  protected:
    const DiscreteFunctionSpaceType& space() const { return spc_; }
    DofStorageType& dofStorage() { return dofVec_; }

    AdaptiveFunctionImplementation(std::string name,
                                   const DiscreteFunctionSpaceType& spc);

    // Constructor getting vector from outside
    template <class VectorPointerType>
    AdaptiveFunctionImplementation(std::string name,
                                   const DiscreteFunctionSpaceType& spc,
                                   VectorPointerType * vector);

    AdaptiveFunctionImplementation(std::string name,
                                   const DiscreteFunctionSpaceType& spc,
                                   DofStorageType& dofVec);
    AdaptiveFunctionImplementation(const ThisType& other);
    virtual ~AdaptiveFunctionImplementation();

  private:
    virtual const LeafType& interface() const = 0;
    const DiscreteFunctionSpaceType& spc_;
    std::string name_;
    DofManagerType& dm_;
    std::pair<MemObjectInterface*, DofStorageType*> memPair_;
    DofStorageType& dofVec_;
  }; // end class AdaptiveFunctionImplementation

} // end namespace Dune

#include "adaptiveimp.cc"

#endif
