// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_FASTBASE_HH
#define DUNE_FASTBASE_HH

#include "basefunctions.hh"

namespace Dune {

  /** @defgroup FastBaseFunctionSet The FastBaseFunctionSet
     @ingroup BaseFunction
     The FastBaseFunctionSet is an special implementation of the interface
     BaseFunctionSetInterface. The idea here is to cache all evaluations of
     the base functions for a given quadrature. That means if we use an
     quadrature the only on the first call of evaluate of the base function
     set the virtual methods of the base functions have to be called. On all
     later calls the return values are memorized and the evaluation is much
     faster.

     @{
   */
  template <class FunctionSpaceImp>
  class FastBaseFunctionSet;

  template <class FunctionSpaceImp>
  struct FastBaseFunctionSetTraits {
    typedef FunctionSpaceImp DiscreteFunctionSpaceType;
    typedef FastBaseFunctionSet<DiscreteFunctionSpaceType> BaseFunctionSetType;

  };

  //*************************************************************************
  //
  //  --FastBaseFunctionSet
  //
  //! FastBaseFunctionSet is the Implementation of a BaseFunctionSet.
  //! It stores the values at the quadrature points to speed up the evaluation
  //! of the base functions. Furthermore the discrete function space is
  //! holding pointers to this class, because for different types of
  //! base function set (i.e. for triangles and quadrilaterals ) the type of
  //! the class has to be the same. There for the list with pointerd to hte
  //! base functions is filled by the derived class.
  //! All this pointer to base function and virtual method calling is not
  //! efficient. Therefor the values of the base function on given quadrature
  //! points is cache an on the next call of evaluate this is much faster.
  //
  // deriType is defined in basefunctions.hh
  //
  //*************************************************************************
  template<class FunctionSpaceType>
  class FastBaseFunctionSet :
    public BaseFunctionSetDefault<FastBaseFunctionSetTraits<FunctionSpaceType> >
  {
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    enum { DimDomain = FunctionSpaceType::DimDomain };
    enum { DimRange  = FunctionSpaceType::DimRange  };

  public:
    typedef FastBaseFunctionSetTraits<FunctionSpaceType> BaseFunctionSetType;
    typedef FastBaseFunctionSetTraits<BaseFunctionSetType> Traits;

    //! the BaseFunctionInterface type
    typedef BaseFunctionInterface < FunctionSpaceType > BaseFunctionInterfaceType;

    //! Constructor
    FastBaseFunctionSet (FunctionSpaceType & fuspace , int numOfBaseFct);

    //! Destructor
    virtual ~FastBaseFunctionSet() {
      for (unsigned int i = 0; i < baseFunctionList_.size(); ++i) {
        delete baseFunctionList_[i];
        baseFunctionList_[i] = 0;
      }
      baseFunctionList_.resize(0);
    }

    //! return the number of base fucntions for this BaseFunctionSet
    int numBaseFunctions () const
    {
      return baseFunctionList_.size();
    }

    //! return the number of base fucntions for this BaseFunctionSet
    int numDifferentBaseFunctions () const
    {
      return numOfDiffFct_;
    }

    //! evaluate base function baseFunct with the given diffVariable and a
    //! point x and range phi
    template <int diffOrd>
    void evaluate ( int baseFunct,
                    const FieldVector<deriType, diffOrd> &diffVariable,
                    const DomainType & x,  RangeType & phi ) const;

    //! evaluate base fucntion baseFunct at a given quadrature point
    //! the identifier of the quadrature is stored to check , whether the
    //! qaudrature has changed an the values at the quadrature have to be
    //! calulated again
    template <int diffOrd, class QuadratureType>
    void evaluate ( int baseFunct,
                    const FieldVector<deriType, diffOrd> &diffVariable,
                    QuadratureType & quad, int quadPoint, RangeType & phi) const;

    //! get a reference of the base function baseFunct
    //! this is the same concept for all basis, we have a number of
    //! base functions, and store the pointers in a vector
    const BaseFunctionInterface<FunctionSpaceType>
    &getBaseFunction( int baseFunct ) const
    {
      return (*baseFunctionList_[baseFunct]);
    }

    //! maximun number of different differentiation order
    enum { numDiffOrd = 3 };

  protected:
    //! set method for storage of the base function pointers
    void setBaseFunctionPointer( int baseFunc, BaseFunctionInterfaceType * func)
    {
      baseFunctionList_[baseFunc] = func;
    };

    //! \todo Please doc me!
    void setNumOfDiffFct ( int num )
    {
      numOfDiffFct_ = num;
    };

  private:
    //! number of different basefunctions
    int numOfDiffFct_;

    //! vector which holds the base function pointers
    std::vector<BaseFunctionInterfaceType *> baseFunctionList_ ;

    //! method to navigate through the vector vecEvaluate, which holds
    //! precalculated values
    template <int diffOrd>
    int index( int baseFunct, const FieldVector<deriType, diffOrd> &diffVariable,
               int quadPt, int numQuadPoints ) const
    {
      int n = 0;
      for ( int i = 0; i < diffOrd; i++ )
        n = diffVariable[i] + i * DimDomain;

      return numQuadPoints*(numBaseFunctions()*n + baseFunct) + quadPt;
    };

    //! vector holding the cached evaluation of the base functions
    std::vector< std::vector< RangeType > > vecEvaluate_;

    //! for which waudrature are we holding precalculated values ;
    IdentifierType evaluateQuad_[ numDiffOrd ];

    //! init the vecEvaluate vector
    template <int diffOrd, class QuadratureType >
    void evaluateInit ( const QuadratureType & quad ) ;

  }; // end class FastBaseFunctionSet

  /** @} end documentation group */

#include "fastbase.cc"

} // end namespace Dune

#endif
