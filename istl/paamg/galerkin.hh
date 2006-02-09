// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_GALERKIN_HH
#define DUNE_GALERKIN_HH

#include "aggregates.hh"
#include "pinfo.hh"
#include <dune/common/poolallocator.hh>
#include <dune/common/enumset.hh>
#include <set>
#include "mpi.h"

namespace Dune
{
  namespace Amg
  {
    /**
     * @addtogroup ISTL_PAAMG
     *
     * @{
     */
    /** @file
     * @author Markus Blatt
     * @brief Provides a class for building the galerkin product
     * based on a aggregation scheme.
     */

    class GalerkinProduct
    {
    public:
      /**
       * @brief Calculates the coarse matrix via a Galerkin product.
       * @param fine The matrix on the fine level.
       * @param fineGraph The graph of the fine matrix.
       * @param visitedMap Map for marking vertices as visited.
       * @param pinfo Parallel information about the fine level.
       * @param aggregates The mapping of the fine level unknowns  onto aggregates.
       * @param size The number of columns and rows of the coarse matrix.
       */
      template<class M, class G, class V, class I, class Set>
      M* build(const M& fine, G& fineGraph, V& visitedMap, const I& pinfo,
               const AggregatesMap<typename G::VertexDescriptor>& aggregates,
               const typename M::size_type& size,
               const Set& overlap);
      /**
       * @brief Calculates the coarse matrix via a Galerkin product.
       * @param fine The matrix on the fine level.
       * @param fineGraph The graph of the fine matrix.
       * @param visitedMap Map for marking vertices as visited.
       * @param pinfo Parallel information about the fine level.
       * @param aggregates The mapping of the fine level unknowns  onto aggregates.
       * @param size The number of columns and rows of the coarse matrix.
       */
      template<class M, class G, class V, class Set>
      M* build(const M& fine, G& fineGraph, V& visitedMap,
               const SequentialInformation& pinfo,
               const AggregatesMap<typename G::VertexDescriptor>& aggregates,
               const typename M::size_type& size,
               const Set& overlap);
      /**
       * @brief Calculates the coarse matrix via a Galerkin product.
       * @param fine The matrix on the fine level.
       * @param fineGraph The graph of the fine matrix.
       * @param aggregates The mapping of the fine level unknowns  onto aggregates.
       * @param size The number of columns and rows of the coarse matrix.
       *
         template<class M, class G, class V>
         M* build(const M& fine, G& fineGraph, V& visitedMap,
               const AggregatesMap<typename G::VertexDescriptor>& aggregates);
       */
      /**
       * @brief Calculate the galerkin product.
       * @param fine The fine matrix.
       * @param aggregates The aggregate mapping.
       * @param coarse The coarse Matric.
       */
      template<class M, class V>
      void calculate(const M& fine, const AggregatesMap<V>& aggregates, M& coarse);

    private:
      std::size_t* overlapStart_;

      template<class T>
      struct OverlapVertex
      {
        /**
         * @brief The aggregate descriptor.
         */
        typedef T Aggregate;


        /**
         * @brief The vertex descriptor.
         */
        typedef T Vertex;
        /**
         * @brief The aggregate the vertex belongs to.
         */
        Aggregate aggregate;

        /**
         * @brief The vertex descriptor.
         */
        Vertex vertex;
      };

      /**
       * @brief Builds the data structure needed for rebuilding the aggregates int the overlap.
       * @param graph The graph of the matrix.
       * @param pinfo The parallel information.
       * @param aggregates The mapping onto the aggregates.
       */
      template<class G, class I, class Set>
      const OverlapVertex<typename G::VertexDescriptor>*
      buildOverlapVertices(const G& graph,  const I& pinfo,
                           const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                           const Set& overlap);

      /**
       * @brief Deallocate the data structure needed for rebuilding the aggregates in the overlap.
       */
      void freeOverlapAggregates();

      /**
       * @brief Visitor for identifying connected aggregates during a breadthFirstSearch.
       */
      template<class G, class S, class V>
      class ConnectedBuilder
      {
      public:
        /**
         * @brief The type of the graph.
         */
        typedef G Graph;
        /**
         * @brief The constant edge iterator.
         */
        typedef typename Graph::ConstEdgeIterator ConstEdgeIterator;

        /**
         * @brief The type of the connected set.
         */
        typedef S Set;

        /**
         * @brief The type of the map for marking vertices as visited.
         */
        typedef V VisitedMap;

