// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BASEFUNCTIONS_HH__
#define __DUNE_BASEFUNCTIONS_HH__

#include "../common/matvec.hh"
#include "quadrature.hh"

namespace Dune {
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
  class BaseFunctionInterface : Mapping < typename FunctionSpaceType::RangeField, typename FunctionSpaceType::Domain, typename FunctionSpaceType::Range > {

  public:
    typedef typename FunctionSpaceType::Domain Domain ;
    typedef typename FunctionSpaceType::Range Range ;
    enum { DimDomain = FunctionSpaceType::DimDomain };
    enum { DimRange  = FunctionSpaceType::DimRange  };

    BaseFunctionInterface ( FunctionSpaceType & f ) : functionSpace_ (f) {} ;

    //! evaluate the function at Domain x, and store the value in Range Phi
    //! diffVariable stores information about which gradient is to be
    //! evaluated. In the derived classes the evaluate method are template
    //! methods with template parameter "length of Vec".
    //! Though the evaluate Methods can be spezialized for each
    //! differentiation order
    virtual void evaluate ( const Vec<0,char> &diffVariable,
                            const Domain & , Range &) const {}; // = 0 ?

    virtual void evaluate ( const Vec<1,char> &diffVariable,
                            const Domain & , Range &) const {}; // = 0 ?

    virtual void evaluate ( const Vec<2,char> &diffVariable,
                            const Domain & , Range &) const {}; // = 0 ?

  private:

    FunctionSpaceType & functionSpace_;

  };


  //****************************************************************************
  //!
  //!
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

  template<class FunctionSpaceType, class BaseFunctionSetImp>
  class BaseFunctionSetInterface
  {
  public:

    typedef typename FunctionSpaceType::Domain Domain ;
    typedef typename FunctionSpaceType::Range Range ;
    typedef typename FunctionSpaceType::GradientRange GradientRange;
    typedef typename FunctionSpaceType::HessianRange HessianRange;

    enum { DimDomain = FunctionSpaceType::DimDomain };
    enum { DimRange  = FunctionSpaceType::DimRange  };

    typedef  Quadrature < FunctionSpaceType > QuadratureType ;
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
    void evaluate ( int baseFunct, const Vec<diffOrd,char> &diffVariable, const
                    Domain & x, Range & phi ) const {
      std::cout << "BaseFunctionSetInterface::evaluate \n";
      asImp().evaluate( baseFunct, diffVariable, x, phi );
    }

    template <int diffOrd>
    void evaluate ( int baseFunct, const Vec<diffOrd,char> &diffVariable, QuadratureType & quad, int quadPoint, Range & phi ) const {
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


}

#endif
