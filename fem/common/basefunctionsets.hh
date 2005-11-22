// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BASEFUNCTIONSETS_HH
#define DUNE_BASEFUNCTIONSETS_HH

#include <vector>

#include <dune/fem/space/dofstorage.hh>
#include "basefunctions.hh"
#include "basefunctionfactory.hh"

namespace Dune {

  // * Factories shall not be stored! (If you want to do it, copy them)

  // Forward declarations
  template <class FunctionSpaceImp>
  class StandardBaseFunctionSet;
  template <class FunctionSpaceImp>
  class VectorialBaseFunctionSet;

  template <class FunctionSpaceImp>
  struct StandardBaseFunctionSetTraits
  {
    typedef FunctionSpaceImp FunctionSpaceType;
    typedef StandardBaseFunctionSet<FunctionSpaceType> BaseFunctionSetType;
  };

  template <class FunctionSpaceImp>
  class StandardBaseFunctionSet :
    public BaseFunctionSetDefault<StandardBaseFunctionSetTraits<FunctionSpaceImp> >
  {
  private:
    typedef FunctionSpaceImp FunctionSpaceType;
    typedef BaseFunctionFactory<FunctionSpaceType> FactoryType;
    typedef typename FactoryType::BaseFunctionType BaseFunctionType;
  public:
    typedef StandardBaseFunctionSetTraits<FunctionSpaceImp> Traits;

    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;

  public:
    StandardBaseFunctionSet(const FactoryType& factory) :
      baseFunctions_()
    {
      for (int i = 0; i < factory.numBaseFunctions(); ++i) {
        baseFunctions_.push_back(factory.baseFunction(i));
      }
    }

    ~StandardBaseFunctionSet() {
      for (size_t i = 0; i < baseFunctions_.size(); ++i) {
        delete baseFunctions_[i];
        baseFunctions_[i] = 0;
      }
    }

    int numBaseFunctions() const;

    template <int diffOrd>
    void evaluate(int baseFunct,
                  const FieldVector<deriType, diffOrd>& diffVar,
                  const DomainType& xLocal,
                  RangeType& phi) const;

    template <int diffOrd, class QuadratureType>
    void evaluate (int baseFunct,
                   const FieldVector<deriType, diffOrd> &diffVariable,
                   QuadratureType & quad,
                   int quadPoint, RangeType & phi ) const;

  private:
    std::vector<BaseFunctionType*> baseFunctions_;
  };

  template <class FunctionSpaceImp>
  struct VectorialBaseFunctionSetTraits
  {
    typedef FunctionSpaceImp FunctionSpaceType;
    typedef VectorialBaseFunctionSet<FunctionSpaceType> BaseFunctionSetType;
  };

  template <class FunctionSpaceImp>
  class VectorialBaseFunctionSet :
    public BaseFunctionSetDefault<VectorialBaseFunctionSetTraits<FunctionSpaceImp> >
  {
  private:
    typedef BaseFunctionSetDefault<
        VectorialBaseFunctionSetTraits<FunctionSpaceImp> > BaseType;
    typedef FunctionSpaceImp FunctionSpaceType;
    typedef typename ToScalarFunctionSpace<
        FunctionSpaceImp>::Type ScalarFunctionSpaceType;
    typedef typename ScalarFunctionSpaceType::RangeType ScalarRangeType;
    typedef typename ScalarFunctionSpaceType::JacobianRangeType
    ScalarJacobianRangeType;
    typedef BaseFunctionFactory<ScalarFunctionSpaceType> FactoryType;
    typedef typename FactoryType::BaseFunctionType BaseFunctionType;

  public:
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::JacobianRangeType JacobianRangeType;

    typedef typename FunctionSpaceType::RangeFieldType DofType;

  public:
    VectorialBaseFunctionSet(const FactoryType& factory) :
      baseFunctions_(),
      util_(FunctionSpaceType::DimRange),
      diffVar0_(),
      diffVar1_(0),
      tmp_(0.),
      jTmp_(0.)
    {
      for (int i = 0; i < factory.numBaseFunctions(); ++i) {
        baseFunctions_.push_back(factory.baseFunction(i));
      }
    }

    ~VectorialBaseFunctionSet()
    {
      for (size_t i = 0; i < baseFunctions_.size(); ++i) {
        delete baseFunctions_[i];
        baseFunctions_[i] = 0;
      }
    }

    // * override other functions as well!!!!

    int numBaseFunctions() const;

    template <int diffOrd>
    void evaluate(int baseFunct,
                  const FieldVector<int, diffOrd>& diffVar,
                  const DomainType& xLocal,
                  RangeType& phi) const;

    template <int diffOrd, class QuadratureType>
    void evaluate(int baseFunct,
                  const FieldVector<deriType, diffOrd> &diffVariable,
                  QuadratureType & quad,
                  int quadPoint, RangeType & phi ) const;

    // * add those methods with quadratures as well
    DofType evaluateSingle(int baseFunct,
                           const DomainType& xLocal,
                           const RangeType& factor) const;

    template <class Entity>
    DofType evaluateGradientSingle(int baseFunct,
                                   Entity& en,
                                   const DomainType& xLocal,
                                   const JacobianRangeType& factor) const;

  private:
    std::vector<BaseFunctionType*> baseFunctions_;
    DofConversionUtility<PointBased> util_;

    mutable FieldVector<int, 0> diffVar0_;
    mutable FieldVector<int, 1> diffVar1_;
    mutable ScalarRangeType tmp_;
    mutable DomainType jTmp_;
  };

} // end namespace Dune

#include "basefunctionsets.cc"

#endif
