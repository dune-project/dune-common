// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// start with including some headers
#include "config.h"
#include <iostream>               // for input/output to shell

#include <dune/istl/paamg/graph.hh>
#include <dune/istl/istlexception.hh>
#include <dune/istl/bcrsmatrix.hh>
#include <dune/istl/fmatrix.hh>
#include <dune/istl/io.hh>

void testGraph ()
{
  const int N=4;

  typedef Dune::FieldMatrix<double,1,1> ScalarDouble;
  typedef Dune::BCRSMatrix<ScalarDouble> BCRSMat;

  double diagonal=4, offdiagonal=-1;

  BCRSMat laplacian2d(N*N,N*N,N*N*5,BCRSMat::row_wise);

  for (BCRSMat::CreateIterator i=laplacian2d.createbegin(); i!=laplacian2d.createend(); ++i) {
    int x = i.index()%N; // x coordinate in the 2d field
    int y = i.index()/N;  // y coordinate in the 2d field

    if(y>0)
      // insert lower neighbour
      i.insert(i.index()-N);
    if(x>0)
      // insert left neighbour
      i.insert(i.index()-1);

    // insert diagonal value
    i.insert(i.index());

    if(x<N-1)
      //insert right neighbour
      i.insert(i.index()+1);
    if(y<N-1)
      // insert upper neighbour
      i.insert(i.index()+N);
  }

  laplacian2d = offdiagonal;

  // Set the diagonal values
  for (BCRSMat::RowIterator i=laplacian2d.begin(); i!=laplacian2d.end(); ++i)
    i->operator[](i.index())=diagonal;

  Dune::printmatrix(std::cout,laplacian2d,"2d Laplacian","row",9,1);

  typedef Dune::amg::Graph<BCRSMat> BCRSGraph;

  BCRSGraph graph;

  graph.build(laplacian2d);
  graph.print(std::cout);
}


int main (int argc , char ** argv)
{
  try {
    testGraph();

  }
  catch (Dune::ISTLError& error)
  {
    std::cout << error << std::endl;
  }
  catch (Dune::Exception& error)
  {
    std::cout << error << std::endl;
  }
  catch (const std::bad_alloc& e)
  {
    std::cout << "memory exhausted" << std::endl;
  }
  catch (...)
  {
    std::cout << "unknown exception caught" << std::endl;
  }

  return 0;
}