        /**
         * @brief The vertex descriptor of the graph.
         */
        typedef typename Graph::VertexDescriptor Vertex;

        /**
         * @brief Constructor
         * @param aggregates The mapping of the vertices onto the aggregates.
         * @param connected The set to added the connected aggregates to.
         */
        ConnectedBuilder(const AggregatesMap<Vertex>& aggregates, Graph& graph,
                         VisitedMap& visitedMap, Set& connected);

        /**
         * @brief Process an edge pointing to another aggregate.
         * @param edge The iterator positioned at the edge.
         */
        void operator()(const ConstEdgeIterator& edge);

      private:
        /**
         * @brief The mapping of the vertices onto the aggregates.
         */
        const AggregatesMap<Vertex>& aggregates_;

        Graph& graph_;

        /**
         * @brief The map for marking vertices as visited.
         */
        VisitedMap& visitedMap_;

        /**
         * @brief The set to add the connected aggregates to.
         */
        Set& connected_;
      };

      template<class T>
      struct OVLess
      {
        bool operator()(const OverlapVertex<T>& o1, const OverlapVertex<T>& o2)
        {
          return o1.aggregate < o2.aggregate;
        }
      };

      /** @brief Wrapper to addres matrix row iterator as index iterator. */
      template<class T>
      class RowToIndex
      {
      public:
        /** @brief The type of the matrix. */
        typedef T Matrix;

        /** @brief The type of the matrix row iterator. */
        typedef typename Matrix::ConstRowIterator MatrixRowIterator;

        RowToIndex(const MatrixRowIterator& row)
          : row_(row)
        {}

        std::size_t local() const
        {
          return row_.index();
        }

        void operator++()
        {
          ++row_;
        }
        bool operator==(const RowToIndex other) const
        {
          return row_ == other.row_;
        }

        bool operator!=(const RowToIndex other) const
        {
          return row_ != other.row_;
        }

        RowToIndex* operator->()
        {
          return this;
        }

        RowToIndex& operator*()
        {
          return *this;
        }


        const RowToIndex* operator->() const
        {
          return this;
        }

        const RowToIndex& operator*() const
        {
          return *this;
        }
      private:
        MatrixRowIterator row_;
      };

      struct BaseConnectivityConstructor
      {
        template<class R, class G, class V>
        static void constructOverlapConnectivity(R& row, G& graph, V& visitedMap,
                                                 const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                                 const OverlapVertex<typename G::VertexDescriptor>*& seed);

        /**
         * @brief Construct the connectivity of an aggregate in the overlap.
         */
        template<class R, class G, class V>
        static void constructNonOverlapConnectivity(R& row, G& graph, V& visitedMap,
                                                    const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                                    const typename G::VertexDescriptor& seed);
      };

      /**
       * @brief Functor for counting the nonzeros and unknowns using examineConnectivity.
       */
      template<class T>
      class NonZeroCounter
      {
      public:
        typedef T Matrix;

        /** @brief Constructor. */
        NonZeroCounter()
          : unknownsNonZeros_(std::make_pair(0,0)), connected_()
        {}

        /**
         * @brief Count the connected vertices and update.
         * @param connected The set of connected vertices.

           void operator(Set connected)
           {
         *++unknowsNonZeros_.first;
           unknownsNonZeros.second += connected.size();
           }
         */

        void insert(const typename Matrix::size_type& index)
        {
          connected_.insert(index);
        }

        void operator++()
        {
          ++unknownsNonZeros_.first;
          unknownsNonZeros_.second+=connected_.size();
          connected_.clear();
        }

        /** @brief Get the number of unknowns and nonzeros.*/
        const std::pair<int,int>& getUnknownsNonZeros()
        {
          return unknownsNonZeros_;
        }

      private:
        /** @brief Pair of the number of unknowns and the number of nonzeros. */
        std::pair<int,int> unknownsNonZeros_;
        std::set<typename Matrix::size_type> connected_;
      };

      /**
       * @brief Functor for building the sparsity pattern of the matrix
       * using examineConnectivity.
       */
      template<class M, class V>
      class SparsityBuilder
      {
      public:
        /**
         * @brief Constructor.
         * @param matrix The matrix whose sparsity pattern we
         * should set up.
         * @param aggregates THe mapping of the vertices onto the aggregates.
         */
        SparsityBuilder(M& matrix, const AggregatesMap<V>& aggregates);

