// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// start with including some headers
#include "config.h"
#include <iostream>               // for input/output to shell

#include <dune/istl/paamg/graph.hh>
#include <dune/istl/paamg/dependency.hh>
#include <dune/istl/istlexception.hh>
#include <dune/istl/bcrsmatrix.hh>
#include <dune/istl/fmatrix.hh>
#include <dune/istl/io.hh>

int testEdgeDepends(const Dune::amg::EdgeProperties& flags)
{
  int ret=0;

  if(!flags.depends()) {
    std::cerr << "Depends does not return true after setDepends! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(flags.influences()) {
    std::cerr << "Influences should not return true after setDepends! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(!flags.isStrong()) {
    std::cerr <<"Should be strong after setDepends! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(!flags.isOneWay()) {
    std::cerr <<"Should be oneWay after setDepends! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(flags.isTwoWay()) {
    std::cerr <<"Should not be twoWay after setDepends! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }
  return ret;
}

int testEdgeInfluences(const Dune::amg::EdgeProperties& flags)
{
  int ret=0;

  if(!flags.influences()) {
    std::cerr << "Influences does not return true after setInfluences! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(!flags.isStrong()) {
    std::cerr <<"Should be strong after setDepends and setInfluences! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(flags.isOneWay()) {
    std::cerr <<"Should not be oneWay after setDepends and setInfluences! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(flags.isTwoWay()) {
    std::cerr <<"Should not be twoWay after setInfluences! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }
  return ret;

}

int testEdgeTwoWay(const Dune::amg::EdgeProperties& flags)
{
  int ret=0;

  if(!flags.depends()) {
    std::cerr << "Depends does not return true after setDepends! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(!flags.influences()) {
    std::cerr << "Influences does not return true after setDepends! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(!flags.isStrong()) {
    std::cerr <<"Should be strong after setDepends and setInfluences! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(flags.isOneWay()) {
    std::cerr <<"Should not be oneWay after setDepends and setInfluences! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }


  if(!flags.isTwoWay()) {
    std::cerr <<"Should be twoWay after setDepends and setInfluences! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }
  return ret;

}

int testEdgeReset(const Dune::amg::EdgeProperties& flags)
{
  int ret=0;
  if(flags.depends()) {
    std::cerr << "Depend bit should be cleared after initialization or reset! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(flags.influences()) {
    std::cerr << "Influence bit should be cleared after initialization or reset! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }


  if(flags.isTwoWay()) {
    std::cerr << "Should not be twoWay after initialization or reset! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(flags.isOneWay()) {
    std::cerr << "Should not be oneWay after initialization or reset! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(flags.isStrong()) {
    std::cerr << "Should not be strong after initialization reset! "<<__FILE__
              <<":"<<__LINE__<<std::endl;
    ret++;
  }

  if(ret>0)
    std::cerr<<"Flags: "<<flags;

  return ret;

}

int testVertexReset(Dune::amg::VertexProperties& flags)
{
  int ret=0;

  if(flags.front()) {
    std::cerr<<"Front flag should not be set if reset!"<<__FILE__ ":"<<__LINE__
             <<std::endl;
    ret++;
  }

  if(flags.visited()) {
    std::cerr<<"Visited flag should not be set if reset!"<<__FILE__ ":"<<__LINE__
             <<std::endl;
    ret++;
  }

  if(flags.isolated()) {
    std::cerr<<"Isolated flag should not be set if reset!"<<__FILE__ ":"<<__LINE__
             <<std::endl;
    ret++;
  }

  return ret;

}

int testVertex()
{
  int ret=0;

  Dune::amg::VertexProperties flags;

  ret+=testVertexReset(flags);

  flags.setIsolated();

  if(!flags.isolated()) {
    std::cerr<<"Isolated flag should be set after setIsolated!"<<__FILE__ ":"<<__LINE__
             <<std::endl;
    ret++;
  }

  flags.resetIsolated();
  ret+=testVertexReset(flags);

  flags.setFront();

  if(!flags.front()) {
    std::cerr<<"Front flag should be set after setFront!"<<__FILE__ ":"<<__LINE__
             <<std::endl;
    ret++;
  }

  flags.resetFront();
  ret+=testVertexReset(flags);

  flags.setVisited();

  if(!flags.visited()) {
    std::cerr<<"Visited flag should be set after setVisited!"<<__FILE__ ":"<<__LINE__
             <<std::endl;
    ret++;
  }

  flags.resetVisited();

  ret+=testVertexReset(flags);

  flags.setExcluded();

  if(!flags.excluded()) {
    std::cerr<<"Excluded flag should be set after setExcluded!"<<__FILE__ ":"<<__LINE__
             <<std::endl;
    ret++;
  }

  flags.resetExcluded();
  ret+=testVertexReset(flags);

  return ret;

}

int testEdge()
{
  int ret=0;

  Dune::amg::EdgeProperties flags;

  ret += testEdgeReset(flags);

  flags.setDepends();

  ret += testEdgeDepends(flags);

  flags.resetDepends();

  flags.setInfluences();

  ret += testEdgeInfluences(flags);

  flags.resetInfluences();

  ret += testEdgeReset(flags);

  flags.setInfluences();
  flags.setDepends();

  testEdgeTwoWay(flags);

  flags.resetDepends();
  flags.resetInfluences();

  flags.setDepends();
  flags.setInfluences();


  flags.resetDepends();
  flags.resetInfluences();

  ret += testEdgeReset(flags);

  return ret;

}


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

  typedef Dune::amg::Graph<BCRSMat,int,int> BCRSGraph;

  BCRSGraph graph;

  graph.build(laplacian2d);
  graph.print(std::cout);
}


int main (int argc , char ** argv)
{
  try {
    testGraph();
    exit(testEdge());
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
