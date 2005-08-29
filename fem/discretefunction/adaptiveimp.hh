// NOTE: The current revision of this file was left untouched when the DUNE source files were reindented!
// NOTE: It contained invalid syntax that could not be processed by uncrustify.

#ifndef DUNE_ADAPTIVEFUNCTIONIMP_HH
#define DUNE_ADAPTIVEFUNCTIONIMP_HH

//- System includes
#include <string>

//- Dune includes

namespace Dune {

  template <class DiscreteFunctionSpaceImp>
  class AdaptiveFunctionImplementation {
  private:
    typedef AdaptiveDiscreteFunctionTraits<DiscreteFunctionSpaceImp> Traits;
    typedef typename Traits::DofIteratorType DofIteratorType;
    typedef typename Traits::ConstDofIteratorType ConstDofIteratorType;
    typedef typename Traits::LocalFunctionType LocalFunctionType

    typedef typename DiscreteFunctionSpaceImp::Traits SpaceTraits;
    typedef typename SpaceTraits::RangeFieldType RangeFieldType;

    typedef DofArray<RangeFieldType> DofStorageType;
  public:
    AdaptiveFunctionImplementation(

    DofIteratorType dbegin();
    DofIteratorType dend();
    ConstDofIteratorType dbegin() const;
    ConstDofIteratorType dend() const;
    
    LocalFunctionType newLocalFunction();
    template <class EntityType>
    void localFunction(const EntityType& en, LocalFunctionType& lf);
  
    //! write data of discrete function to file filename|timestep 
    //! with xdr methods 
    bool write_xdr(const std::string& filename);

    //! write data of discrete function to file filename|timestep 
    //! with xdr methods 
    bool read_xdr(const std::string& filename);
    
    //! write function data to file filename|timestep in ascii Format
    bool write_ascii(const std::string& filename);
    
    //! read function data from file filename|timestep in ascii Format
    bool read_ascii(const std::string& filename);

  protected:
    AdaptiveFunctionImplementation(std::string name);
    ~AdaptiveFunctionImplementation();

    DiscreteFunctionSpaceType& spc_;
    MemObjectType& memObj_;
    DofStorageType& dofVec_;
    std::string name_;
  }; // end class AdaptiveFunctionImplementation

} // end namespace Dune

#include "adaptiveimp.cc"

#endif