        void insert(const typename M::size_type& index);

        void operator++();

      private:
        /** @brief Create iterator for the current row. */
        typename M::CreateIterator row_;
        /** @brief The aggregates mapping. */
        const AggregatesMap<V>& aggregates_;

      };

      template<class G, class T>
      struct ConnectivityConstructor : public BaseConnectivityConstructor
      {
        typedef typename G::VertexDescriptor Vertex;

        template<class V, class O, class R>
        static void examine(G& graph,
                            V& visitedMap,
                            const T& pinfo,
                            const AggregatesMap<Vertex>& aggregates,
                            const O& overlap,
                            const OverlapVertex<Vertex>* overlapVertices,
                            R& row);
      };

      template<class G>
      struct ConnectivityConstructor<G,SequentialInformation> : public BaseConnectivityConstructor
      {
        typedef typename G::VertexDescriptor Vertex;

        template<class V, class R>
        static void examine(G& graph,
                            V& visitedMap,
                            const SequentialInformation& pinfo,
                            const AggregatesMap<Vertex>& aggregates,
                            R& row);
      };
    };

    template<class R, class G, class V>
    void GalerkinProduct::BaseConnectivityConstructor::constructNonOverlapConnectivity(R& row, G& graph, V& visitedMap,
                                                                                       const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                                                                       const typename G::VertexDescriptor& seed)
    {
      row.insert(aggregates[seed]);
      ConnectedBuilder<G,R,V> conBuilder(aggregates, graph, visitedMap, row);
      typedef typename G::VertexDescriptor Vertex;
      typedef PoolAllocator<Vertex,100*sizeof(int)> Allocator;
      typedef SLList<Vertex,Allocator> VertexList;
      typedef typename AggregatesMap<Vertex>::DummyEdgeVisitor DummyVisitor;
      VertexList vlist;
      DummyVisitor dummy;
      aggregates.template breadthFirstSearch<true,false>(seed,aggregates[seed], graph, vlist, dummy,
                                                         conBuilder, visitedMap);
    }

    template<class R, class G, class V>
    void GalerkinProduct::BaseConnectivityConstructor::constructOverlapConnectivity(R& row, G& graph, V& visitedMap,
                                                                                    const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                                                                    const OverlapVertex<typename G::VertexDescriptor>*& seed)
    {
      const typename G::VertexDescriptor aggregate=seed->aggregate;
      row.insert(aggregate);
      ConnectedBuilder<G,R,V> conBuilder(aggregates, graph, visitedMap, row);
      while(aggregate == seed->aggregate) {
        // Walk over all neighbours and add them to the connected array.
        visitNeighbours(graph, seed->vertex, conBuilder);
        // Mark vertex as visited
        put(visitedMap, seed->vertex, true);
        ++seed;
      }
    }

    template<class G, class S, class V>
    GalerkinProduct::ConnectedBuilder<G,S,V>::ConnectedBuilder(const AggregatesMap<Vertex>& aggregates,
                                                               Graph& graph, VisitedMap& visitedMap,
                                                               Set& connected)
      : aggregates_(aggregates), graph_(graph), visitedMap_(visitedMap), connected_(connected)
    {}

    template<class G, class S, class V>
    void GalerkinProduct::ConnectedBuilder<G,S,V>::operator()(const ConstEdgeIterator& edge)
    {
      typedef typename G::VertexDescriptor Vertex;
      const Vertex& vertex = aggregates_[edge.target()];
      assert(vertex!= AggregatesMap<Vertex>::UNAGGREGATED);
      if(vertex!= AggregatesMap<Vertex>::ISOLATED)
        connected_.insert(vertex);
    }

