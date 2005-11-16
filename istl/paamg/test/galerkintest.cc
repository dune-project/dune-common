// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <iostream>
#include <dune/common/enumset.hh>
#include <dune/istl/paamg/galerkin.hh>
#include <dune/istl/paamg/dependency.hh>
#include <dune/istl/paamg/globalaggregates.hh>
#include <dune/istl/io.hh>
#include <dune/istl/communicator.hh>
#include <dune/istl/bcrsmatrix.hh>
#include <dune/common/fmatrix.hh>
#include <dune/istl/paamg/indicescoarsener.hh>
#include <dune/istl/paamg/galerkin.hh>
#include <dune/common/propertymap.hh>
#include "anisotropic.hh"
//#include<dune/istl/paamg/aggregates.hh>

template<int BS>
void testCoarsenIndices(int N)
{

  int procs, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);

  ParallelIndexSet indices;
  typedef Dune::FieldMatrix<double,BS,BS> Block;
  typedef Dune::BCRSMatrix<Block> BCRSMat;
  int n;

  BCRSMat mat = setupAnisotropic2d<BS>(N, indices, &n);

  RemoteIndices remoteIndices(indices,indices,MPI_COMM_WORLD);
  remoteIndices.rebuild<false>();

  typedef Dune::Amg::MatrixGraph<BCRSMat> MatrixGraph;
  typedef Dune::Amg::SubGraph<Dune::Amg::MatrixGraph<BCRSMat>,std::vector<bool> > SubGraph;
  typedef Dune::Amg::PropertiesGraph<SubGraph,Dune::Amg::VertexProperties,
      Dune::Amg::EdgeProperties, Dune::IdentityMap, typename SubGraph::EdgeIndexMap> PropertiesGraph;
  typedef typename PropertiesGraph::VertexDescriptor Vertex;
  typedef Dune::Amg::SymmetricCriterion<BCRSMat,Dune::Amg::FirstDiagonal>
  Criterion;

  MatrixGraph mg(mat);
  std::vector<bool> excluded(mat.N());

  typedef ParallelIndexSet::iterator IndexIterator;
  IndexIterator iend = indices.end();
  typename std::vector<bool>::iterator iter=excluded.begin();

  for(IndexIterator index = indices.begin(); index != iend; ++index, ++iter)
    *iter = (index->local().attribute()==overlap);

  SubGraph sg(mg, excluded);
  PropertiesGraph pg(sg, Dune::IdentityMap(), sg.getEdgeIndexMap());
  Dune::Amg::AggregatesMap<Vertex> aggregatesMap(pg.maxVertex());

  std::cout << "fine indices: "<<indices << std::endl;
  std::cout << "fine remote: "<<remoteIndices << std::endl;

  aggregatesMap.buildAggregates(mat, pg, Criterion());

  Dune::Amg::printAggregates2d(aggregatesMap, n, N, std::cout);

  ParallelIndexSet coarseIndices;
  RemoteIndices coarseRemote(coarseIndices,coarseIndices, MPI_COMM_WORLD);

  typename Dune::PropertyMapTypeSelector<Dune::Amg::VertexVisitedTag,PropertiesGraph>::Type visitedMap = Dune::get(Dune::Amg::VertexVisitedTag(), pg);

  Dune::Amg::IndicesCoarsener<Dune::EnumItem<GridFlag,overlap>,ParallelIndexSet>::coarsen(indices,
                                                                                          remoteIndices,
                                                                                          pg,
                                                                                          visitedMap,
                                                                                          aggregatesMap,
                                                                                          coarseIndices,
                                                                                          coarseRemote);
  std::cout << rank <<": coarse indices: " <<coarseIndices << std::endl;
  std::cout << rank <<": coarse remote indices:"<<coarseRemote <<std::endl;


  Interface interface;
  interface.build(remoteIndices, Dune::EnumItem<GridFlag,owner>(), Dune::EnumItem<GridFlag, overlap>());
  Communicator communicator;

  typedef Dune::Amg::GlobalAggregatesMap<Vertex,ParallelIndexSet> GlobalMap;

  GlobalMap gmap(aggregatesMap,coarseIndices);

  communicator.build<GlobalMap>(interface);

  Dune::Amg::printAggregates2d(aggregatesMap, n, N, std::cout);

  communicator.template forward<Dune::Amg::AggregatesGatherScatter<typename MatrixGraph::VertexDescriptor,ParallelIndexSet> >(gmap);

  std::cout<<"Communicated: ";
  Dune::Amg::printAggregates2d(aggregatesMap, n, N, std::cout);

  Dune::Amg::GalerkinProduct productBuilder;

  typedef std::vector<bool> Vector;
  typedef typename Vector::iterator Iterator;
  //typedef Dune::IteratorPropertyMap<Iterator, Dune::IdentityMap> VisitedMap2;
  typedef Dune::IteratorPropertyMap<bool*, Dune::IdentityMap> VisitedMap2;
  /*
     Vector visited;
     visited.reserve(mg.maxVertex());
     Iterator visitedIterator=visited.begin();
   */
  std::cout<<n*n<<"=="<<mg.maxVertex()<<std::endl;

  assert(mat.N()==mg.maxVertex());

  bool visitedIterator[N*N];
  for(Vertex i=0; i < mg.maxVertex(); ++i)
    visitedIterator[i]=false;

  VisitedMap2 visitedMap2(visitedIterator, Dune::IdentityMap());

  BCRSMat* coarseMat = productBuilder.build(mat, mg, visitedMap2, indices,
                                            aggregatesMap, coarseIndices.size(),
                                            Dune::EnumItem<GridFlag,overlap>());
  productBuilder.calculate(mat, aggregatesMap, *coarseMat);
  if(N<5) {
    Dune::printmatrix(std::cout,mat,"fine","row",9,1);
    Dune::printmatrix(std::cout,*coarseMat,"coarse","row",9,1);
  }
}


int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);
  int N=5;

  if(argc>1)
    N = atoi(argv[1]);
  testCoarsenIndices<1>(N);
  MPI_Finalize();
}
