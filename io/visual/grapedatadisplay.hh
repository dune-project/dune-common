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
    typedef typename FunctionSpaceType::Domain DomainType;
    typedef typename FunctionSpaceType::Range RangeType;

    enum { dim = GridType::dimension };
    enum { dimworld = GridType::dimensionworld };

    typedef typename GrapeInterface<dim,dimworld>::DUNE_ELEM DUNE_ELEM;
    typedef typename GrapeInterface<dim,dimworld>::DUNE_FDATA DUNE_FDATA;

  public:

    typedef typename GridType::Traits::template codim<0>::LevelIterator LevelIterator;
    typedef DiscFuncType DiscreteFunctionType;

  public:
    //! Constructor, make a GrapeDataDisplay for given grid and myRank = -1
    GrapeDataDisplay(GridType &grid);

    //! Constructor, make a GrapeDataDisplay for given grid
    GrapeDataDisplay(GridType &grid, const int myrank);

    //! Calls the display of the grid and draws the discrete function
    //! if discretefunction is NULL, then only the grid is displayed
    void dataDisplay(DiscFuncType &func);

    //!
    void addData(DiscFuncType &func, const char * name , double time );

  private:
    void createQuadrature();

    //! hold the diffrent datas on this mesh
    // std::vector sucks
    std::vector < DUNE_FDATA * > vecFdata_;

    typedef FixedOrderQuad < typename FunctionSpaceType::RangeField ,
        typename FunctionSpaceType::Domain , 1 > QuadType;
    QuadType * quad_;


    RangeType tmp_;
    DomainType domTmp_;

    // for the data visualization
    template <class EntityType, class LocalFuncType>
    void evalCoord (EntityType &en, DiscFuncType &func, LocalFuncType &lf,
                    int comp, const double *coord, double * val);

    // for the data visualization
    template <class EntityType, class LocalFuncType>
    void evalDof (EntityType &en, DiscFuncType &func,LocalFuncType &lf,
                  int comp , int localNum, double * val);
    // for the data visualization
    template <class EntityType, class LocalFuncType>
    void evalScalar (EntityType &en, DiscFuncType & func, LocalFuncType &lf,
                     int comp , int localNum, double * val);

    // for the data visualization
    template <class EntityType, class LocalFuncType>
    void evalVector (EntityType &en, DiscFuncType & func,LocalFuncType &lf,
                     int comp , int localNum, double * val);

    void evalCoord (DUNE_ELEM *he, DUNE_FDATA *df,
                    const double *coord, double * val);
    void evalDof (DUNE_ELEM *he, DUNE_FDATA *df, int localNum, double * val);

    static void func_real (DUNE_ELEM *he , DUNE_FDATA * fe,int ind,
                           const double *coord, double *val);
  };

  //! default, do nothing
  template <GeometryType elType>
  int mapElType(int localNum)
  {
    return localNum;
  }

  //! specialization for quadrilaterals
  template <>
  int mapElType<quadrilateral>(int localNum)
  {
    switch(localNum)
    {
    case 2 :  return 3;
    case 3 :  return 2;
    default : return localNum;
    }
  }

  //! specialization for hexahedrons
  template <>
  int mapElType<hexahedron>(int localNum)
  {
    switch(localNum)
    {
    case 2 :  return 3;
    case 3 :  return 2;
    case 6 :  return 7;
    case 7 :  return 6;
    default : return localNum;
    }
  }


} // end namespace Dune

#include "grape/grapedatadisplay.cc"


#endif
