// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMBINEDSPACE_HH
#define DUNE_COMBINEDSPACE_HH

//- System includes
#include <vector>

//- Dune includes
#include <dune/fem/common/discretefunctionspace.hh>
#include <dune/fem/common/basefunctions.hh>
#include <dune/fem/common/dofmapperinterface.hh>

namespace Dune {

  // Forward declarations
  template <class DiscreteFunctionSpaceImp, int N>
  class CombinedSpaceange;
  template <class DiscreteFunctionSpaceImp, int N>
  class CombinedMapper;
  //template <class BaseFunctionImp, int N>
  //class CombinedBaseFunction;
  template <class BaseFunctionSetImp, int N>
  class CombinedBaseFunctionSet;

  template <class DiscreteFunctionSpaceImp, int N>
  struct CombinedSpaceTraits {
  private:
    typedef DiscreteFunctionSpaceImp ContainedDiscreteFunctionSpaceType;
    typedef ContainedDiscreteFunctionSpaceType::MapperType ContainedMapperType;
    typedef typename ContainedDiscreteFunctionSpaceType::Traits ContainedTraits;
    typedef typename ContainedTraits::FunctionSpaceType ContainedFunctionSpaceType;
    typedef typename ContainedTraits::BaseFunctionSetType ContainedBaseFunctionSetType;

    enum { ContainedDimRange = ContainedFunctionSpace::DimRange,
           ContainedDimDomain = ContainedFunctionSpace::DimDomain };
  public:
    typedef typename ContainedFunctionSpace::DomainFieldType DomainFieldType;
    typedef typename ContainedFunctionSpace::RangeFieldType RangeFieldType;

    // * Require contained space to have dimRange == 1? (ie to be scalar)
    typedef FunctionSpace<
        DomainFieldType, RangeFieldType,
        ContainedDimDomain, ContainedDimRange*N
        > FunctionSpaceType;
    typedef CombinedSpace<
        DiscreteFunctionSpaceImp, N
        > DiscreteFunctionSpaceType;
    typedef typename ContainedTraits::GridType GridType;
    typedef typename ContainedTraits::IteratorType IteratorType;
    typedef CombinedBaseFunctionSet<ContainedBaseFunctionSetType, N> BaseFunctionSetType;
  };

  template <class DiscreteFunctionSpaceImp, int N>
  class CombinedSpace :
    public DiscreteFunctionSpaceDefault<
        CombinedSpaceTraits<DiscreteFunctionSpaceImp, N>
        >
  {
  public:
    //- Public typedefs and enums
    typedef CombinedSpaceTraits<DiscreteFunctionSpaceImp, N> Traits;
    typedef DiscreteFunctionSpaceImp ContainedDiscreteFunctionSpaceType;

    typedef typename Traits::IteratorType IteratorType;

  public:
    //- Public methods
    CombinedSpace(ContainedDiscreteFunctionSpaceType& spc);

    //! begin iterator
    IteratorType begin() const { return spc_.begin(); }

    //! end iterator
    IteratorType end() const { return spc_.end(); }

    //! total number of dofs
    int size() const { return mapper_.size(); }

    //! map a local dof number to a global one
    template <class EntityType>
    int mapToGlobal(EntityType& en, int local) const {
      mapper_.mapToGlobal(en, localNum);
    }

    template <class EntityType>
    const BaseFunctionSetType& getBaseFunctionSet(EntityType& en) const {
      // * More to come
      return ? ?;
    }

    GridType& grid() { return spc_.grid(); }
  private:
    //- Private typedefs

  private:
    //- Private methods

  private:
    //- Member data
    DiscreteFunctionSpaceType& spc_;

    MapperType mapper_;
    std::vector<BaseFunctionSetType*> baseSetVec_;

  }; // end class CombinedSpace

