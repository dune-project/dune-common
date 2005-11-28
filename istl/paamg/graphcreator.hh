// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_AMG_GRAPHCREATOR_HH
#define DUNE_AMG_GRAPHCREATOR_HH

#include "graph.hh"
#include "pmatrix.hh"
#include "dependency.hh"
#include <dune/istl/bcrsmatrix.hh>
#include <dune/common/tuples.hh>

namespace Dune
{
  namespace Amg
  {
    template<class M>
    struct PropertiesGraphCreator
    {};

    template<class T, class A>
    struct PropertiesGraphCreator<BCRSMatrix<T, A> >
    {

      typedef MatrixGraph<const BCRSMatrix<T, A> >MatrixGraph;

      typedef PropertiesGraph<MatrixGraph,
          VertexProperties,
          EdgeProperties,
          IdentityMap,
          IdentityMap>
      PropertiesGraph;

      typedef Tuple<MatrixGraph,PropertiesGraph> GraphTuple;


    };

    template<class M,class I>
    struct PropertiesGraphCreator<ParallelMatrix<M,I> >
    {
      typedef MatrixGraph<const M> MatrixGraph;
      typedef SubGraph<MatrixGraph,
          std::vector<bool> > SubGraph;
      typedef PropertiesGraph<SubGraph,
          VertexProperties,
          EdgeProperties,
          IdentityMap,
          typename SubGraph::EdgeIndexMap>
      PropertiesGraph;

      typedef Tuple<MatrixGraph*,PropertiesGraph*,SubGraph*> GraphTuple;

      template<class OF, class T>
      static GraphTuple create(const ParallelMatrix<M,I>& matrix, T& excluded, const OF&)
      {
        typedef OF OverlapFlags;
        MatrixGraph* mg = new MatrixGraph(matrix.getmat());
        typedef typename ParallelMatrix<M,I>::ParallelIndexSet ParallelIndexSet;
        typedef typename ParallelIndexSet::const_iterator IndexIterator;
        IndexIterator iend = matrix.indexSet().end();
        typename T::iterator iter=excluded.begin();

        for(IndexIterator index = matrix.indexSet().begin(); index != iend; ++index, ++iter)
          *iter = (OverlapFlags::contains(index->local().attribute()));

        SubGraph* sg= new SubGraph(*mg, excluded);
        PropertiesGraph* pg = new PropertiesGraph(*sg, IdentityMap(), sg->getEdgeIndexMap());
        return GraphTuple(mg,pg,sg);
      }
    };

  } //namespace Amg
} // namespace Dune
#endif
