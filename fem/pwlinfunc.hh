// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE__PWLINFUNC_H__
#define __DUNE__PWLINFUNC_H__

#include "fastbase.hh"
#include "discretefunctionspace.hh"


namespace Dune {

  template<class FunctionSpaceType>
  class LinBaseFunction_phi0 : public BaseFunctionInterface<FunctionSpaceType> {
  public:

    LinBaseFunction_phi0 ( FunctionSpaceType & f )
      : BaseFunctionInterface<FunctionSpaceType>(f) {} ;

    virtual void evaluate ( const Vec<0,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // phi0 = x Lagrange base on triangles
      // to be done correctly next time
      std::cout << "Evaluate phi0 \n";
      phi = x.read(0);
    }

    virtual void evaluate ( const Vec<1,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // 1 falls diffVariable = x, 0 sonst
      phi = 0.0 ;
    }

    virtual void evaluate ( const Vec<2,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // function is linear, therfore
      phi = 0.0 ;
    }

  };

  template<class FunctionSpaceType>
  class LinBaseFunction_phi1 : public BaseFunctionInterface<FunctionSpaceType>
  {
  public:

    LinBaseFunction_phi1 ( FunctionSpaceType & f )
      : BaseFunctionInterface<FunctionSpaceType>(f) {} ;

    virtual void evaluate ( const Vec<0,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // phi0 = x Lagrange base on triangles
      // to be done correctly next time
      std::cout << "Evaluate phi1 \n";
      phi = x.read(1);
    }

    virtual void evaluate ( const Vec<1,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // 1 falls diffVariable = x, 0 sonst
      phi = 0.0 ;
    }

    virtual void evaluate ( const Vec<2,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // function is linear, therfore
      phi = 0.0 ;
    }

  };

  template<class FunctionSpaceType>
  class LinBaseFunction_phi2 : public BaseFunctionInterface<FunctionSpaceType>
  {
  public:

    LinBaseFunction_phi2 ( FunctionSpaceType & f )
      : BaseFunctionInterface<FunctionSpaceType>(f) {} ;

    virtual void evaluate ( const Vec<0,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // phi0 = x Lagrange base on triangles
      // to be done correctly next time
      std::cout << "Evaluate phi2 \n";
      phi = 1.0-x.read(0)-x.read(1);
    }

    virtual void evaluate ( const Vec<1,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // 1 falls diffVariable = x, 0 sonst
      phi = 0.0 ;
    }

    virtual void evaluate ( const Vec<2,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // function is linear, therfore
      phi = 0.0 ;
    }

  };



  //*********************************************************************
  //
  //  --LinFastBaseFunctionSet
  //
  //*********************************************************************
  template<class FunctionSpaceType>
  class LinFastBaseFunctionSet
    : public FastBaseFunctionSet<FunctionSpaceType >
  {
    //! know the number of base functions
    enum { numOfBaseFct = 3 };

  public:

    //! Constructor, calls Constructor of FastBaseFunctionSet, which is the
    //! InterfaceImplementation
    LinFastBaseFunctionSet( FunctionSpaceType &fuSpace )
      :  FastBaseFunctionSet<FunctionSpaceType >
          ( fuSpace, numOfBaseFct ),
        phi0_ ( fuSpace ),
        phi1_ ( fuSpace ),
        phi2_ ( fuSpace )
    {
      std::cout << "Constructor of LinFastBaseFunctionSet \n";
      int i=0;
      setBaseFunctionPointer ( i , &phi0_ );

      i++;
      setBaseFunctionPointer ( i , &phi1_ );

      i++;
      setBaseFunctionPointer ( i , &phi2_ );
    };

    int getNumberOfBaseFunctions() const { return numOfBaseFct; }

  protected:
    LinBaseFunction_phi0 <FunctionSpaceType> phi0_;
    LinBaseFunction_phi1 <FunctionSpaceType> phi1_;
    LinBaseFunction_phi2 <FunctionSpaceType> phi2_;
  };

  //************************************************************************
  //
  //  --LagrangeMapper
  //
  //! This Class knows what the space dimension is and how to map for a
  //! given grid entity from local dof number to global dof number
  //
  //************************************************************************
  template <int polOrd>
  class LagrangeMapper
    : public MapperDefault < LagrangeMapper <polOrd> >
  {
  public:

    //! default is Lagrange with polOrd = 1
    template <class GridType>
    int size (GridType &grid , int level ) const
    {
      // return number of vertices
      return grid.size( level , GridType::dimension );
    };

    //! map Entity an local Dof number to global Dof number
    //! for Lagrange with polOrd = 1
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const
    {
      enum { codim = EntityType::dimension };
      // return vertex number
      return (*en.entity<codim>( localNum )).index();
    };

  };

  //****************************************************************
  //
  //  --LinDiscreteFunctionSpace
  //
  //! Provides access to base function set and size of functionspace
  //! and map from local to global dof number
  //
  //****************************************************************
  template< typename DomainFieldType, typename RangeFieldType,
      int n, int m, class GridType >
  class LinDiscreteFunctionSpace
    : public DiscreteFunctionSpace < DomainFieldType, RangeFieldType, n, m,
          GridType, LinDiscreteFunctionSpace <DomainFieldType,RangeFieldType,n,m,GridType> >
  {
    typedef DiscreteFunctionSpace < DomainFieldType, RangeFieldType, n, m, GridType,
        LinDiscreteFunctionSpace <DomainFieldType,RangeFieldType,n,m,GridType> >
    DiscreteFunctionSpaceType;

    typedef LinDiscreteFunctionSpace
    <DomainFieldType,RangeFieldType,n,m,GridType> LinDiscreteFunctionSpaceType;
    typedef LinFastBaseFunctionSet < LinDiscreteFunctionSpaceType > LinFastBaseFunctionSetType;
    typedef FastBaseFunctionSet < LinDiscreteFunctionSpaceType > FastBaseFunctionSetType;

    // at the moment just for fake
    static const IdentifierType id = 2;

    // Lagrange 1 , to be revised in this matter
  public:
    LinDiscreteFunctionSpace ( GridType & g ) :
      DiscreteFunctionSpaceType (g,id),baseFuncSet_(*this)  { };

    //! provide the access to the base function set
    template <class EntityType>
    const FastBaseFunctionSetType& getBaseFunctionSet ( EntityType &en ) const
    {
      return baseFuncSet_;
    };

    //! length of the dof vector
    //! size knows the correct way to calculate the size of the functionspace
    int size ( int level ) const
    {
      return mapper_.size ( grid_ ,level );
    };

    //! for given entity map local dof number to global dof number
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const
    {
      return mapper_.mapToGlobal ( en , localNum );
    };

  protected:
    //! the corresponding base set
    LinFastBaseFunctionSetType baseFuncSet_;

    //! the corresponding mapper
    LagrangeMapper<1> mapper_;

  }; // end class LinDiscreteFunctionSpace


} // end namespace Dune
#endif