    template<class G, class I, class Set>
    const GalerkinProduct::OverlapVertex<typename G::VertexDescriptor>*
    GalerkinProduct::buildOverlapVertices(const G& graph, const I& pinfo,
                                          const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                          const Set& overlap)
    {
      // count the overlap vertices.
      typedef typename G::ConstVertexIterator ConstIterator;
      typedef typename I::GlobalLookupIndexSet GlobalLookup;
      typedef typename GlobalLookup::IndexPair IndexPair;

      const ConstIterator end = graph.end();
      int overlapCount = 0;

      const GlobalLookup& lookup=pinfo.globalLookup();

      for(ConstIterator vertex=graph.begin(); vertex != end; ++vertex) {
        const IndexPair* pair = lookup.pair(*vertex);

        if(pair!=0 && overlap.contains(pair->local().attribute()))
          ++overlapCount;
      }

      // Allocate space
      typedef typename G::VertexDescriptor Vertex;

      OverlapVertex<Vertex>* overlapVertices = new OverlapVertex<Vertex>[overlapCount];

      // Initialize them
      overlapCount=0;
      for(ConstIterator vertex=graph.begin(); vertex != end; ++vertex) {
        const IndexPair* pair = lookup.pair(*vertex);

        if(pair!=0 && overlap.contains(pair->local().attribute())) {
          overlapVertices[overlapCount].aggregate = aggregates[pair->local()];
          overlapVertices[overlapCount].vertex = pair->local();
          ++overlapCount;
        }
      }

      dinfo << overlapCount<<" overlap vertices"<<std::endl;

      std::sort(overlapVertices, overlapVertices+overlapCount, OVLess<Vertex>());

      overlapStart_ = new std::size_t[graph.maxVertex()];

      std::size_t startIndex = 0;

      Vertex aggregate = graph.maxVertex()+1;

      for(OverlapVertex<Vertex>* vertex=overlapVertices; vertex != overlapVertices+overlapCount;
          ++vertex) {
        if(aggregate != vertex->aggregate) {
          aggregate = vertex->aggregate;
          startIndex=vertex-overlapVertices;
        }
        overlapStart_[vertex->vertex]=startIndex;
      }
      return overlapVertices;
    }

    template<class G, class T>
    template<class V, class O, class R>
    void GalerkinProduct::
    ConnectivityConstructor<G,T>::examine(G& graph,
                                          V& visitedMap,
                                          const T& pinfo,
                                          const AggregatesMap<Vertex>& aggregates,
                                          const O& overlap,
                                          const OverlapVertex<Vertex>* overlapVertices,
                                          R& row)
    {
      typedef typename T::GlobalLookupIndexSet GlobalLookup;
      const GlobalLookup& lookup = pinfo.globalLookup();

      typedef typename G::VertexIterator VertexIterator;

      VertexIterator vend=graph.end();

      // The aggregates owned by the process have lower local indices
      // then those not owned. We process them in the first pass.
      // They represent the rows 0, 1, ..., n of the coarse matrix
      for(VertexIterator vertex = graph.begin(); vertex != vend; ++vertex)
        if(!get(visitedMap, *vertex)) {
          // In the first pass we only process owner nodes
          typedef typename GlobalLookup::IndexPair IndexPair;
          const IndexPair* pair = lookup.pair(*vertex);
          if(pair==0 || !overlap.contains(pair->local().attribute())) {
            constructNonOverlapConnectivity(row, graph, visitedMap, aggregates, *vertex);
            ++row;
          }
        }

      // Now come the aggregates not owned by use.
      // They represent the rows n+1, ..., N
      for(VertexIterator vertex = graph.begin(); vertex != vend; ++vertex)
        if(!get(visitedMap, *vertex)) {

#ifdef ISTL_WITH_CHECKING
          typedef typename GlobalLookup::IndexPair IndexPair;
          const IndexPair* pair = lookup.pair(seed);
          assert(pair!=0 && overlap.contains(pair->local().attribute()));
#endif

          constructOverlapConnectivity(row, graph, visitedMap, aggregates, overlapVertices);
          ++row;
        }
    }

    template<class G>
    template<class V, class R>
    void GalerkinProduct::
    ConnectivityConstructor<G,SequentialInformation>::examine(G& graph,
                                                              V& visitedMap,
                                                              const SequentialInformation& pinfo,
                                                              const AggregatesMap<Vertex>& aggregates,
                                                              R& row)
    {
      typedef typename G::VertexIterator VertexIterator;

      VertexIterator vend=graph.end();
      for(VertexIterator vertex = graph.begin(); vertex != vend; ++vertex) {
        if(!get(visitedMap, *vertex)) {
          constructNonOverlapConnectivity(row, graph, visitedMap, aggregates, *vertex);
          ++row;
        }
      }

    }

    template<class M, class V>
    GalerkinProduct::SparsityBuilder<M,V>::SparsityBuilder(M& matrix, const AggregatesMap<V>& aggregates)
      : row_(matrix.createbegin()), aggregates_(aggregates)
    {}

    template<class M, class V>
    void GalerkinProduct::SparsityBuilder<M,V>::operator++()
    {
      ++row_;
    }

