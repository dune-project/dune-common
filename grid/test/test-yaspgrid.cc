// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#include <config.h>

#include <iostream>

#include <dune/grid/yaspgrid.hh>

#include "gridcheck.cc"

template <int dim>
void check_yasp() {
  typedef Dune::FieldVector<int,dim> iTupel;
  typedef Dune::FieldVector<double,dim> fTupel;
  typedef Dune::FieldVector<bool,dim> bTupel;

  std::cout << std::endl << "YaspGrid<" << dim << "," << dim << ">"
            << std::endl << std::endl;

  fTupel Len; Len = 1.0;
  iTupel s; s = 5;
  bTupel p; p = false;
  int overlap = 1;

  Dune::YaspGrid<dim,dim> grid(MPI_COMM_WORLD,Len,s,p,overlap);

  grid.globalRefine(1);

  gridcheck(grid);
};

int main (int argc , char **argv) {
  try {
    // initialize MPI
    MPI_Init(&argc,&argv);

    // get own rank
    /* int rank;
       MPI_Comm_rank(MPI_COMM_WORLD,&rank); */

    check_yasp<2>();
    check_yasp<3>();

  } catch (Dune::Exception &e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }

  // Terminate MPI
  MPI_Finalize();

  return 0;
};