  /* Probably not needed
     template <class BaseFunctionImp, int N>
     class CombinedBaseFunction :
     public BaseFunctionInterface<
     CombinedSpace<typename BaseFunctionImp::FunctionSpaceType, N>
     >
     {
     public:
     //- Typedefs and enums
     typedef CombinedSpace<typename BaseFunctionImp::FunctionSpaceType, N> DisreteFunctionSpaceType;
     typedef BaseFunctionImp ContainedBaseFunctionType;
     typedef ... DomainType;
     typedef ... RangeType;

     public:
     //- Public methods
     CombinedBaseFunction(const ContainedBaseFunctionType& bf) :
      containedBaseFunction_(bf) {}

     //! evaluate the function at DomainType x, and store the value in Range Phi
     //! diffVariable stores information about which gradient is to be
     //! evaluated. In the derived classes the evaluate method are template
     //! methods with template parameter "length of Vec".
     //! Though the evaluate Methods can be spezialized for each
     //! differentiation order
     //! \param x The local coordinate in the reference element
     virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                          const DomainType& x, RangeType& result);

     //! diffVariable contain the component of the gradient which is delivered.
     //! for example gradient of the basefunction x component ==>
     //! diffVariable(0) == 0, y component ==> diffVariable(0) == 1 ...
     virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                          const DomainType& x, RangeType& result) const;

     virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                          const DomainType& x, RangeType& result) const;

     private:
     //- Private typedefs
     typedef ... ContainedRangeType;

     private:
     //- Private methods
     void expand(const ContainedRangeType& arg, RangeType& result) const;

     private:
     //- Data members
     const ContainedBaseFunctionType& containedBaseFunction_;
     };
   */

  template <class BaseFunctionSetImp, int N>
  struct CombinedBaseFunctionSetTraits {
  private:
    typedef typename BaseFunctionSetImp::DiscreteFunctionSpaceType ContainedFunctionSpaceType;
  public:
    typedef CombinedBaseFunctionSet<BaseFunctionSetImp, N> BaseFunctionSetType;
    typedef CombinedSpace<ContainedFunctionSpaceType, N> DiscreteFunctionSpaceType;
    typedef DiscreteFunctionSpaceType::RangeType RangeType;
    typedef DiscreteFunctionSpaceType::DomainType DomainType;
    typedef typename DiscreteFunctionSpaceType::JacobianRangeType JacobianRangeType;
    typedef typename DiscreteFunctionSpaceType::HessianRangeType HessianRangeType;

    enum { DimDomain = DiscreteFunctionSpaceType::DimDomain };
    enum { DimRange  = DiscreteFunctionSpaceType::DimRange  };
  };

  template <class BaseFunctionSetImp, int N>
  class CombinedBaseFunctionSet :
    public BaseFunctionSetDefault<
        CombinedBaseFunctionSetTraits<BaseFunctionSetImp, int N>
        >
  {
  private:
    //- Private typedefs
    typedef BaseFunctionSetImp ContainedBaseFunctionSetType;
    typedef ContainedBaseFunctionSetType::RangeType ContainedRangeType;
  public:
    //- Typedefs and enums
    typedef CombinedBaseFunctionSetTraits<BaseFunctionSetImp, N> Traits;
    typedef typename Traits::DiscreteFunctionSpaceType DiscreteFunctionSpaceType;

  public:
    //- Public methods
    CombinedBaseFunctionSet(const ContainedBaseFunctionSetType& bfSet) :
      containedResult_(0.0),
      baseFunctionSet_(bfSet)
    {}

    int getNumberOfBaseFunctions() const {
      return baseFunctionSet_.getNumberOfBaseFunctions()*N;
    }

    template <int diffOrd>
    void evaluate (int baseFunct,
                   const FieldVector<deriType, diffOrd> &diffVariable,
                   const DomainType & x, RangeType & phi ) const;

    template <int diffOrd, class QuadratureType >
    void evaluate (int baseFunct,
                   const FieldVector<deriType, diffOrd> &diffVariable,
                   QuadratureType & quad,
                   int quadPoint, RangeType & phi ) const;
  private:
    //- Private methods
    void expand(const ContainedRangeType& arg, RangeType& dest) const ;

  private:
    //- Data members
    mutable ContainedRangeType containtedResult_;
    const ContainedBaseFunctionSetType& baseFunctionSet_;
  };

  template <class DiscreteFunctionSpaceImp, int N>
  class CombinedMapper :
    public DofMapperDefault<CombinedMapper<DiscreteFunctionSpaceImp, int N> >
  {
  public:
    //- Typedefs and enums

  public:
    //- Public methods

  private:
    //- Private methods

  private:
    //- Data members

  };

  // include implementation
  #include "combinedspace.cc"

} // end namespace Dune

#endif
