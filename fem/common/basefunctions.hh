// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BASEFUNCTIONS_HH__
#define __DUNE_BASEFUNCTIONS_HH__

#include <dune/common/matvec.hh>
#include <dune/common/mapping.hh>

#include "quadrature.hh"

namespace Dune {

  /** @defgroup BaseFunctionSets Base Function Sets for various Elements

     The base functions are essential to describe a numerical solutions.
     Here the interface of base functions and the corresponding base
     function set is presented. The user always works with the base function
     set, where all diffrent base functions for on element type are known.

     @{
   */




  // just to make it easy to change
  typedef int deriType;

  // just for make changes easy
  template <int dim>
  struct DiffVariable
  {
    typedef Vec<dim, deriType> Type;
  };

  //*************************************************************************
  //
  //! BaseFunctionInterface is the interface to a base function.
  //! A base function can be evaluated on a point from the Domain and the
  //! outcome is a point from Range. The Types of Domain and Range are stored
  //! by typedefs in FunctionSpaceType which is the template parameter of
  //! BaseFunctionInterface.
  //
  //*************************************************************************
  template<class FunctionSpaceType>
  class BaseFunctionInterface
    : public Mapping< typename FunctionSpaceType::DomainField,
          typename FunctionSpaceType::RangeField,
          typename FunctionSpaceType::Domain, typename FunctionSpaceType::Range >
  {

  public:
    typedef typename FunctionSpaceType::Domain Domain;
    typedef typename FunctionSpaceType::Range Range;
    enum { DimDomain = FunctionSpaceType::DimDomain };
    enum { DimRange  = FunctionSpaceType::DimRange  };

    BaseFunctionInterface ( FunctionSpaceType & f ) : functionSpace_ (f) {} ;

    //! evaluate the function at Domain x, and store the value in Range Phi
    //! diffVariable stores information about which gradient is to be
    //! evaluated. In the derived classes the evaluate method are template
    //! methods with template parameter "length of Vec".
    //! Though the evaluate Methods can be spezialized for each
    //! differentiation order
    virtual void evaluate ( const Vec<0,deriType> &diffVariable,
                            const Domain & , Range &) const {}; // = 0 ?

    virtual void evaluate ( const Vec<1,deriType> &diffVariable,
                            const Domain & , Range &) const {}; // = 0 ?

    virtual void evaluate ( const Vec<2,deriType> &diffVariable,
                            const Domain & , Range &) const {}; // = 0 ?

  private:
    FunctionSpaceType & functionSpace_;

  };


  //****************************************************************************
  //
  //  --BaseFunctionSetInterface
  //
  //! Why has the BaseFunctionInterface class virtual methods?
  //!
  //! Because the we want to have different base functions in
  //! at the same time and we havent found a way to do this with
  //! Barton-Nackman. But there is a solution which dosent cost to much
  //! efficiency, for example in FastBaseFunctionSet all evaluations of the
  //! BaseFunction are cached in a vector, though the virtual method evaluate
  //! of the BaseFunctionImp has only to be called once for the same quadrature
  //! rule. If you change the quadrature rule then on the first call the
  //! evaluations are stored again.
  //! This method brings us flexebility and effeciency.
  //!
  //****************************************************************************
  template<class FunctionSpaceType, class BaseFunctionSetImp>
  class BaseFunctionSetInterface
  {
  public:

    typedef typename FunctionSpaceType::Domain Domain ;
    typedef typename FunctionSpaceType::Range Range ;
    typedef typename FunctionSpaceType::JacobianRange JacobianRange;
    typedef typename FunctionSpaceType::HessianRange HessianRange;

    enum { DimDomain = FunctionSpaceType::DimDomain };
    enum { DimRange  = FunctionSpaceType::DimRange  };

    //typedef  Quadrature < FunctionSpaceType > QuadratureType ;
    typedef  BaseFunctionInterface < FunctionSpaceType > BaseFunctionType;

    // to be declared in derived extension class:
    // void gradient ( int baseFunct, Domain & x,  GradientRange & phi ) ;
    //   void gradient ( int baseFunct, QuadratureType & quad, int quadPoint, GradientRange & phi ) ;

    // void hessian ( int baseFunct, Domain & x,  HessianRange & phi ) ;
    // void hessian ( int baseFunct, QuadratureType & quad, int quadPoint, HessianRange & phi ) ;
    // Range baseIntegral( int baseFunct1, int diffOrder1, int diffVariable1,
    //         int baseFunct2, int diffOrder2, int diffVariable2 );

    /*** Begin Interface ***/

