// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE__PWBILINFUNC_H__
#define __DUNE__PWBILINFUNC_H__

#include "fastbase.hh"
#include "discretefunctionspace.hh"
#include "quadlqh.hh"


namespace Dune {

  //*********************************************************************
  //
  //! Bilinear BaseFunctions for quadrilaterals
  //! v(x,y) = (alpha + beta * x) * ( gamma + delta * y)
  //! see W. Hackbusch page 162
  //
  //*********************************************************************
  template<class FunctionSpaceType, int baseNum>
  class BiLinBaseFunc : public BaseFunctionInterface<FunctionSpaceType>
  {
    //! phi(x,y) = (alpha + beta * x) * ( gamma + delta * y)
    enum { alpha = ( baseNum%2 == 0 ) ?  1 : 0 };
    enum { beta  = ( baseNum%2 == 0 ) ? -1 : 1 };
    enum { gamma = ( baseNum < 2    ) ?  1 : 0 };
    enum { delta = ( baseNum < 2    ) ? -1 : 1 };
  public:

    BiLinBaseFunc ( FunctionSpaceType & f )
      : BaseFunctionInterface<FunctionSpaceType>(f) {} ;

    virtual void evaluate ( const Vec<0,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // supposse that phi is element R
      phi = (alpha + beta * x.read(0)) * ( gamma + delta * x.read(1));
    }

    //! derivative with respect to x or y
    //! diffVariable(0) == 0   ==> x
    //! diffVariable(0) == 1   ==> y
    //! diffVariable(0) == 2   ==> z,  and so on
    virtual void evaluate ( const Vec<1,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      if(diffVariable.read(0)) // differtiate to x component
      {
        phi = beta * ( gamma + delta * x.read(1));
        return;
      }
      else // differtiate to y component
      {
        phi = (alpha + beta * x.read(0)) * delta;
        return;
      }
    }

    virtual void evaluate ( const Vec<2,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // which means derivative xx or yy
      if(diffVariable.read(0) == diffVariable.read(1))
      {
        phi = 0.0;
        return;
      }
      // which means derivative xy or yx
      else
      {
        phi = beta * delta;
        return;
      }
    }

  };

  //*********************************************************************
  //
  //  --BiLinFastBaseFuncSet
  //
  //*********************************************************************
  template<class FunctionSpaceType>
  class BiLinFastBaseFuncSet
    : public FastBaseFunctionSet<FunctionSpaceType >
  {
    //! know the number of base functions
    enum { numOfBaseFct = 4 };

  public:

    //! Constructor, calls Constructor of FastBaseFunctionSet, which is the
    //! InterfaceImplementation
    BiLinFastBaseFuncSet( FunctionSpaceType &fuSpace )
      :  FastBaseFunctionSet<FunctionSpaceType >
          ( fuSpace, numOfBaseFct ),
        phi0_ ( fuSpace ),
        phi1_ ( fuSpace ),
        phi2_ ( fuSpace ),
        phi3_ ( fuSpace )
    {
      std::cout << "Constructor of BiLinFastBaseFuncSet \n";
      int i=0;
      setBaseFunctionPointer ( i , &phi0_ );

      i++;
      setBaseFunctionPointer ( i , &phi1_ );

      i++;
      setBaseFunctionPointer ( i , &phi2_ );

      i++;
      setBaseFunctionPointer ( i , &phi3_ );
    };

    int getNumberOfBaseFunctions() const { return numOfBaseFct; }

  protected:
    BiLinBaseFunc <FunctionSpaceType,0> phi0_;
    BiLinBaseFunc <FunctionSpaceType,1> phi1_;
    BiLinBaseFunc <FunctionSpaceType,2> phi2_;
    BiLinBaseFunc <FunctionSpaceType,3> phi3_;
  };

  //****************************************************************
  //
  //  --BiLinDiscFuSp
  //
  //! Provides access to base function set and size of functionspace
  //! and map from local to global dof number
  //
  //****************************************************************
  template< typename DomainFieldType, typename RangeFieldType,
      int n, int m, class GridType >
  class BiLinDiscFuSp
    : public DiscreteFunctionSpace < DomainFieldType, RangeFieldType, n, m,
          GridType, LinDiscreteFunctionSpace <DomainFieldType,RangeFieldType,n,m,GridType>,
          LinFastBaseFunctionSet < LinDiscreteFunctionSpace
              <DomainFieldType,RangeFieldType,n,m,GridType> >  >
  {
    typedef DiscreteFunctionSpace < DomainFieldType, RangeFieldType, n, m, GridType,
        LinDiscreteFunctionSpace <DomainFieldType,RangeFieldType,n,m,GridType>, LinFastBaseFunctionSet < LinDiscreteFunctionSpace
            <DomainFieldType,RangeFieldType,n,m,GridType> > >
    DiscreteFunctionSpaceType;

    typedef BiLinDiscFuSp
    <DomainFieldType,RangeFieldType,n,m,GridType> BiLinDiscreteFSType;
    typedef BiLinFastBaseFuncSet < BiLinDiscreteFSType > BiLinFastBaseFunctionSetType;
    typedef FastBaseFunctionSet < BiLinDiscreteFSType > FastBaseFunctionSetType;

    // at the moment just for fake
    static const IdentifierType id = 2;

    // Lagrange 1 , to be revised in this matter
  public:
    BiLinDiscFuSp ( GridType & g ) :
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
    BiLinFastBaseFunctionSetType baseFuncSet_;

    //! the corresponding mapper
    LagrangeMapper<1> mapper_;

  }; // end class LinDiscreteFunctionSpace

} // end namespace Dune
#endif
