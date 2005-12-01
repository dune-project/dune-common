// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRAPE_DATA_DISPLAY_HH
#define DUNE_GRAPE_DATA_DISPLAY_HH

//- system includes
#include <vector>

//- dune includes
#include <dune/grid/common/gridpart.hh>
#include <dune/fem/lagrangebase.hh>
#include <dune/fem/dfadapt.hh>

//- local includes
#include "grapegriddisplay.hh"

namespace Dune
{

  template <typename ctype, int dim, int dimworld, int polOrd>
  class GrapeLagrangePoints;
  /*!
     GrapeDataDisplay
   */

  template <class GridType, class DiscreteFunctionType>
  struct EvalDiscreteFunctions
  {
    typedef typename GridType :: template Codim<0> :: Entity EntityType;
    enum { dim = GridType::dimension };
    enum { dimworld = GridType::dimensionworld };

    typedef typename GridType :: ctype ctype;

    typedef typename DiscreteFunctionType :: LocalFunctionType LocalFunctionType;
    typedef typename DiscreteFunctionType :: FunctionSpaceType FunctionSpaceType;

    typedef typename FunctionSpaceType :: RangeType RangeType;
    typedef typename FunctionSpaceType :: DomainType DomainType;

    typedef typename GrapeInterface<dim,dimworld>::DUNE_ELEM DUNE_ELEM;
    typedef typename GrapeInterface<dim,dimworld>::DUNE_FDATA DUNE_FDATA;

    // for the data visualization
    inline static void evalCoordNow (EntityType &en, DUNE_FDATA *fdata, const double *coord, double * val);

    // for the data visualization
    inline static void evalDofNow (EntityType &en, int geomType, DUNE_FDATA *fdata , int localNum, double * val);

    inline static void evalCoord (DUNE_ELEM *he, DUNE_FDATA *df,
                                  const double *coord, double * val);

    inline static void evalDof (DUNE_ELEM *he, DUNE_FDATA *df, int localNum, double * val);

    // for the data visualization
    inline static void evalScalar (EntityType &en, int geomType,
                                   DiscreteFunctionType & func, LocalFunctionType &lf,
                                   const int * comp , int localNum, double * val);

    // for the data visualization
    inline static void evalVector (EntityType &en, int geomType,
                                   DiscreteFunctionType & func, LocalFunctionType &lf,
                                   const int * comp, int vend, int localNum, double * val);
  };


  template<class GridType>
  class GrapeDataDisplay : public GrapeGridDisplay < GridType >
  {
    typedef GrapeDataDisplay < GridType > MyDisplayType;
    //typedef typename DiscFuncType::FunctionSpaceType FunctionSpaceType;
    //typedef typename FunctionSpaceType::DomainType DomainType;
    //typedef typename FunctionSpaceType::RangeType RangeType;

    enum { dim = GridType::dimension };
    enum { dimworld = GridType::dimensionworld };

    typedef typename GridType :: ctype ctype;

    typedef typename GrapeInterface<dim,dimworld>::DUNE_ELEM DUNE_ELEM;
    typedef typename GrapeInterface<dim,dimworld>::DUNE_FDATA DUNE_FDATA;

  public:

    typedef typename GridType::Traits::template Codim<0>::LevelIterator LevelIterator;
    //typedef DiscFuncType DiscreteFunctionType;
    //enum { polynomialOrder = FunctionSpaceType::polynomialOrder };
    enum { polynomialOrder = 1 };

  public:
    //! Constructor, make a GrapeDataDisplay for given grid and myRank = -1
    inline GrapeDataDisplay(const GridType &grid);

    //! Constructor, make a GrapeDataDisplay for given grid
    inline GrapeDataDisplay(const GridType &grid, const int myrank);

    inline ~GrapeDataDisplay();

    //! Calls the display of the grid and draws the discrete function
    //! if discretefunction is NULL, then only the grid is displayed
    template <class DiscFuncType>
    inline void dataDisplay(DiscFuncType &func, bool vector = false);

