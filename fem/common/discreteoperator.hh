// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETEOPERATOR_INTERFACE_HH__
#define __DUNE_DISCRETEOPERATOR_INTERFACE_HH__

#include <dune/common/operator.hh>

namespace Dune {


  // Note: Range has to have Vector structure as well.
  template <class DiscreteFunctionType, class LocalOperatorImp
      , class DiscreteOperatorImp  >
  class DiscreteOperatorInterface
    : public Operator <typename DiscreteFunctionType::RangeFieldType,
          DiscreteFunctionType , DiscreteFunctionType>
  {
  public:
    //! prepare operator for apply
    void prepare ( int level , Domain &Arg, Range &Dest,
                   Range *tmp , double a, double b)
    {
      asImp().prepare(level,Arg,Dest,tmp,a,b);
    }

    //! go over all Entitys and call the LocalOperator.applyLocal Method
    //! Note that the LocalOperator can be an combined Operator
    //! Domain and Range are defined through class Operator
    void apply ( Domain &Arg, Range &Dest )
    {
      asImp().apply( Arg, Dest);
    }

    //! finalize the operation
    double finalize (  Domain &Arg, Range &Dest )
    {
      return asImp().finalize( Arg, Dest );
    }

    //! apply the operator
    void operator()( Domain &Arg, Range &Dest )
    {
      apply(Arg,Dest);
    }

  private:
    //! Barton Nackman
    DiscreteOperatorImp & asImp()
    {
      return static_cast<DiscreteOperatorImp &> (*this);
    }

  };


  // Note: Range has to have Vector structure as well.
  template <class DiscreteFunctionType, class LocalOperatorImp
      , class DiscreteOperatorImp  >
  class DiscreteOperatorDefault
    : public DiscreteOperatorInterface < DiscreteFunctionType,
          LocalOperatorImp , DiscreteOperatorImp >
  {

  private:
    //! Barton Nackman
    DiscreteOperatorImp & asImp()
    {
      return static_cast<DiscreteOperatorImp &> (*this);
    }
  };

} // end namespace Dune

#endif
