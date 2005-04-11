// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_GALERKIN_HH
#define DUNE_GALERKIN_HH

#include "aggregates.hh"

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
       * @param aggregates The mapping of the fine level unknowns  onto aggregates.
       * @param coarse A reference to a matrix were the resulting coarse martix should
       * be stored
       */
      template<class M, class G, class I, class A, class Set>
      void build(const M& fine, const G& fineGraph, const I& fineIndices, const A aggregates, M& coarse,
                 const Set& overlap);


    private:
      int* overlapStart_;

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
       * @param aggregates The mapping onto the aggregates.
       */
      template<class G, class I, class Set>
      const OverlapVertex<typename G::VertexDescriptor>*
      buildOverlapVertices(const G& graph,  const I& fineIndices,
                           const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                           const Set& overlap);

      /**
       * @brief Deallocate the data structure needed for rebuilding the aggregates in the overlap.
       */
      void freeOverlapAggregates();

      /**
       * @brief Visitor for identifying connected aggregates during a breadthFirstSearch.
       */
      template<class G, class S>
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
         * @brief The vertex descriptor of the graph.
         */
        typedef typename Graph::VertexDescriptor Vertex;

        /**
         * @brief Constructor
         * @param aggregates The mapping of the vertices onto the aggregates.
         * @param connected The set to added the connected aggregates to.
         */
        ConnectedBuilder(const AggregatesMap<Vertex>& aggregates, Set& connected);

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
        /**
         * @brief The set to add the connected vertices to.
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


      /**
       * @brief Construct the connectivity of an aggregate.
       *
         template<class S, class G, class I, class A, class C>
         void constructConnectivity(S& connected, const G& graph, const I& indices,
         const A& aggregates, const typename G::VertexDescriptor& seed,
         const C& overlap);
       */
      template<class S, class G>
      void constructOverlapConnectivity(S& connected, const G& graph,
                                        const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                        const OverlapVertex<typename G::VertexDescriptor>* seed);

      /**
       * @brief Construct the connectivity of an aggregate in the overlap.
       */
      template<class S, class G>
      void constructConnectivity(S& connected, const G& graph,
                                 const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                 const typename G::VertexDescriptor& seed);

      /**
       * @brief Count the number of nonzero blocks of the coarse matrix.
       */
      template<class S, class G, class I, class Set>
      int countNonZeros(S& connected, const G& graph, const I& indices,
                        const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                        const Set& overlap,
                        const OverlapVertex<typename G::VertexDescriptor> overlapVertices);
    };

    template<class S, class G>
    void GalerkinProduct::constructConnectivity(S& connected, const G& graph,
                                                const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                                const typename G::VertexDescriptor& seed)
    {
      connected.put(aggregates[seed]);
      ConnectedBuilder<G,S> conBuilder(aggregates, connected);
      aggregates.breadthFirstSearch(seed,aggregates[seed],graph, conBuilder);
    }

    template<class S, class G>
    void GalerkinProduct::constructOverlapConnectivity(S& connected, const G& graph,
                                                       const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                                       const OverlapVertex<typename G::VertexDescriptor>* seed)
    {
      const typename G::VertexDescriptor aggregate=seed.aggregate;
      connected.put(aggregate);
      ConnectedBuilder<G,S> conBuilder(aggregates, connected);
      while(aggregate != seed.aggregate) {
        // Walk over all neighbours and add them to the connected array.
        visitNeighbours(graph, seed.vertex, conBuilder);
        // Mark vertex as visited
        graph.getVertexProperties(seed.vertex).setVisited();
        ++seed;
      }
    }

    template<class G, class S>
    GalerkinProduct::ConnectedBuilder<G,S>::ConnectedBuilder(const AggregatesMap<Vertex>& aggregates, Set& connected)
      : aggregates_(aggregates), connected_(connected)
    {}


    template<class G, class S>
    void GalerkinProduct::ConnectedBuilder<G,S>::operator()(const ConstEdgeIterator& edge)
    {
      connected_.put(edge.target());
    }

    template<class G, class I, class Set>
    const GalerkinProduct::OverlapVertex<typename G::VertexDescriptor>*
    GalerkinProduct::buildOverlapVertices(const G& graph, const I& fineIndices,
                                          const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                          const Set& overlap)
    {
      // count the overlap vertices.
      typedef typename I::const_iterator ConstIterator;
      const ConstIterator end = fineIndices.end();
      int overlapCount = 0;

      for(ConstIterator pair=fineIndices.begin(); pair != end; ++pair)
        if(overlap.includes(pair.local().attribute()))
          ++overlapCount;

      // Allocate space
      typedef typename G::VertexDescriptor Vertex;

      OverlapVertex<Vertex>* overlapVertices = new OverlapVertex<Vertex>[overlapCount];

      // Initialize them
      overlapCount=0;
      for(ConstIterator pair=fineIndices.begin(); pair != end; ++pair)
        if(overlap.includes(pair.local().attribute())) {
          overlapVertices[overlapCount].aggregate = aggregates[pair.local()];
          overlapVertices[overlapCount].vertex = pair.local();
          ++overlapCount;
        }

      std::sort(overlapVertices, overlapVertices+overlapCount, OVLess<Vertex>());

      overlapStart_ = new std::size_t[graph.maxVertex()];

      std::size_t index = 0;
      std::size_t i=0;

      Vertex aggregate = graph.maxVertex()+1;

      for(OverlapVertex<Vertex>* vertex=overlapVertices; vertex != overlapVertices+overlapCount;
          ++vertex, index++) {
        if(aggregate != vertex.aggregate) {
          aggregate = vertex.aggregate;
          index=i;
        }
        overlapStart_[vertex.vertex]=i;
      }
      return overlapVertices;
    }

    template<class S, class G, class I, class Set>
    int GalerkinProduct::countNonZeros(S& connected, const G& graph, const I& indices,
                                       const AggregatesMap<typename G::VertexDescriptor>& aggregates,
                                       const Set& overlap,
                                       const OverlapVertex<typename G::VertexDescriptor> overlapVertices)
    {
      typedef typename I::const_iterator IndexIterator;
      const IndexIterator end = indices.end();
      int nonzeros = 0;

      for(IndexIterator index=indices.begin(); index != end; ++index) {
        connected.clear();
        if(!graph.getVertexProperties(index.local()).isVisited()) {
          // Mark vertex
          graph.getVertexProperties(index.local()).setVisited();
          if(overlap.includes(index.local().attribute())) {
            costructOverlapConnectivity(connected, graph, aggregates, overlapVertices);
          }else{
            constructConnectivity(connected, graph, aggregates, index);
          }
          nonzeros += connected.size();
        }
      }
      connected.clear();

      return nonzeros;
    }
  } // namespace Amg
} // namespace Dune
#endif
