// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_LOCALFUNCTION_HH__
#define __DUNE_LOCALFUNCTION_HH__

namespace Dune {

  //************************************************************************
  //!
  //! LocalFunction is the Interface for a local DofEntity which is on the
  //! dof point of view the equivalent to an grid entity. The
  //! LocalFunction provides the interface methods for a DofEntity. The dof
  //! Entities can be stored hierarchic like the grid or stored in a dof
  //! vector. This decicion is left to the programer and user.
  //!
  //
  //************************************************************************
  //template < class DiscreteFunctionType, class EntityType,
  //           class LocalFunctionImp >
  template < typename RangeField, class LocalFunctionImp>
  class LocalFunction : public DynamicType {

    //typedef BaseFunctionSet < DiscreteFunctionType::FunctionSpace > BaseFunctionSetType;
    //typedef DiscreteFunctionType::FunctionSpace::Range Range;
    //typedef DiscreteFunctionType::FunctionSpace::RangeField RangeField;

    //! Lenght of the tupel , i.e. level and element number
    enum { indexMax = 2 };

  public:
    typedef Vec<indexMax,int> MapTupel;

    //! Constructor
    LocalFunction ( int ident ) : //, DiscreteFunctionType & dfunct) :
                                 DynamicType (ident) {} ; // , discreteFunction_(dfunct) ;

    //! return number of local dofs
    int numberOfDofs() const ;

    //! return reference to local dof 'number'
    RangeField & operator [] ( int number ) ;

    //! returns level and element number for this localfunction
    MapTupel mapToGlobal(int localDofNum ) const;

    //! init localfunction with a given entity
    template <class EntityType >
    void init ( EntityType & ) ;

  protected:
    //  DiscreteFunctionType & discreteFunction_ ;

  };

#if 0
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
#endif


}
#endif