    BaseFunctionSetInterface ( FunctionSpaceType & f ) : funcSpace_ ( f ) {};

    int getNumberOfBaseFunctions () const {
      return asImp().getNumberOfBaseFunctions();
    };

    template <int diffOrd>
    void evaluate ( int baseFunct, const Vec<diffOrd,deriType> &diffVariable, const
                    Domain & x, Range & phi ) const {
      std::cout << "BaseFunctionSetInterface::evaluate \n";
      asImp().evaluate( baseFunct, diffVariable, x, phi );
    }

    template <int diffOrd, class QuadratureType >
    void evaluate ( int baseFunct, const Vec<diffOrd,deriType> &diffVariable, QuadratureType & quad, int quadPoint, Range & phi ) const {
      asImp().evaluate( baseFunct, diffVariable, quad, quadPoint, phi );
    }
  protected:

    const BaseFunctionInterface<FunctionSpaceType> &getBaseFunction( int baseFunct ) const {
      std::cout << "Interface getBaseFunction \n";
      return asImp().getBaseFunction( baseFunct );
    }


  private:
    FunctionSpaceType & funcSpace_;

    //! Barton-Nackman trick
    BaseFunctionSetImp &asImp() { return static_cast<BaseFunctionSetImp&>(*this); }
    const BaseFunctionSetImp &asImp() const
    { return static_cast<const BaseFunctionSetImp&>(*this); }

  };

  //*************************************************************************
  //
  //  --BaseFunctionSetDefault
  //
  //! The BaseFunctionSetDefault class is the internal interface. Here some
  //! default behavoir is implemented which always can be overloaded by the
  //! implementation class, but not has to.
  //!
  //*************************************************************************
  template<class FunctionSpaceType, class BaseFunctionSetImp>
  class BaseFunctionSetDefault
    : public BaseFunctionSetInterface < FunctionSpaceType , BaseFunctionSetImp>
  {
    enum { dimRow = JacobianRange::dimRow };
    enum { dimCol = JacobianRange::dimCol };

  public:
    typedef typename FunctionSpaceType::Domain Domain ;
    typedef typename FunctionSpaceType::Range Range ;
    typedef typename FunctionSpaceType::JacobianRange JacobianRange;
    typedef typename FunctionSpaceType::HessianRange HessianRange;
    //! set the default diffVar Types
    BaseFunctionSetDefault ( FunctionSpaceType & f ) :
      BaseFunctionSetInterface < FunctionSpaceType , BaseFunctionSetImp> (f)
    {
      for(int i=0; i<dimCol; i++)
        jacobianDiffVar_[i] = i;
    };

    //! default evaluate using the evaluate interface
    void eval ( int baseFunct, const Domain & x, Range & phi ) const
    {
      asImp().evaluate(baseFunct, diffVariable_ , x , phi);
      return;
    }

    //! default implementation for evaluation
    template <class QuadratureType>
    void eval ( int baseFunct, QuadratureType & quad, int quadPoint, Range & phi ) const
    {
      asImp().evaluate( baseFunct, diffVariable_ , quad, quadPoint, phi );
      return;
    }

    //! default evaluate using the evaluate interface
    void jacobian ( int baseFunct, const Domain & x, JacobianRange & phi ) const
    {
      Range tmp;
      for(int i=0; i<dimCol; i++)
      {
        asImp().evaluate( baseFunct, jacobianDiffVar_[i] , x , tmp );
        for(int j=0; j<dimRow; j++)
          phi(i,j) = tmp(j);
      }
      return;
    }

    //! default implementation of evaluation the gradient
    template <class QuadratureType>
    void jacobian ( int baseFunct, QuadratureType & quad,
                    int quadPoint, JacobianRange & phi ) const
    {
      Range tmp;
      for(int i=0; i<dimCol; i++)
      {
        asImp().evaluate( baseFunct, jacobianDiffVar_[i] , quad, quadPoint, tmp );
        for(int j=0; j<dimRow; j++)
          phi(i,j) = tmp(j);
      }
      return;
    }

  private:
    //! just diffVariable for evaluation of the functions
    const Vec<0,deriType> diffVariable_;

    Vec<1,deriType> jacobianDiffVar_[dimCol];

    //! Barton-Nackman trick
    BaseFunctionSetImp &asImp() { return static_cast<BaseFunctionSetImp&>(*this); }
    const BaseFunctionSetImp &asImp() const
    { return static_cast<const BaseFunctionSetImp&>(*this); }

  };
  /** @} end documentation group */

}

#endif