    //! Calls the display of the grid and draws the discrete function
    //! if discretefunction is NULL, then only the grid is displayed
    template <class VectorPointerType>
    inline void displayVector(const VectorPointerType * vector);

    //! add discrete function to display
    template <class DiscFuncType>
    inline void addData(DiscFuncType &func, const DATAINFO * , double time );

    //! add discrete function to display
    template <class DiscFuncType>
    inline void addData(DiscFuncType &func, const char * name , double time , bool vector = false );

    // retrun whether we have data or not
    bool hasData () { return vecFdata_.size() > 0; }

    // return vector for copying in combined display
    std::vector < DUNE_FDATA * > & getFdataVec () { return vecFdata_; }

  private:
    //! hold the diffrent datas on this mesh
    // std::vector sucks
    std::vector < DUNE_FDATA * > vecFdata_;

    // store lagrange points for evaluation
    GrapeLagrangePoints<ctype,dim,dimworld,polynomialOrder> lagrangePoints_;

    typedef typename GridType :: template Codim<0> :: Entity EntityCodim0Type;
    typedef void evalCoord_t (EntityCodim0Type &, DUNE_FDATA *, const double *, double * );
    typedef void evalDof_t (EntityCodim0Type &,int , DUNE_FDATA * , int , double * );

  public:
  protected:

    inline static void func_real (DUNE_ELEM *he , DUNE_FDATA * fe,int ind,
                                  const double *coord, double *val);
  };

  template <typename ctype, int dim, int dimworld, int polOrd>
  class GrapeLagrangePoints
  {
    enum { maxPoints = 20 };
    enum { numberOfTypes = (dim == 2) ? 2 : 6 };

    std::vector < FieldMatrix<ctype,maxPoints,dim> > points_;
  public:
    //! create lagrange points for given polyOrder and dim,dimworld
    GrapeLagrangePoints ()
    {
      for(int type=0; type<numberOfTypes; type++)
      {
        FieldMatrix<ctype,maxPoints,dim> coords(0.0);
        int nvx = numberOfVertices(type);

        for(int i=0; i<nvx; i++)
        {
          const double * p = getCoordinate(type,i);
          for(int j=0; j<dimworld; j++)
          {
            assert( p );
            coords[i][j] = p[j];
          }
        }
        points_.push_back( coords );
      }
    }

    //! return lagrange point with localNum
    //! for given element type and polyOrder
    const FieldVector<ctype,dim> &
    getPoint (int geomType, int polyOrder , int localNum ) const
    {
      assert( polOrd == polyOrder );
      assert( geomType >= 0 );
      assert( geomType < numberOfTypes );
      return points_[geomType][localNum];
    }

  private:
    int numberOfVertices( int type )
    {
      if(type < 2)
        return GrapeInterface_two_two::getElementDescription(type)->number_of_vertices;
      else
        return GrapeInterface_three_three::getElementDescription(type)->number_of_vertices;
    }

    const double * getCoordinate( int type, int i )
    {
      if(type < 2)
        return GrapeInterface_two_two::getElementDescription(type)->coord[i];
      else
        return GrapeInterface_three_three::getElementDescription(type)->coord[i];
    }
  };

  /*
     template <int polOrd>
     struct GrapeVectorDisplay
     {
     template <class GridType, class VectorPointerType>
     static void
     display(const GridType & grid, const VectorPointerType * vector )
     {
      enum { dim = GridType :: dimension };
      typedef FunctionSpace <VectorPointerType ,
        VectorPointerType, dim, 1 >  FuncSpaceType;

      typedef typename GridType :: Traits:: LeafIndexSet LeafSet;
      typedef DefaultGridPart<GridType,LeafSet> GridPartType;
      typedef LagrangeDiscreteFunctionSpace
        < FuncSpaceType , GridPartType , polOrd > FunctionSpaceType;
      typedef DFAdapt< FunctionSpaceType > DiscreteFunctionType;

      GrapeDataDisplay < GridType , DiscreteFunctionType > disp(grid);
      disp.displayVector( vector );
     }
     };
   */

} // end namespace Dune

#include "grape/grapedatadisplay.cc"


#endif