    template<class M, class V>
    void GalerkinProduct::SparsityBuilder<M,V>::insert(const typename M::size_type& index)
    {
      row_.insert(index);
    }

    template<class M, class G, class V, class I, class Set>
    M* GalerkinProduct::build(const M& fine, G& fineGraph, V& visitedMap,
                              const I& pinfo,
                              const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                              const typename M::size_type& size,
                              const Set& overlap)
    {

      typedef OverlapVertex<typename G::VertexDescriptor> OverlapVertex;

      const OverlapVertex* overlapVertices = buildOverlapVertices(fineGraph,
                                                                  pinfo,
                                                                  aggregates,
                                                                  overlap);
      M* coarseMatrix = new M(size, size, M::row_wise);

      // Reset the visited flags of all vertices.
      // As the isolated nodes will be skipped we simply mark them as visited

      typedef typename G::VertexIterator Vertex;
      Vertex vend = fineGraph.end();
      for(Vertex vertex = fineGraph.begin(); vertex != vend; ++vertex) {
        assert(aggregates[*vertex] != AggregatesMap<typename G::VertexDescriptor>::UNAGGREGATED);
        put(visitedMap, *vertex, aggregates[*vertex]==AggregatesMap<typename G::VertexDescriptor>::ISOLATED);
      }

      SparsityBuilder<M,typename G::VertexDescriptor> sparsityBuilder(*coarseMatrix, aggregates);

      ConnectivityConstructor<G,I>::examine(fineGraph, visitedMap, pinfo,
                                            aggregates, overlap, overlapVertices,
                                            sparsityBuilder);

      delete[] overlapVertices;
      delete[] overlapStart_;

      //calculate(fine, aggregates, *coarse, overlap);

      return coarseMatrix;
    }

    template<class M, class G, class V, class Set>
    M* GalerkinProduct::build(const M& fine, G& fineGraph, V& visitedMap,
                              const SequentialInformation& pinfo,
                              const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                              const typename M::size_type& size,
                              const Set& overlap)
    {
      M* coarseMatrix = new M(size, size, M::row_wise);

      // Reset the visited flags of all vertices.
      // As the isolated nodes will be skipped we simply mark them as visited

      typedef typename G::VertexIterator Vertex;
      Vertex vend = fineGraph.end();
      for(Vertex vertex = fineGraph.begin(); vertex != vend; ++vertex) {
        assert(aggregates[*vertex] != AggregatesMap<typename G::VertexDescriptor>::UNAGGREGATED);
        put(visitedMap, *vertex, aggregates[*vertex]==AggregatesMap<typename G::VertexDescriptor>::ISOLATED);
      }

      SparsityBuilder<M,typename G::VertexDescriptor> sparsityBuilder(*coarseMatrix, aggregates);

      ConnectivityConstructor<G,SequentialInformation>::examine(fineGraph, visitedMap, pinfo,
                                                                aggregates, sparsityBuilder);
      return coarseMatrix;
    }

    template<class M, class V>
    void GalerkinProduct::calculate(const M& fine, const AggregatesMap<V>& aggregates, M& coarse)
    {
      coarse = static_cast<typename M::field_type>(0);

      typedef typename M::ConstIterator RowIterator;
      RowIterator endRow = fine.end();

      for(RowIterator row = fine.begin(); row != endRow; ++row)
        if(aggregates[row.index()] != AggregatesMap<V>::ISOLATED) {
          assert(aggregates[row.index()]!=AggregatesMap<V>::UNAGGREGATED);
          //typedef typename RowIterator::Iterator ColIterator;
          typedef typename M::ConstColIterator ColIterator;
          ColIterator endCol = row->end();

          for(ColIterator col = row->begin(); col != endCol; ++col)
            if(aggregates[col.index()] != AggregatesMap<V>::ISOLATED) {
              assert(aggregates[row.index()]!=AggregatesMap<V>::UNAGGREGATED);
              coarse[aggregates[row.index()]][aggregates[col.index()]]+=*col;
            }
        }

      // Set the dirichlet border
      //typedef D::ConstIterator DirichletIterator;
      //DirichletIterator endborder = dirichlet.end();

      int procs;
      MPI_Comm_size(MPI_COMM_WORLD, &procs);

#warning "Galerkin: Process borders should be set to dirichlet borders"
    }

  } // namespace Amg
} // namespace Dune
#endif
