// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE__PWCONSTFUNC_H__
#define __DUNE__PWCONSTFUNC_H__

#include "discretefunctionspace.hh"

namespace Dune {

  template<class FunctionSpaceType>
  class ConstBaseFunction : public BaseFunctionInterface<FunctionSpaceType> {
  public:

    ConstBaseFunction ( FunctionSpaceType & f )
      : BaseFunctionInterface<FunctionSpaceType>(f) {} ;

    virtual void evaluate ( const Vec<0,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      phi = 1.0 ;
    }

    virtual void evaluate ( const Vec<1,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      phi = 0.0 ;
    }

    virtual void evaluate ( const Vec<2,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      phi = 0.0 ;
    }

  private:
  };


  template<class FunctionSpaceType>
  class ConstFastBaseFunctionSet
    : public FastBaseFunctionSet<FunctionSpaceType >
  {
    //! know the number of base functions
    enum { numOfBaseFct = 1 };

  public:

    //! Constructor, calls Constructor of FastBaseFunctionSet, which is the
    //! InterfaceImplementation
    ConstFastBaseFunctionSet( FunctionSpaceType &fuspace )
      :  FastBaseFunctionSet<FunctionSpaceType >
          ( fuspace, numOfBaseFct ), baseFunc_(fuspace)
    {
      for(int i=0; i<numOfBaseFct; i++)
        setBaseFunctionPointer ( i , &baseFunc_ );
    } ;

    int getNumberOfBaseFunctions() const { return 1; }

  protected:

    ConstBaseFunction<FunctionSpaceType> baseFunc_;
  };

  template< typename DomainFieldType, typename RangeFieldType, int n, int m, class GridType >
  class ConstDiscreteFunctionSpace
    : public DiscreteFunctionSpace < DomainFieldType, RangeFieldType, n, m, GridType,
          ConstDiscreteFunctionSpace <DomainFieldType,RangeFieldType,n,m,GridType> > {

    typedef DiscreteFunctionSpace < DomainFieldType, RangeFieldType, n, m, GridType,
        ConstDiscreteFunctionSpace <DomainFieldType,RangeFieldType,n,m,GridType> >
    DiscreteFunctionSpaceType;

    typedef ConstDiscreteFunctionSpace <DomainFieldType,RangeFieldType,n,m,GridType> ConstDiscreteFunctionSpaceType;
    typedef ConstFastBaseFunctionSet < ConstDiscreteFunctionSpaceType > ConstFastBaseFunctionSetType;

    static const IdentifierType id = 1;

  public:
    typedef BaseFunctionSetInterface < ConstDiscreteFunctionSpaceType ,
        FastBaseFunctionSet <ConstDiscreteFunctionSpaceType >  > BaseFunctionSetType;

    ConstDiscreteFunctionSpace ( GridType & g ) : DiscreteFunctionSpaceType (g,id),baseFuncSet_(*this) {};

    template <class EntityType>
    const BaseFunctionSetType & getBaseFunctionSet ( EntityType &en ) const
    {
      return baseFuncSet_;
    };

  protected:

    ConstFastBaseFunctionSetType baseFuncSet_;

  };

  template < class DiscreteFunctionType, class EntityType>
  class ConstLocalFunction : public LocalFunction<DiscreteFunctionType, EntityType,
                                 ConstLocalFunction<DiscreteFunctionType, EntityType> >
  {
    enum { id = 2 };
  public:

    ConstLocalFunction ( DiscreteFunctionType & dfunct ) :
      LocalFunction( id, dfunct ) {} ;

    int numberOfDofs() const { return 1; }

    RangeField & operator [] ( int number ) ;

    Vec<2,int> mapToGlobal(int localDofNum ) const;

    void init ( EntityType &en );

  protected:
    DiscreteFunctionType::BaseFunctionSetType;

  };


}
#endif
