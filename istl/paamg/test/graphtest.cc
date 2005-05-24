// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// start with including some headers
#include "config.h"
#include <iostream>               // for input/output to shell

#include <dune/istl/paamg/graph.hh>
#include <dune/istl/paamg/dependency.hh>
#include <dune/istl/paamg/aggregates.hh>
#include <dune/istl/istlexception.hh>
#include <dune/istl/bcrsmatrix.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/typetraits.hh>
#include <dune/istl/io.hh>

int testEdgeDepends(const Dune::Amg::EdgeProperties& flags)
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

int testEdgeInfluences(const Dune::Amg::EdgeProperties& flags)
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

int testEdgeTwoWay(const Dune::Amg::EdgeProperties& flags)
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

int testEdgeReset(const Dune::Amg::EdgeProperties& flags)
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

int testVertexReset(Dune::Amg::VertexProperties& flags)
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

  Dune::Amg::VertexProperties flags;

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

  Dune::Amg::EdgeProperties flags;

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

template<int N, class B>
void setupSparsityPattern(Dune::BCRSMatrix<B>& A)
{
  for (typename Dune::BCRSMatrix<B>::CreateIterator i = A.createbegin(); i != A.createend(); ++i) {
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
}

template<int N, class B>
void setupAnisotropic(Dune::BCRSMatrix<B>& A, double eps)
{
  B diagonal = 0, bone=0, beps=0;
  for(typename B::RowIterator b = diagonal.begin(); b !=  diagonal.end(); ++b)
    b->operator[](b.index())=2.0+2.0*eps;


  for(typename B::RowIterator b=bone.begin(); b !=  bone.end(); ++b)
    b->operator[](b.index())=-1.0;

  for(typename B::RowIterator b=beps.begin(); b !=  beps.end(); ++b)
    b->operator[](b.index())=-eps;

  for (typename Dune::BCRSMatrix<B>::RowIterator i = A.begin(); i != A.end(); ++i) {
    int x = i.index()%N; // x coordinate in the 2d field
    int y = i.index()/N;  // y coordinate in the 2d field

    i->operator[](i.index())=diagonal;

    if(y>0)
      i->operator[](i.index()-N)=beps;

    if(y<N-1)
      i->operator[](i.index()+N)=beps;

    if(x>0)
      i->operator[](i.index()-1)=bone;

    if(x < N-1)
      i->operator[](i.index()+1)=bone;
  }
}

template<class N, class G>
void printWeightedGraph(G& graph, std::ostream& os, const N& norm=N())
{
  using Dune::RemoveConst;
  using Dune::SelectType;
  using Dune::SameType;

  typedef typename RemoveConst<G>::Type Mutable;
  typedef typename SelectType<SameType<G,Mutable>::value,
      typename G::VertexIterator,
      typename G::ConstVertexIterator>::Type VertexIterator;

  typedef typename SelectType<SameType<G,Mutable>::value,
      typename G::EdgeIterator,
      typename G::ConstEdgeIterator>::Type EdgeIterator;
  for(VertexIterator vertex = graph.begin(); vertex!=graph.end(); ++vertex) {
    const EdgeIterator endEdge = vertex.end();
    os<<"Edges starting from Vertex "<<*vertex<<" (weight="<<norm(vertex.weight());
    os<<") to vertices ";

    for(EdgeIterator edge = vertex.begin(); edge != endEdge; ++edge)
      os<<edge.target()<<" (weight="<<edge.weight()<<"), ";
    os<<std::endl;
  }
}

template<class G>
void printPropertiesGraph(G& graph, std::ostream& os)
{
  using Dune::RemoveConst;
  using Dune::SelectType;
  using Dune::SameType;

  typedef typename RemoveConst<G>::Type Mutable;
  typedef typename SelectType<SameType<G,Mutable>::value,
      typename G::VertexIterator,
      typename G::ConstVertexIterator>::Type VertexIterator;

  typedef typename SelectType<SameType<G,Mutable>::value,
      typename G::EdgeIterator,
      typename G::ConstEdgeIterator>::Type EdgeIterator;
  for(VertexIterator vertex = graph.begin(); vertex!=graph.end(); ++vertex) {
    const EdgeIterator endEdge = vertex.end();
    os<<"Edges starting from Vertex "<<*vertex<<" to vertices (";
    os<<vertex.properties()<<") ";

    for(EdgeIterator edge = vertex.begin(); edge != endEdge; ++edge)
      os<<edge.target()<<" ("<<edge.properties()<<"), ";
    os<<std::endl;
  }
}

template<class G>
void printGraph(G& graph, std::ostream& os)
{
  using Dune::RemoveConst;
  using Dune::SelectType;
  using Dune::SameType;

  typedef typename RemoveConst<G>::Type Mutable;
  typedef typename SelectType<SameType<G,Mutable>::value,
      typename G::VertexIterator,
      typename G::ConstVertexIterator>::Type VertexIterator;

  typedef typename SelectType<SameType<G,Mutable>::value,
      typename G::EdgeIterator,
      typename G::ConstEdgeIterator>::Type EdgeIterator;
  for(VertexIterator vertex = graph.begin(); vertex!=graph.end(); ++vertex) {
    const EdgeIterator endEdge = vertex.end();
    os<<"Edges starting from Vertex "<<*vertex<<" to vertices ";

    for(EdgeIterator edge = vertex.begin(); edge != endEdge; ++edge)
      os<<edge.target()<<", ";
    os<<std::endl;
  }
}


void testGraph ()
{
  const int N=8;

  typedef Dune::FieldMatrix<double,1,1> ScalarDouble;
  typedef Dune::BCRSMatrix<ScalarDouble> BCRSMat;

  double diagonal=4, offdiagonal=-1;

  BCRSMat laplacian2d(N*N,N*N,N*N*5,BCRSMat::row_wise);

  setupSparsityPattern<N>(laplacian2d);

  laplacian2d = offdiagonal;

  // Set the diagonal values
  for (BCRSMat::RowIterator i=laplacian2d.begin(); i!=laplacian2d.end(); ++i)
    i->operator[](i.index())=diagonal;

  //Dune::printmatrix(std::cout,laplacian2d,"2d Laplacian","row",9,1);

  typedef Dune::Amg::MatrixGraph<BCRSMat> MatrixGraph;

  MatrixGraph mg(laplacian2d);

  using Dune::Amg::FirstDiagonal;
  printWeightedGraph(mg,std::cout,FirstDiagonal<BCRSMat::block_type>());
  printWeightedGraph(static_cast<const MatrixGraph&>(mg),
                     std::cout,FirstDiagonal<BCRSMat::block_type>());

  std::vector<bool> excluded(N*N, false);

  for(int i=0; i < N; i++) {
    excluded[i]=excluded[(N-1)*N+i]=true;
    excluded[i*N]=excluded[i*N+N-1]=true;
  }


  typedef Dune::Amg::SubGraph<Dune::Amg::MatrixGraph<BCRSMat>,std::vector<bool> > SubGraph;
  SubGraph sub(mg, excluded);

  for(std::vector<bool>::iterator iter=excluded.begin(); iter != excluded.end(); ++iter)
    std::cout<<*iter<<" ";

  std::cout<<std::endl<<"SubGraph:"<<std::endl;

  printGraph(sub, std::cout);

  typedef Dune::Amg::PropertiesGraph<MatrixGraph,
      Dune::Amg::VertexProperties,
      Dune::Amg::EdgeProperties> PropertiesGraph;

  std::cout<<std::endl<<"PropertiesGraph: ";
  PropertiesGraph pgraph(mg);

  std::cout<<" noVertices="<<pgraph.noVertices()<<std::endl; //" noEdges=";
  //std::cout<<pgraph.noEdges()<<std:endl

  printPropertiesGraph(pgraph, std::cout);
  //printPropertiesGraph(static_cast<const PropertiesGraph&>(pgraph), std::cout);

  using Dune::Amg::SymmetricDependency;
  using Dune::Amg::SymmetricCriterion;

  //SymmetricCriterion<BCRSGraph, FirstDiagonal<typename BCRSMat::block_type> > crit;
  SymmetricCriterion<PropertiesGraph,BCRSMat,FirstDiagonal> crit;

  Dune::Amg::Aggregates<PropertiesGraph> aggregates;
  Dune::Amg::AggregatesMap<int> aggregatesMap(pgraph.maxVertex());
  aggregates.build(laplacian2d, pgraph,  aggregatesMap, crit);
  Dune::Amg::printAggregates2d(aggregatesMap, N, N, std::cout);

}


void testAggregate(double eps)
{

  typedef Dune::FieldMatrix<double,1,1> ScalarDouble;
  typedef Dune::BCRSMatrix<ScalarDouble> BCRSMat;
  const int N=20;

  BCRSMat mat(N*N,N*N,N*N*5,BCRSMat::row_wise);

  setupSparsityPattern<N>(mat);
  setupAnisotropic<N>(mat, .001);

  typedef Dune::Amg::MatrixGraph<BCRSMat> BCRSGraph;
  typedef Dune::Amg::SubGraph<BCRSGraph,std::vector<bool> > SubGraph;
  typedef Dune::Amg::PropertiesGraph<BCRSGraph,Dune::Amg::VertexProperties,Dune::Amg::EdgeProperties> PropertiesGraph;
  typedef Dune::Amg::PropertiesGraph<SubGraph,Dune::Amg::VertexProperties,Dune::Amg::EdgeProperties> SPropertiesGraph;

  BCRSGraph graph(mat);
  PropertiesGraph pgraph(graph);

  std::vector<bool> excluded(N*N, false);

  for(int i=0; i < N; i++) {
    excluded[i]=excluded[(N-1)*N+i]=true;
    excluded[i*N]=excluded[i*N+N-1]=true;
  }

  SubGraph sgraph(graph, excluded);
  SPropertiesGraph spgraph(sgraph);


  using Dune::Amg::FirstDiagonal;
  using Dune::Amg::SymmetricDependency;
  using Dune::Amg::SymmetricCriterion;

  //SymmetricCriterion<BCRSGraph, FirstDiagonal<typename BCRSMat::block_type> > crit;
  SymmetricCriterion<PropertiesGraph,BCRSMat, FirstDiagonal> crit;

  Dune::Amg::Aggregates<PropertiesGraph> aggregates;

  SymmetricCriterion<SPropertiesGraph,BCRSMat, FirstDiagonal> scrit;

  Dune::Amg::Aggregates<SPropertiesGraph> saggregates;
  Dune::Amg::AggregatesMap<int> aggregatesMap(pgraph.maxVertex());

  aggregates.build(mat, pgraph, aggregatesMap, crit);

  Dune::Amg::printAggregates2d(aggregatesMap, N, N, std::cout);

  std::cout<<"Excluded!"<<std::endl;

  Dune::Amg::AggregatesMap<int> saggregatesMap(pgraph.maxVertex());
  saggregates.build(mat, spgraph, saggregatesMap, scrit);
  Dune::Amg::printAggregates2d(saggregatesMap, N, N, std::cout);



}

int main (int argc , char ** argv)
{
  try {
    testGraph();
    testAggregate(.001);
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
