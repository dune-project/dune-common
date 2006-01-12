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

      /**
       * @brief Construct the connectivity of an aggregate.
       *
         template<class S, class G, class I, class A, class C>
         void constructConnectivity(S& connected, const G& graph, const I& indices,
         const A& aggregates, const typename G::VertexDescriptor& seed,
         const C& overlap);
       */
      template<class S, class G, class V>
      void constructOverlapConnectivity(S& connected, G& graph, V& visitedMap,
                                        const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                        const OverlapVertex<typename G::VertexDescriptor>* seed) const;

      /**
       * @brief Construct the connectivity of an aggregate in the overlap.
       */
      template<class S, class G, class V>
      void constructNonOverlapConnectivity(S& connected, G& graph, V& visitedMap,
                                           const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                           const typename G::VertexDescriptor& seed) const;

      template<class S, class G, class V, class O>
      void constructConnectivity(S& connected, G& graph, V& visitedMap,
                                 const SequentialInformation& pinfo,
                                 const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                 const typename G::VertexDescriptor& seed,
                                 const OverlapVertex<typename G::VertexDescriptor>* overlapVertices,
                                 const O& overlapFlags) const;

      template<class S, class G, class V, class P, class O>
      void constructConnectivity(S& connected, G& graph, V& visitedMap,
                                 const P& pinfo,
                                 const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                 const typename G::VertexDescriptor& seed,
                                 const OverlapVertex<typename G::VertexDescriptor>* overlapVertices,
                                 const O& overlapFlags) const;

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
      template<class Set, class M, class V>
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

        /**
         * @brief Examine the connected vertices and set up the sparsity of
         * the current row.
         * @param connected The set of connected vertices.
         *
           void operator()(const Set& connected);
         */
        void insert(const typename M::size_type& index);

        void operator++();

      private:
        /** @brief Create iterator for the current row. */
        typename M::CreateIterator row_;
        /** @brief The aggregates mapping. */
        const AggregatesMap<V>& aggregates_;

      };

      /**
       * @brief Examine all connected aggregates.
       * @param connected Set to store the connected vertices in.
       * @param graph The fine level matrix graph.
       * @param visitedMap The map for marking the vertices as visited.
       * @param pinfo The parallel information.
       * @param overlap The set of flags identifying the overlap vertices.
       * @param overlapVertices helper array for efficient building of overlap aggregates.
       * @param func A functor to examine all connected aggregates of an aggregate.
       */
      template<class S, class G, class V, class I, class Set, class Functor>
      void examineConnectivity(S& connected, G& graph, V& visitedMap, I& pinfo,
                               const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                               const Set& overlap,
                               const OverlapVertex<typename G::VertexDescriptor>* overlapVertices,
                               Functor& func) const;

      /**
       * @brief Initialize the sparsity patteren from the aggregates.
       * @param connected Set to store the connected vertices in.
       * @param graph The fine level matrix graph.
       * @param visitedMap The map for marking the vertices as visited.
       * @param pinfo The parallel information.
       * @param aggregate The aggregate mapping.
       * @param coarseMatrix The coarse matrix.
       * @param overlap The set of flags identifying the overlap vertices.
       * @param overlapVertices helper array for efficient building of overlap aggregates.
       */
      template<class S, class G, class V, class I, class M, class Set>
      void setupSparsityPattern(S& connected, G& graph, V& visitedMap, I& pinfo,
                                const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                M& coarseMatrix,
                                const Set& overlap,
                                const OverlapVertex<typename G::VertexDescriptor>* overlapVertices=0) const;
    };

    template<class S, class G, class V, class P, class O>
    void GalerkinProduct::constructConnectivity(S& connected, G& graph, V& visitedMap,
                                                const P& pinfo,
                                                const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                                const typename G::VertexDescriptor& seed,
                                                const OverlapVertex<typename G::VertexDescriptor>* overlapVertices,
                                                const O& overlap) const
    {
      typedef typename P::GlobalLookupIndexSet GlobalLookup;
      typedef typename GlobalLookup::IndexPair IndexPair;
      const GlobalLookup& lookup = pinfo.globalLookup();
      const IndexPair* pair = lookup.pair(seed);

      if(seed != 0 && overlap.contains(pair->local().attribute())) {
        constructOverlapConnectivity(connected, graph, visitedMap, aggregates, overlapVertices);
      }else{
        constructNonOverlapConnectivity(connected, graph, visitedMap, aggregates, seed);
      }
    }
    template<class S, class G, class V, class O>
    void GalerkinProduct::constructConnectivity(S& connected, G& graph, V& visitedMap,
                                                const SequentialInformation& pinfo,
                                                const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                                const typename G::VertexDescriptor& seed,
                                                const OverlapVertex<typename G::VertexDescriptor>* overlapVertices,
                                                const O& overlap) const
    {
      constructNonOverlapConnectivity(connected, graph, visitedMap, aggregates, seed);
    }
    template<class S, class G, class V>
    void GalerkinProduct::constructNonOverlapConnectivity(S& connected, G& graph, V& visitedMap,
                                                          const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                                          const typename G::VertexDescriptor& seed) const
    {
      connected.insert(aggregates[seed]);
      ConnectedBuilder<G,S,V> conBuilder(aggregates, graph, visitedMap, connected);
      typedef typename G::VertexDescriptor Vertex;
      typedef PoolAllocator<Vertex,100*sizeof(int)> Allocator;
      typedef SLList<Vertex,Allocator> VertexList;
      typedef typename AggregatesMap<Vertex>::DummyEdgeVisitor DummyVisitor;
      VertexList vlist;
      DummyVisitor dummy;
      aggregates.template breadthFirstSearch<true,false>(seed,aggregates[seed], graph, vlist, dummy,
                                                         conBuilder, visitedMap);
    }

    template<class S, class G, class V>
    void GalerkinProduct::constructOverlapConnectivity(S& connected, G& graph, V& visitedMap,
                                                       const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                                       const OverlapVertex<typename G::VertexDescriptor>* seed) const
    {
      const typename G::VertexDescriptor aggregate=seed->aggregate;
      connected.insert(aggregate);
      ConnectedBuilder<G,S,V> conBuilder(aggregates, graph, visitedMap, connected);
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

      std::size_t index = 0;
      std::size_t i=0;

      Vertex aggregate = graph.maxVertex()+1;

      for(OverlapVertex<Vertex>* vertex=overlapVertices; vertex != overlapVertices+overlapCount;
          ++vertex, index++) {
        if(aggregate != vertex->aggregate) {
          aggregate = vertex->aggregate;
          index=i;
        }
        overlapStart_[vertex->vertex]=i;
      }
      return overlapVertices;
    }

    template<class S, class G, class V, class I, class Set, class Functor>
    void GalerkinProduct::examineConnectivity(S& connected, G& graph, V& visitedMap, I& pinfo,
                                              const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                              const Set& overlap,
                                              const OverlapVertex<typename G::VertexDescriptor>* overlapVertices,
                                              Functor& func) const
    {
      // Reset the visited flags of all vertices.
      typedef typename G::VertexIterator Vertex;
      Vertex vend = graph.end();
      for(Vertex vertex = graph.begin(); vertex != vend; ++vertex)
        put(visitedMap, *vertex, false);

      for(Vertex vertex = graph.begin(); vertex != vend; ++vertex) {
        connected.clear();
        if(!get(visitedMap, *vertex)) {
          // Skip isolated vertices
          if(aggregates[*vertex] != AggregatesMap<typename G::VertexDescriptor>::ISOLATED) {
            constructConnectivity(func, graph, visitedMap, pinfo, aggregates, *vertex,
                                  overlapVertices, overlap);
            ++func;
          }else
            put(visitedMap, *vertex, true);
        }
      }
      connected.clear();
    }

    template<class Set, class M, class V>
    GalerkinProduct::SparsityBuilder<Set,M,V>::SparsityBuilder(M& matrix, const AggregatesMap<V>& aggregates)
      : row_(matrix.createbegin()), aggregates_(aggregates)
    {}

    template<class Set, class M, class V>
    void GalerkinProduct::SparsityBuilder<Set,M,V>::operator++()
    {
      ++row_;
    }

    template<class Set, class M, class V>
    void GalerkinProduct::SparsityBuilder<Set,M,V>::insert(const typename M::size_type& index)
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
      std::set<typename G::VertexDescriptor> connected;

      const OverlapVertex* overlapVertices = buildOverlapVertices(fineGraph,
                                                                  pinfo,
                                                                  aggregates,
                                                                  overlap);
      M* coarseMatrix = new M(size, size, M::row_wise);

      setupSparsityPattern(connected, fineGraph, visitedMap, pinfo, aggregates,
                           *coarseMatrix, overlap, overlapVertices);

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
      std::set<typename G::VertexDescriptor> connected;

      M* coarseMatrix = new M(size, size, M::row_wise);

      setupSparsityPattern(connected, fineGraph, visitedMap, pinfo, aggregates, *coarseMatrix, overlap);
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
          //typedef typename RowIterator::Iterator ColIterator;
          typedef typename M::ConstColIterator ColIterator;
          ColIterator endCol = row->end();

          for(ColIterator col = row->begin(); col != endCol; ++col)
            if(aggregates[col.index()] != AggregatesMap<V>::ISOLATED) {
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


    template<class S, class G, class V, class I, class M, class Set>
    void
    GalerkinProduct::setupSparsityPattern(S& connected, G& graph, V& visitedMap, I& pinfo,
                                          const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                          M& coarseMatrix, const Set& overlap,
                                          const OverlapVertex<typename G::VertexDescriptor>* overlapVertices)
    const
    {
      SparsityBuilder<S,M,typename G::VertexDescriptor> sparsityBuilder(coarseMatrix, aggregates);

      examineConnectivity(connected, graph, visitedMap, pinfo, aggregates, overlap, overlapVertices,
                          sparsityBuilder);
    }

  } // namespace Amg
} // namespace Dune
#endif
