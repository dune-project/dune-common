// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <iostream>
#include <dune/common/enumset.hh>
#include <dune/istl/paamg/galerkin.hh>
#include <dune/istl/paamg/dependency.hh>
#include <dune/istl/io.hh>
#include <dune/istl/communicator.hh>
#include <dune/istl/bcrsmatrix.hh>
#include <dune/common/fmatrix.hh>
#include <dune/istl/paamg/indicescoarsener.hh>
#include <dune/istl/paamg/galerkin.hh>
#include <dune/common/propertymap.hh>
//#include<dune/istl/paamg/aggregates.hh>

enum GridFlag { owner, overlap   };

typedef Dune::ParallelLocalIndex<GridFlag> LocalIndex;
typedef Dune::IndexSet<int,LocalIndex,101> IndexSet;
typedef Dune::RemoteIndices<int,GridFlag,101> RemoteIndices;
typedef Dune::Interface<int,GridFlag,101> Interface;
typedef Dune::BufferedCommunicator<int,GridFlag,101> Communicator;

template<int N, class M>
void setupPattern(M& mat, IndexSet& indices, int overlapStart, int overlapEnd,
                  int start, int end)
{
  int n = overlapEnd - overlapStart;

  typename M::CreateIterator iter = mat.createbegin();
  indices.beginResize();

  for(int j=0; j < N; j++)
    for(int i=overlapStart; i < overlapEnd; i++, ++iter) {
      int global = j*N+i;
      GridFlag flag = owner;
      bool isPublic = false;

      if(i<start || i>= end)
        flag=overlap;

      if(i<start+1 || i>= end-1)
        isPublic = true;

      indices.add(global, LocalIndex(iter.index(), flag, isPublic));

      iter.insert(iter.index());

      // i direction
      if(i > overlapStart )
        // We have a left neighbour
        iter.insert(iter.index()-1);

      if(i < overlapEnd-1)
        // We have a rigt neighbour
        iter.insert(iter.index()+1);

      // j direction
      // Overlap is a dirichlet border, discard neighbours
      if(flag != overlap) {
        if(j>0)
          // lower neighbour
          iter.insert(iter.index()-n);
        if(j<N-1)
          // upper neighbour
          iter.insert(iter.index()+n);
      }
    }
  indices.endResize();
}

template<int N, class M>
void fillValues(M& mat, int overlapStart, int overlapEnd, int start, int end)
{
  typedef typename M::block_type Block;
  double eps=.01;
  Block dval = 0, bone=0, bmone=0, beps=0;

  for(typename Block::RowIterator b = dval.begin(); b !=  dval.end(); ++b)
    b->operator[](b.index())=2.0+2.0*eps;

  for(typename Block::RowIterator b=bone.begin(); b !=  bone.end(); ++b)
    b->operator[](b.index())=1.0;

  for(typename Block::RowIterator b=bmone.begin(); b !=  bmone.end(); ++b)
    b->operator[](b.index())=-1.0;

  for(typename Block::RowIterator b=beps.begin(); b !=  beps.end(); ++b)
    b->operator[](b.index())=-eps;

  int n = overlapEnd-overlapStart;
  typedef typename Dune::BCRSMatrix<Block>::ColIterator ColIterator;
  typedef typename Dune::BCRSMatrix<Block>::RowIterator RowIterator;

  for (RowIterator i = mat.begin(); i != mat.end(); ++i) {
    // calculate coordinate
    int y = i.index() / n;
    int x = overlapStart + i.index() - y * n;

    ColIterator endi = (*i).end();

    if(x<start || x >= end) {
      // overlap node is dirichlet border
      ColIterator j = (*i).begin();

      for(; j.index() < i.index(); ++j)
        *j=0;

      *j = bone;

      for(++j; j != endi; ++j)
        *j=0;
    }else{
      for(ColIterator j = (*i).begin(); j != endi; ++j)
        if(j.index() == i.index())
          *j=dval;
        else if(j.index()+1==i.index() || i.index()+1==j.index())
          *j=beps;
        else
          *j=bmone;
    }
  }
}

template<int N, int BS>
Dune::BCRSMatrix<Dune::FieldMatrix<double,BS,BS> > setupAnisotropic2d(IndexSet& indices, int *nout)
{
  int procs, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);


  typedef Dune::FieldMatrix<double,BS,BS> Block;
  typedef Dune::BCRSMatrix<Block> BCRSMat;

  // calculate size of lokal matrix in the distributed direction
  int start, end, overlapStart, overlapEnd;

  int n = N/procs; // number of unknowns per process
  int bigger = N%procs; // number of process with n+1 unknows

  // Compute owner region
  if(rank<bigger) {
    start = rank*(n+1);
    end   = (rank+1)*(n+1);
  }else{
    start = bigger + rank * n;
    end   = bigger + (rank + 1) * n;
  }

  // Compute overlap region
  if(start>0)
    overlapStart = start - 1;
  else
    overlapStart = start;

  if(end<N)
    overlapEnd = end + 1;
  else
    overlapEnd = end;

  int noKnown = overlapEnd-overlapStart;

  *nout = noKnown;

  BCRSMat mat(noKnown*N, noKnown*N, noKnown*N*5, BCRSMat::row_wise);

  setupPattern<N>(mat, indices, overlapStart, overlapEnd, start, end);
  fillValues<N>(mat, overlapStart, overlapEnd, start, end);

  Dune::printmatrix(std::cout,mat,"aniso 2d","row",9,1);

  return mat;
}

template<typename T, typename TG, typename TA, int N>
struct GlobalAggregatesMap
{
public:
  typedef TG IndexedType;

