// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_AMG_GRAPHCREATOR_HH
#define DUNE_AMG_GRAPHCREATOR_HH

#include "graph.hh"
#include "pmatrix.hh"
#include "dependency.hh"
#include <dune/istl/operators.hh>
#include <dune/istl/bcrsmatrix.hh>
#include <dune/common/tuples.hh>

namespace Dune
{
  namespace Amg
  {
    template<class M, int cat=M::category>
    struct PropertiesGraphCreator
    {};

    template<class M>
    struct PropertiesGraphCreator<M,SolverCategory::sequential>
    {
      typedef typename M::matrix_type Matrix;

      typedef MatrixGraph<const Matrix> MatrixGraph;

      typedef PropertiesGraph<MatrixGraph,
          VertexProperties,
          EdgeProperties,
          IdentityMap,
          IdentityMap> PropertiesGraph;

      typedef Tuple<MatrixGraph*,PropertiesGraph*> GraphTuple;

      template<class OF, class T>
      static GraphTuple create(const MatrixGraph& matrix, T& excluded,
                               const SequentialInformation& pinfo,
                               const OF&)
      {
        MatrixGraph* mg = new MatrixGraph(matrix.getmat());
        PropertiesGraph* pg = new PropertiesGraph(*mg, IdentityMap(), IdentityMap());
        return GraphTuple(mg,pg);
      }

      static void free(GraphTuple& graphs)
      {
        delete Element<1>::get(graphs);
      }

    };

    template<class M>
    struct PropertiesGraphCreator<M,SolverCategory::overlapping>
    {
      typedef typename M::matrix_type Matrix;
      typedef MatrixGraph<const Matrix> MatrixGraph;
      typedef SubGraph<MatrixGraph,
          std::vector<bool> > SubGraph;
      typedef PropertiesGraph<SubGraph,
          VertexProperties,
          EdgeProperties,
          IdentityMap,
          typename SubGraph::EdgeIndexMap>
      PropertiesGraph;

      typedef Tuple<MatrixGraph*,PropertiesGraph*,SubGraph*> GraphTuple;

      template<class OF, class T, class TI>
      static GraphTuple create(const M& matrix, T& excluded,
                               ParallelInformation<TI>& pinfo, const OF&)
      {
        typedef OF OverlapFlags;
        MatrixGraph* mg = new MatrixGraph(matrix.getmat());
        typedef typename ParallelInformation<TI>::IndexSet ParallelIndexSet;
        typedef typename ParallelIndexSet::const_iterator IndexIterator;
        IndexIterator iend = pinfo.indexSet().end();
        typename T::iterator iter=excluded.begin();

        for(IndexIterator index = pinfo.indexSet().begin(); index != iend; ++index, ++iter)
          *iter = (OverlapFlags::contains(index->local().attribute()));

        SubGraph* sg= new SubGraph(*mg, excluded);
        PropertiesGraph* pg = new PropertiesGraph(*sg, IdentityMap(), sg->getEdgeIndexMap());
        return GraphTuple(mg,pg,sg);
      }

      static void free(GraphTuple& graphs)
      {
        delete Element<2>::get(graphs);
        delete Element<1>::get(graphs);
      }
    };

  } //namespace Amg
} // namespace Dune
#endif
