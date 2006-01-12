// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#include <config.h>

#include <iostream>

/*

   Instantiate UG-Grid and feed it to the generic gridcheck()

   Currently UGGrid can only be initialized via the AmiraMeshReader

 */

#include <dune/grid/uggrid.hh>
#include <dune/io/file/amirameshreader.hh>

#include "gridcheck.cc"

class ArcOfCircle : public Dune::BoundarySegment<2>
{
public:

  ArcOfCircle(const Dune::FieldVector<double,2>& center, double radius,
              double fromAngle, double toAngle)
    : center_(center), radius_(radius), fromAngle_(fromAngle), toAngle_(toAngle)
  {}

  Dune::FieldVector<double,2> operator()(const Dune::FieldVector<double,1>& local) const {

    double angle = fromAngle_ + local[0]*(toAngle_ - fromAngle_);

    Dune::FieldVector<double,2> result = center_;
    result[0] += radius_ * std::cos(angle);
    result[1] += radius_ * std::sin(angle);

    return result;
  }

  Dune::FieldVector<double,2> center_;

  double radius_;

  double fromAngle_;

  double toAngle_;
};


void makeHalfCircleQuad(Dune::UGGrid<2,2>& grid, bool parametrization)
{
  using namespace Dune;

  grid.createbegin();

  // /////////////////////////////
  //   Create boundary segments
  // /////////////////////////////
  if (parametrization) {

    FieldVector<double,2> center(0);
    center[1] = 15;

    std::vector<int> vertices(2);

    vertices[0] = 1;  vertices[1] = 2;
    grid.insertBoundarySegment(vertices, new ArcOfCircle(center, 15, M_PI, M_PI*4/3));

    vertices[0] = 2;  vertices[1] = 3;
    grid.insertBoundarySegment(vertices, new ArcOfCircle(center, 15, M_PI*4/3, M_PI*5/3));

    vertices[0] = 3;  vertices[1] = 0;
    grid.insertBoundarySegment(vertices, new ArcOfCircle(center, 15, M_PI*5/3, M_PI*2));

  }

  // ///////////////////////
  //   Insert vertices
  // ///////////////////////
  FieldVector<double,2> pos;

  pos[0] = 15;  pos[1] = 15;
  grid.insertVertex(pos);

  pos[0] = -15; pos[1] = 15;
  grid.insertVertex(pos);

  pos[0] = -7.5; pos[1] = 2.00962;
  grid.insertVertex(pos);

  pos[0] = 7.5; pos[1] = 2.00962;
  grid.insertVertex(pos);
  // /////////////////
  // Insert elements
  // /////////////////

  std::vector<unsigned int> cornerIDs(4);
  cornerIDs[0] = 0;
  cornerIDs[1] = 1;
  cornerIDs[2] = 3;
  cornerIDs[3] = 2;

  grid.insertElement(cube, cornerIDs);

  // //////////////////////////////////////
  //   Finish initialization
  // //////////////////////////////////////
  grid.createend();

}


template <class GridType >
void markOne ( GridType & grid , int num , int ref )
{
  typedef typename GridType::template Codim<0>::LeafIterator LeafIterator;

  int count = 0;

  LeafIterator endit = grid.template leafend  <0> ();
  for(LeafIterator it = grid.template leafbegin<0> (); it != endit ; ++it )
  {
    if(num == count) grid.mark( ref, it );
    count++;
  }

  grid.preAdapt();
  grid.adapt();
  grid.postAdapt();
}

int main () {
  try {

    // ////////////////////////////////////////////////////////////////////////
    //  Do the standard grid test for a 2d UGGrid
    // ////////////////////////////////////////////////////////////////////////

    // extra-environment to check destruction
    {
      std::cout << std::endl << "UGGrid<2,2> with grid file: ug-testgrid-2.am"
                << std::endl << std::endl;
      Dune::UGGrid<2,2> grid;
      Dune::AmiraMeshReader<Dune::UGGrid<2,2> >::read(grid, "ug-testgrid-2.am");

      // check macro grid
      gridcheck(grid);

      // create hybrid grid
      markOne(grid,0,1) ;
      gridcheck(grid);

      grid.globalRefine(1);
      gridcheck(grid);
    }

    // ////////////////////////////////////////////////////////////////////////
    //  Do the standard grid test for a 3d UGGrid
    // ////////////////////////////////////////////////////////////////////////
    {
      std::cout << std::endl << "UGGrid<3,3> with grid file: ug-testgrid-3.am"
                << std::endl << std::endl;
      Dune::UGGrid<3,3> grid;
      Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::read(grid, "ug-testgrid-3.am");

      // check macro grid
      gridcheck(grid);

      // create hybrid grid
      markOne(grid,0,1) ;
      gridcheck(grid);

      grid.globalRefine(1);
      gridcheck(grid);
    }

    // ////////////////////////////////////////////////////////////////////////
    //   Check whether geometryInFather returns equal results with and
    //   without parametrized boundaries
    // ////////////////////////////////////////////////////////////////////////

    Dune::UGGrid<2,2> gridWithParametrization, gridWithoutParametrization;

    makeHalfCircleQuad(gridWithParametrization, true);
    makeHalfCircleQuad(gridWithoutParametrization, false);

    gridWithParametrization.globalRefine(1);
    gridWithoutParametrization.globalRefine(1);

    typedef Dune::UGGrid<2,2>::Codim<0>::LevelIterator ElementIterator;
    ElementIterator eIt    = gridWithParametrization.lbegin<0>(1);
    ElementIterator eWoIt  = gridWithoutParametrization.lbegin<0>(1);
    ElementIterator eEndIt = gridWithParametrization.lend<0>(1);

    for (; eIt!=eEndIt; ++eIt, ++eWoIt) {

      // The grids where constructed identically and they are traversed identically
      // Thus their respective output from geometryInFather should be the same
      for (int i=0; i<eIt->geometry().corners(); i++) {

        Dune::FieldVector<double,2> diff = eIt->geometryInFather()[i] - eWoIt->geometryInFather()[i];

        if ( diff.two_norm() > 1e-5 )
          DUNE_THROW(Dune::GridError, "output of geometryInFather() depends on boundary parametrization!");

      }

    }

  } catch (Dune::Exception &e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }

  return 0;
};
