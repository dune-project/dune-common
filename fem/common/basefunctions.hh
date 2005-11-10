// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BASEFUNCTIONS_HH
#define DUNE_BASEFUNCTIONS_HH

#include <dune/common/mapping.hh>

#include <dune/quadrature/common/quadrature.hh>

namespace Dune {

  /** @defgroup BaseFunction BaseFunction
     @ingroup DiscreteFunction

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
    typedef FieldVector<deriType, dim> Type;
  };

  //*************************************************************************
  //! BaseFunctionInterface is the interface to a base function.
  //! A base function can be evaluated on a point from the Domain and the
  //! outcome is a point from Range. The Types of Domain and Range are stored
  //! by typedefs in FunctionSpaceType which is the template parameter of
  //! BaseFunctionInterface.
  //*************************************************************************
  template<class FunctionSpaceImp>
  class BaseFunctionInterface
    : public Mapping< typename FunctionSpaceImp::DomainFieldType,
          typename FunctionSpaceImp::RangeFieldType,
          typename FunctionSpaceImp::DomainType,
          typename FunctionSpaceImp::RangeType >
  {

  public:
    typedef FunctionSpaceImp DiscreteFunctionSpaceType;
    typedef typename DiscreteFunctionSpaceType::DomainType DomainType;
    typedef typename DiscreteFunctionSpaceType::RangeType RangeType;
    enum { DimDomain = DiscreteFunctionSpaceType::DimDomain };
    enum { DimRange  = DiscreteFunctionSpaceType::DimRange  };

    BaseFunctionInterface () {}

    virtual ~BaseFunctionInterface() {}

    //! evaluate the function at Domain x, and store the value in Range Phi
    //! diffVariable stores information about which gradient is to be
    //! evaluated. In the derived classes the evaluate method are template
    //! methods with template parameter "length of Vec".
    //! Though the evaluate Methods can be spezialized for each
    //! differentiation order
    //! \param x The local coordinate in the reference element
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & , RangeType &) const = 0;

    //! diffVariable contain the component of the gradient which is delivered.
    //! for example gradient of the basefunction x component ==>
    //! diffVariable(0) == 0, y component ==> diffVariable(0) == 1 ...
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & , RangeType &) const = 0;

    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & , RangeType &) const = 0;

  };

  /** @} end documentation group */


  /** @defgroup BaseFunctionSet BaseFunctionSet
     @ingroup DiscreteFunction

     The base functions are essential to describe a numerical solutions.
     Here the interface of base functions and the corresponding base
     function set is presented. The user always works with the base function
     set, where all diffrent base functions for on element type are known.

     @{
   */

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
  template<class BaseFunctionSetTraits>
  class BaseFunctionSetInterface
  {
  public:
    typedef typename BaseFunctionSetTraits::DiscreteFunctionSpaceType DiscreteFunctionSpaceType;
    typedef typename BaseFunctionSetTraits::BaseFunctionSetType BaseFunctionSetType;
    typedef typename DiscreteFunctionSpaceType::DomainType DomainType;
    typedef typename DiscreteFunctionSpaceType::RangeType RangeType;
    typedef typename DiscreteFunctionSpaceType::JacobianRangeType JacobianRangeType;
    typedef typename DiscreteFunctionSpaceType::HessianRangeType HessianRangeType;

    enum { DimDomain = DiscreteFunctionSpaceType::DimDomain };
    enum { DimRange  = DiscreteFunctionSpaceType::DimRange  };

    //typedef  Quadrature < FunctionSpaceType > QuadratureType ;
    typedef  BaseFunctionInterface<DiscreteFunctionSpaceType> BaseFunctionType;

  public:

    //! \todo Please doc me!
    BaseFunctionSetInterface () {}

    virtual ~BaseFunctionSetInterface() {}

    //! Number of base functions
    int numBaseFunctions() const {
      // stay on the safe side and call the deprecated version for now
      return asImp().numBaseFunctions();
    }

    //! Number of really differing (scalar) base functions in the case of a
    //! vectorial problem
    int numDifferentBaseFunctions() const {
      return asImp().numDifferentBaseFunctions();
    }

    //! \todo Please doc me!
    template <int diffOrd>
    void evaluate (int baseFunct,
                   const FieldVector<deriType, diffOrd> &diffVariable,
                   const DomainType & x, RangeType & phi ) const {
      asImp().evaluate( baseFunct, diffVariable, x, phi );
    }

    //! \todo Please doc me!
    template <int diffOrd, class QuadratureType>
    void evaluate (int baseFunct,
                   const FieldVector<deriType, diffOrd> &diffVariable,
                   QuadratureType & quad,
                   int quadPoint, RangeType & phi ) const {
      asImp().evaluate( baseFunct, diffVariable, quad, quadPoint, phi );
    }
  protected:

    //! This function should not be here at all!
    const BaseFunctionInterface<DiscreteFunctionSpaceType> &getBaseFunction( int baseFunct ) const {
      std::cout << "Interface getBaseFunction \n";
      return asImp().getBaseFunction( baseFunct );
    }


  private:

    //! Barton-Nackman trick
    BaseFunctionSetType &asImp() {
      return static_cast<BaseFunctionSetType&>(*this);
    }