  GlobalAggregatesMap(Dune::Amg::AggregatesMap<T>& aggregates,
                      Dune::IndexSet<TG,Dune::ParallelLocalIndex<TA>,N>& indexset)
    : aggregates_(aggregates), indexset_(indexset)
  {}

  inline const TG& operator[](std::size_t index) const
  {
    const T& aggregate = aggregates_[index];
    const Dune::IndexPair<TG,Dune::ParallelLocalIndex<TA> >& pair = indexset_.pair(aggregate);
    assert(aggregate>=0 && pair.local()==static_cast<size_t>(aggregate));
    assert(pair.local().attribute()==owner);
    return pair.global();
  }

  inline void put(const TG& global, size_t i)
  {
    const T& index = indexset_[global].local();
    aggregates_[i]=index;

  }

private:
  Dune::Amg::AggregatesMap<T>& aggregates_;
  Dune::GlobalLookupIndexSet<Dune::IndexSet<TG,Dune::ParallelLocalIndex<TA>,N> > indexset_;
};

template<typename T, typename TG, typename TA, int N>
struct AggregatesGatherScatter
{
  static const TG& gather(const GlobalAggregatesMap<T,TG,TA,N>& ga, size_t i)
  {
    const TG& g = ga[i];
    return g;
  }

  static void scatter(GlobalAggregatesMap<T,TG,TA,N>& ga, TG global, size_t i)
  {
    ga.put(global, i);
  }
};


namespace Dune
{
  template<typename T, typename TG, typename TA, int N>
  struct CommPolicy<GlobalAggregatesMap<T,TG,TA,N> >
  {
    typedef Amg::AggregatesMap<T> Type;
    typedef TG IndexedType;
    typedef SizeOne IndexedTypeFlag;
    static int getSize(const Type&, int)
    {
      return 1;
    }
  };
}

template<int N, int BS>
void testCoarsenIndices()
{

  int procs, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);

  IndexSet indices;
  typedef Dune::FieldMatrix<double,BS,BS> Block;
  typedef Dune::BCRSMatrix<Block> BCRSMat;
  int n;

  BCRSMat mat = setupAnisotropic2d<N,BS>(indices, &n);

  RemoteIndices remoteIndices(indices,indices,MPI_COMM_WORLD);
  remoteIndices.rebuild<false>();

  typedef Dune::Amg::MatrixGraph<BCRSMat> MatrixGraph;
  typedef Dune::Amg::SubGraph<Dune::Amg::MatrixGraph<BCRSMat>,std::vector<bool> > SubGraph;
  typedef Dune::Amg::PropertiesGraph<SubGraph,Dune::Amg::VertexProperties,
      Dune::Amg::EdgeProperties, Dune::IdentityMap, typename SubGraph::EdgeIndexMap> PropertiesGraph;
  typedef typename PropertiesGraph::VertexDescriptor Vertex;
  typedef Dune::Amg::Aggregates<PropertiesGraph> Aggregates;
  typedef Dune::Amg::SymmetricCriterion<PropertiesGraph,BCRSMat,Dune::Amg::FirstDiagonal>
  Criterion;

  MatrixGraph mg(mat);
  std::vector<bool> excluded(mat.N());

  typedef IndexSet::iterator IndexIterator;
  IndexIterator iend = indices.end();
  typename std::vector<bool>::iterator iter=excluded.begin();

  for(IndexIterator index = indices.begin(); index != iend; ++index, ++iter)
    *iter = (index->local().attribute()==overlap);

  SubGraph sg(mg, excluded);
  PropertiesGraph pg(sg, Dune::IdentityMap(), sg.getEdgeIndexMap());
  Aggregates aggregates;
  Dune::Amg::AggregatesMap<Vertex> aggregatesMap(pg.maxVertex());

  std::cout << "fine indices: "<<indices << std::endl;
  std::cout << "fine remote: "<<remoteIndices << std::endl;

  aggregates.build(mat, pg, aggregatesMap, Criterion());

  IndexSet coarseIndices;
  RemoteIndices coarseRemote(coarseIndices,coarseIndices, MPI_COMM_WORLD);

  typename Dune::PropertyMapTypeSelector<Dune::Amg::VertexVisitedTag,PropertiesGraph>::Type visitedMap = Dune::get(Dune::Amg::VertexVisitedTag(), pg);

  Dune::Amg::IndicesCoarsener<Dune::EnumItem<GridFlag,overlap>,int,GridFlag,101>::coarsen(indices,
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

  typedef GlobalAggregatesMap<Vertex,int,GridFlag,101> GlobalMap;

  GlobalMap gmap(aggregatesMap,coarseIndices);

  communicator.build<GlobalMap>(interface);

  Dune::Amg::printAggregates2d(aggregatesMap, n, N, std::cout);

  communicator.template forward<AggregatesGatherScatter<typename MatrixGraph::VertexDescriptor,int,GridFlag,101> >(gmap);

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
  std::cout<<N*N<<"=="<<mg.maxVertex()<<std::endl;

  assert(N*N==mg.maxVertex());

  bool visitedIterator[N*N];
  for(Vertex i=0; i < mg.maxVertex(); ++i)
    visitedIterator[i]=false;

  VisitedMap2 visitedMap2(visitedIterator, Dune::IdentityMap());

  BCRSMat* coarseMat = productBuilder.build(mat, mg, visitedMap2, indices,
                                            aggregatesMap,
                                            Dune::EnumItem<GridFlag,overlap>());
  productBuilder.calculate(mat, aggregatesMap, *coarseMat);
  Dune::printmatrix(std::cout,*coarseMat,"coarse","row",9,1);
}


int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);
  testCoarsenIndices<10,1>();
  MPI_Finalize();
}
