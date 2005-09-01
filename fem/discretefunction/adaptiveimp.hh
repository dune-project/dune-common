// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ADAPTIVEFUNCTIONIMP_HH
#define DUNE_ADAPTIVEFUNCTIONIMP_HH

//- System includes
#include <string>

//- Dune includes

namespace Dune {

  //- Forward declarations
  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  class AdaptiveDiscreteFunction;
  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  class AdaptiveLocalFunction;
  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  class AdaptiveDiscreteFunctionTraits;
  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  class AdaptiveLocalFunctionTraits;


  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  class AdaptiveFunctionImplementation {
  private:
    typedef DiscreteFunctionSpaceImp DiscreteFunctionSpaceType;
    typedef DofManagerImp DofManagerType;

    typedef AdaptiveFunctionImplementation<
        DiscreteFunctionSpaceImp, DofManagerImp> ThisType;
    typedef AdaptiveDiscreteFunctionTraits<
        DiscreteFunctionSpaceImp, DofManagerImp> Traits;
    typedef typename Traits::DofIteratorType DofIteratorType;
    typedef typename Traits::ConstDofIteratorType ConstDofIteratorType;
    typedef typename Traits::LocalFunctionType LocalFunctionType;

    typedef typename DiscreteFunctionSpaceImp::Traits SpaceTraits;
    typedef typename SpaceTraits::RangeFieldType RangeFieldType;

    typedef typename Traits::DofIteratorType DofIteratorType;
    typedef typename Traits::ConstDofIteratorType ConstDofIteratorType;
    typedef typename Traits::MapperType MapperType;

    typedef typename Traits::DofStorageType DofStorageType;
    typedef typename Traits::MemObjectType MemObjectType;
  public:
    std::string name() const;

    DofIteratorType dbegin();
    DofIteratorType dend();
    ConstDofIteratorType dbegin() const;
    ConstDofIteratorType dend() const;

    LocalFunctionType newLocalFunction();
    template <class EntityType>
    void localFunction(const EntityType& en, LocalFunctionType& lf);

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

  protected:
    const DiscreteFunctionSpaceType& space() const { return spc_; }

    AdaptiveFunctionImplementation(std::string name,
                                   const DiscreteFunctionSpaceType& spc);
    AdaptiveFunctionImplementation(const ThisType& other);
    ~AdaptiveFunctionImplementation();

  private:
    const DiscreteFunctionSpaceType& spc_;
    std::string name_;
    DofManagerType& dm_;
    MemObjectType& memObj_;
    DofStorageType& dofVec_;
  };  // end class AdaptiveFunctionImplementation

} // end namespace Dune

#include "adaptiveimp.cc"

#endif
