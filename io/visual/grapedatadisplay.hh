// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRAPE_DATA_DISPLAY_HH
#define DUNE_GRAPE_DATA_DISPLAY_HH

#include "grapegriddisplay.hh"
#include <dune/quadrature/fixedorder.hh>

namespace Dune
{

  /*!
     GrapeDataDisplay
   */

  template<class GridType, class DiscFuncType>
  class GrapeDataDisplay : public GrapeGridDisplay < GridType >
  {
    typedef GrapeDataDisplay < GridType , DiscFuncType > MyDisplayType;
    typedef typename DiscFuncType::FunctionSpaceType FunctionSpaceType;
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;

    enum { dim = GridType::dimension };
    enum { dimworld = GridType::dimensionworld };

    typedef typename GrapeInterface<dim,dimworld>::DUNE_ELEM DUNE_ELEM;
    typedef typename GrapeInterface<dim,dimworld>::DUNE_FDATA DUNE_FDATA;

  public:

    typedef typename GridType::Traits::template Codim<0>::LevelIterator LevelIterator;
    typedef DiscFuncType DiscreteFunctionType;

  public:
    //! Constructor, make a GrapeDataDisplay for given grid and myRank = -1
    inline GrapeDataDisplay(GridType &grid);

    //! Constructor, make a GrapeDataDisplay for given grid
    inline GrapeDataDisplay(GridType &grid, const int myrank);

    inline ~GrapeDataDisplay();

    //! Calls the display of the grid and draws the discrete function
    //! if discretefunction is NULL, then only the grid is displayed
    inline void dataDisplay(DiscFuncType &func, bool vector = false);

    //! add discrete function to display
    inline void addData(DiscFuncType &func, const DATAINFO * , double time );

    //! add discrete function to display
    inline void addData(DiscFuncType &func, const char * name , double time , bool vector = false );

    // retrun whether we have data or not
    bool hasData () { return vecFdata_.size() > 0; }

    // return vector for copying in combined display
    std::vector < DUNE_FDATA * > & getFdataVec () { return vecFdata_; }

  private:
    inline void createQuadrature();

    //! hold the diffrent datas on this mesh
    // std::vector sucks
    std::vector < DUNE_FDATA * > vecFdata_;

    typedef FixedOrderQuad < typename FunctionSpaceType::RangeFieldType ,
        typename FunctionSpaceType::DomainType , 1 > QuadType;
    QuadType * quad_;

    // tmp variables
    RangeType tmp_;
    DomainType domTmp_;

    // for the data visualization
    template <class EntityType>
    inline void evalCoord (EntityType &en, DUNE_FDATA *, const double *coord, double * val);

    // for the data visualization
    template <class EntityType>
    inline void evalDof (EntityType &en, DUNE_FDATA * , int localNum, double * val);
    // for the data visualization
    template <class EntityType, class LocalFuncType>
    inline void evalScalar (EntityType &en, DiscFuncType & func, LocalFuncType &lf,
                            const int * comp , int localNum, double * val);

    // for the data visualization
    template <class EntityType, class LocalFuncType>
    inline void evalVector (EntityType &en, DiscFuncType & func,LocalFuncType &lf,
                            const int * comp, int vend, int localNum, double * val);
  public:
    inline void evalCoord (DUNE_ELEM *he, DUNE_FDATA *df,
                           const double *coord, double * val);
    inline void evalDof (DUNE_ELEM *he, DUNE_FDATA *df, int localNum, double * val);

  protected:

    inline static void func_real (DUNE_ELEM *he , DUNE_FDATA * fe,int ind,
                                  const double *coord, double *val);
  };

} // end namespace Dune

#include "grape/grapedatadisplay.cc"


#endif
