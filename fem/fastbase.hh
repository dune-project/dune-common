// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_FASTBASE_HH__
#define __DUNE_FASTBASE_HH__

#include "basefunctions.hh"

namespace Dune {


  //************************************************************************
  //
  //  --MapperInterface
  //
  //! Interface for calculating the size of a function space for a grid on a
  //! specified level.
  //! Furthermore the local to global mapping of dof number is done.
  //
  //************************************************************************
  template <class MapperImp>
  class MapperInterface
  {
  public:
    //! return number of dofs for special function space and grid on
    //! specified level
    template <class GridType>
    int size ( GridType &grid, int level ) const
    {
      return asImp().size(grid,level);
    };

    //! map a local dof num of a given entity to a global dof num
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const
    {
      return asImp().mapToGlobal( en , localNum );
    };

  private:
    //! Barton-Nackman trick
    MapperImp &asImp()  { return static_cast<MapperImp &>(*this); };
  };

  //***********************************************************************
  //
  //!  Provide default implementation of MapperInterface
  //
  //***********************************************************************
  template <class MapperImp>
  class MapperDefault
  {
  public: //! at the moment nothin'
  };


  //*************************************************************************
  //!
  //! FastBaseFunctionSet is the Implementation of a BaseFunctionSet.
  //! It stores the values at the quadrature points to speed up the evaluation
  //! of the base functions
  //! deriType is defined in basefunctions.hh
  //!
  //*************************************************************************
  template<class FunctionSpaceType>
  class FastBaseFunctionSet
    : public BaseFunctionSetDefault <FunctionSpaceType, FastBaseFunctionSet<FunctionSpaceType> >
  {

  public:
    //! the BaseFunctionInterface type
    typedef BaseFunctionInterface < FunctionSpaceType > BaseFunctionInterfaceType;

    //! Constructor
    FastBaseFunctionSet (FunctionSpaceType & fuspace , int numOfBaseFct);

    //! return the number of base fucntions for this BaseFunctionSet
    int getNumberOfBaseFunctions () const
    {
      return baseFunctionList_.size();
    };

    //! evaluate base function baseFunct with the given diffVariable and a
    //! point x and range phi
    template <int diffOrd>
    void evaluate ( int baseFunct, const Vec<diffOrd, deriType> &diffVariable,
                    const Domain & x,  Range & phi ) const;

    //! evaluate base fucntion baseFunct at a given quadrature point
    //! the identifier of the quadrature is stored to check , whether the
    //! qaudrature has changed an the values at the quadrature have to be
    //! calulated again
    template <int diffOrd, class QuadratureType>
    void evaluate ( int baseFunct, const Vec<diffOrd, deriType> &diffVariable,
                    QuadratureType & quad, int quadPoint, Range & phi ) const;

    //! get a reference of the base function baseFunct
    //! this is the same concept for all basis, we have a number of
    //! base functions, and store the pointers in a vector
    const BaseFunctionInterface<FunctionSpaceType>
    &getBaseFunction( int baseFunct ) const
    {
      //std::cout << "getBaseFunction \n";
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

  private:
    //! vector which holds the base function pointers
    std::vector< BaseFunctionInterfaceType *  > baseFunctionList_ ;

    //! method to navigate through the vector vecEvaluate, which holds
    //! precalculated values
    template <int diffOrd>
    int index( int baseFunct, const Vec<diffOrd, deriType> &diffVariable,
               int quadPt, int numQuadPoints ) const
    {
      int n = 0;
      for ( int i = 0; i < diffOrd; i++ )
        n = diffVariable.get(i) + i * DimDomain;

      return numQuadPoints*(getNumberOfBaseFunctions()*n + baseFunct) + quadPt;
    };

    //! vector holding the cached evaluation of the base functions
    std::vector< std::vector< Range > > vecEvaluate_;

    //! for which waudrature are we holding precalculated values ;
    IdentifierType evaluateQuad_[ numDiffOrd ];

    //! init the vecEvaluate vector
    template <int diffOrd, class QuadratureType >
    void evaluateInit ( const QuadratureType & quad ) ;

  }; // end class FastBaseFunctionSet

#include "fastbase.cc"

} // end namespace Dune

#endif
