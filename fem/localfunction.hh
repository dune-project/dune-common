// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_LOCALFUNCTION_HH__
#define __DUNE_LOCALFUNCTION_HH__

namespace Dune {


  template < class DiscreteFunctionType, class EntityType,
      class LocalFunctionImp >
  class LocalFunction : public DynamicType {

    typedef BaseFunctionSet < DiscreteFunctionType::FunctionSpace > BaseFunctionSetType;
    typedef DiscreteFunctionType::FunctionSpace::Range Range;
    typedef DiscreteFunctionType::FunctionSpace::RangeField RangeField;

  public:

    LocalFunction ( int ident, DiscreteFunctionType & dfunct) :
      DynamicType (ident), discreteFunction_(dfunct) ;

    int numberOfDofs() const ;

    RangeField & operator [] ( int number ) ;

    Vec<2,int> mapToGlobal(int localDofNum ) const;

    void init ( EntityType & ) ;


  protected:
    DiscreteFunctionType & discreteFunction_ ;

  };


  template < class FunctionSpaceType , class GridType, class MapperImp, class
      LocalFunctionType>
  class Mapper {

    Mapper ( FunctionSpaceType & fuspace ) : functionSpace_ (fuspace) { } ;

    template <class EntityType>
    const LocalFunctionType &
    getLocalFunction ( EntityType &en ) const ;


  private:
    FunctionSpaceType & functionSpace_ ;


  };

}
#endif