    const BaseFunctionSetType &asImp() const {
      return static_cast<const BaseFunctionSetType&>(*this);
    }

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
  template<class BaseFunctionSetTraits>
  class BaseFunctionSetDefault  :
    public BaseFunctionSetInterface <BaseFunctionSetTraits>
  {
  public:
    typedef typename BaseFunctionSetTraits::BaseFunctionSetType BaseFunctionSetType;
    typedef typename BaseFunctionSetTraits::DiscreteFunctionSpaceType DiscreteFunctionSpaceType;
    typedef typename DiscreteFunctionSpaceType::JacobianRangeType JacobianRangeType;
    enum { dimRow = JacobianRangeType::rows };
    enum { dimCol = JacobianRangeType::cols };

    typedef typename DiscreteFunctionSpaceType::DomainType DomainType ;
    typedef typename DiscreteFunctionSpaceType::RangeType RangeType ;
    typedef typename DiscreteFunctionSpaceType::HessianRangeType HessianRangeType;
    typedef typename DiscreteFunctionSpaceType::RangeFieldType DofType;
    typedef std::vector<DofType> DofVectorType;
  public:
    //! set the default diffVar Types
    BaseFunctionSetDefault () :
      BaseFunctionSetInterface<BaseFunctionSetTraits> ()
    {
      for(int i=0; i<dimCol; i++)
        jacobianDiffVar_[i] = i;
    };

    virtual ~BaseFunctionSetDefault() {}

    //! By default, assume that the number of different base functions are the
    //! same as the overall number of base functions
    int numDifferentBaseFunctions() const
    {
      return this->numBaseFunctions();
    }

    //! default evaluate using the evaluate interface
    void eval(int baseFunct, const DomainType & x, RangeType & phi) const
    {
      asImp().evaluate(baseFunct, diffVariable_ , x , phi);
      return;
    }

    //! default implementation for evaluation
    template <class QuadratureType>
    void eval(int baseFunct, QuadratureType & quad, int quadPoint, RangeType & phi) const
    {
      asImp().evaluate( baseFunct, diffVariable_ , quad, quadPoint, phi );
      return;
    }

    //! default implementation for scalar evaluation
    void evaluateScalar(int baseFunct, const DomainType& x, DofType& phi) const
    {
      asImp().evaluate(baseFunct, diffVariable_, x, tmp_);
      phi = tmp_[0];
    }

    //! default implementation for scalar evaluation
    template <class QuadratureType>
    void evaluateScalar(int baseFunct, QuadratureType& quad, int quadPoint, DofType& phi) const
    {
      asImp().evaluate(baseFunct, diffVariable_, quad, quadPoint, tmp_);
      phi = tmp_[0];
    }

    //! default evaluate using the evaluate interface
    void jacobian(int baseFunct, const DomainType & x, JacobianRangeType & phi) const
    {
      for(int i=0; i<dimCol; i++)
      {
        asImp().evaluate( baseFunct, jacobianDiffVar_[i] , x , tmp_ );
        for(int j=0; j<dimRow; j++)
          phi[j][i] = tmp_[j];
      }
    }

    //! default implementation of evaluation the gradient
    template <class QuadratureType>
    void jacobian ( int baseFunct, QuadratureType & quad,
                    int quadPoint, JacobianRangeType & phi ) const
    {
      for(int i=0; i<dimCol; i++)
      {
        asImp().evaluate( baseFunct, jacobianDiffVar_[i] , quad, quadPoint, tmp_ );
        for(int j=0; j<dimRow; j++)
          phi[j][i] = tmp_[j];
      }
    }

    // * add methods for quadrature type as well
    void evaluateSingle(int baseFunct,
                        const DomainType& xLocal,
                        const RangeType& factor,
                        DofType& result) const
    {
      RangeType phi(0.);
      eval(baseFunct, xLocal, phi);
      result = phi*factor;
    }

    void evaluateSet(const DomainType& xLocal,
                     const RangeType& factor,
                     DofVectorType& result) const
    {
      result.resize(this->numBaseFunctions());
      DofType tmp;
      for (size_t i = 0; i < this->numBaseFunctions(); ++i) {
        evaluateSingle(i, xLocal, factor, tmp);
        result[i] = tmp;
      }
    }

    void evaluateGradientSingle(int baseFunct,
                                const DomainType& xLocal,
                                const JacobianRangeType& factor,
                                DofType& result) const
    {
      JacobianRangeType gradPhi(0.);
      jacobian(baseFunct, xLocal, gradPhi);

      result = 0;
      for (int i = 0; i < DiscreteFunctionSpaceType::dimDomain; ++i) {
        result += gradPhi[i]*factor[i];
      }
    }

    void evaluateGradientSet(const DomainType& xLocal,
                             const JacobianRangeType& factor,
                             DofVectorType& result) const
    {
      result.resize(this->numBaseFunctions());
      DofType tmp;
      for (size_t i = 0; i < this->numBaseFunctions(); ++i) {
        evaluateGradientSingle(i, xLocal, factor, tmp);
        result[i] = tmp;
      }
    }

  private:
    //! just diffVariable for evaluation of the functions
    const FieldVector<deriType, 0> diffVariable_;

    //! temporary RangeType vec
    mutable RangeType tmp_;

    FieldVector<deriType, 1> jacobianDiffVar_[dimCol];

    //! Barton-Nackman trick
    BaseFunctionSetType &asImp() { return static_cast<BaseFunctionSetType&>(*this); }
    const BaseFunctionSetType &asImp() const
    { return static_cast<const BaseFunctionSetType&>(*this); }

  };
  /** @} end documentation group */

}

#endif
