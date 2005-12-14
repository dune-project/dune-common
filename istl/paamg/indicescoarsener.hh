// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_AMG_INDICESCOARSENER_HH
#define DUNE_AMG_INDICESCOARSENER_HH

#include <dune/istl/indicessyncer.hh>
#include <vector>
#include "renumberer.hh"

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
     * @brief Provides a class for building the index set
     * and remote indices on the coarse level.
     */

    template<typename T, typename E>
    class IndicesCoarsener
    {
    public:
      /**
       * @brief The set of excluded attributes
       */
      typedef E ExcludedAttributes;

      /**
       * @brief The type of the parallel information.
       */
      typedef T ParallelInformation;

      typedef typename ParallelInformation::IndexSet ParallelIndexSet;

      /**
       * @brief The type of the global index.
       */
      typedef typename ParallelIndexSet::GlobalIndex GlobalIndex;

      /**
       * @brief The type of the local index.
       */
      typedef typename ParallelIndexSet::LocalIndex LocalIndex;

      /**
       * @brief The type of the attribute.
       */
      typedef typename LocalIndex::Attribute Attribute;

      /**
       * @brief The type of the remote indices.
       */
      typedef RemoteIndices<ParallelIndexSet> RemoteIndices;

      template<typename Graph, typename VM>
      static typename Graph::VertexDescriptor
      coarsen(ParallelInformation& fineInfo,
              Graph& fineGraph,
              VM& visitedMap,
              AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
              ParallelInformation& coarseInfo);

    private:
      template<typename G>
      class ParallelAggregateRenumberer : public AggregateRenumberer<G>
      {
        typedef typename G::VertexDescriptor Vertex;

      public:
        ParallelAggregateRenumberer(AggregatesMap<Vertex>& aggregates)
          :  AggregateRenumberer<G>(aggregates), isPublic_(false)
        {}


        void operator()(const typename G::ConstEdgeIterator& edge)
        {
          AggregateRenumberer<G>::operator()(edge);
        }

        Vertex operator()(const GlobalIndex& global)
        {
          Vertex current = this->number_;
          this->operator++();
          return current;
        }

        bool isPublic()
        {
          return isPublic_;
        }

        void isPublic(bool b)
        {
          isPublic_ = isPublic_ || b;
        }

        void reset()
        {
          isPublic_=false;
        }

        void attribute(const Attribute& attribute)
        {
          attribute_=attribute;
        }

        Attribute attribute()
        {
          return attribute_;
        }

      private:
        bool isPublic_;
        Attribute attribute_;
      };

      template<typename Graph, typename VM>
      static void buildCoarseIndexSet(const ParallelInformation& pinfo,
                                      Graph& fineGraph,
                                      VM& visitedMap,
                                      AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                      ParallelIndexSet& coarseIndices,
                                      ParallelAggregateRenumberer<Graph>& renumberer);

      template<typename Graph>
      static void buildCoarseRemoteIndices(const RemoteIndices& fineRemote,
                                           const AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                           ParallelIndexSet& coarseIndices,
                                           RemoteIndices& coarseRemote,
                                           ParallelAggregateRenumberer<Graph>& renumberer);

    };

    /**
     * @brief Coarsen Indices in the sequential case.
     *
     * Nothing to be coarsened here. Just renumber the aggregates
     * consecutively
     */
    template<typename E>
    class IndicesCoarsener<SequentialInformation,E>
    {
    public:
      template<typename Graph, typename VM>
      static typename Graph::VertexDescriptor
      coarsen(const SequentialInformation& fineInfo,
              Graph& fineGraph,
              VM& visitedMap,
              AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
              SequentialInformation& coarseInfo);
    };

    template<typename T, typename E>
    template<typename Graph, typename VM>
    inline typename Graph::VertexDescriptor
    IndicesCoarsener<T,E>::coarsen(ParallelInformation& fineInfo,
                                   Graph& fineGraph,
                                   VM& visitedMap,
                                   AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                   ParallelInformation& coarseInfo)
    {
      ParallelAggregateRenumberer<Graph> renumberer(aggregates);
      fineInfo.buildGlobalLookup(fineGraph.noVertices());
      buildCoarseIndexSet(fineInfo, fineGraph, visitedMap, aggregates,
                          coarseInfo.indexSet(), renumberer);
      buildCoarseRemoteIndices(fineInfo.remoteIndices(), aggregates, coarseInfo.indexSet(),
                               coarseInfo.remoteIndices(), renumberer);

      return renumberer;
    }

    template<typename T, typename E>
    template<typename Graph, typename VM>
    void IndicesCoarsener<T,E>::buildCoarseIndexSet(const ParallelInformation& pinfo,
                                                    Graph& fineGraph,
                                                    VM& visitedMap,
                                                    AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                                    ParallelIndexSet& coarseIndices,
                                                    ParallelAggregateRenumberer<Graph>& renumberer)
    {
      typedef typename Graph::VertexDescriptor Vertex;
      typedef typename Graph::ConstVertexIterator Iterator;
      typedef typename ParallelInformation::GlobalLookupIndexSet GlobalLookup;

      Iterator end = fineGraph.end();
      const GlobalLookup& lookup = pinfo.globalLookup();

      coarseIndices.beginResize();

      // Setup the coarse index set and renumber the aggregate consecutively
      // ascending from zero according to the minimum global index belonging
      // to the aggregate
      for(Iterator index = fineGraph.begin(); index != end; ++index) {
        if(aggregates[*index]!=AggregatesMap<typename Graph::VertexDescriptor>::ISOLATED)
          if(!get(visitedMap, *index)) {

            typedef typename GlobalLookup::IndexPair IndexPair;
            const IndexPair* pair= lookup.pair(*index);

            renumberer.reset();
            if(pair==0) {
              // Reconstruct aggregate and mark vertices as visited
              aggregates.template breadthFirstSearch<false>(*index, aggregates[*index],
                                                            fineGraph, renumberer, visitedMap);
            }
            else if(!ExcludedAttributes::contains(pair->local().attribute())) {
              renumberer.attribute(pair->local().attribute());
              renumberer.isPublic(pair->local().isPublic());

              // Reconstruct aggregate and mark vertices as visited
              aggregates.template breadthFirstSearch<false>(*index, aggregates[*index],
                                                            fineGraph, renumberer, visitedMap);
              coarseIndices.add(pair->global(),
                                LocalIndex(renumberer, renumberer.attribute(),
                                           renumberer.isPublic()));
            }

            aggregates[*index] = renumberer;
          }
      }

      coarseIndices.endResize();

      // Reset the visited flags
      for(Iterator vertex=fineGraph.begin(); vertex != end; ++vertex)
        put(visitedMap, *vertex, false);
    }

    template<typename T, typename E>
    template<typename Graph>
    void IndicesCoarsener<T,E>::buildCoarseRemoteIndices(const RemoteIndices& fineRemote,
                                                         const AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                                         ParallelIndexSet& coarseIndices,
                                                         RemoteIndices& coarseRemote,
                                                         ParallelAggregateRenumberer<Graph>& renumberer)
    {
      std::vector<char> attributes(coarseIndices.size());

      typedef typename RemoteIndices::const_iterator Iterator;
      Iterator end = fineRemote.end();

      for(Iterator neighbour = fineRemote.begin();
          neighbour != end; ++neighbour) {
        int process = neighbour->first;

        assert(neighbour->second.first==neighbour->second.second);

        // Mark all as not known
        for(size_t i=0; i < coarseIndices.size(); i++)
          attributes[i] = std::numeric_limits<char>::max();

        typedef typename RemoteIndices::RemoteIndexList::const_iterator
        Iterator;
        Iterator riEnd = neighbour->second.second->end();
        for(Iterator index = neighbour->second.second->begin();
            index != riEnd; ++index) {
          if(!E::contains(index->localIndexPair().local().attribute()))
          {
            assert(aggregates[index->localIndexPair().local()]<(int)attributes.size());
            assert(attributes[aggregates[index->localIndexPair().local()]] == std::numeric_limits<char>::max()
                   || attributes[aggregates[index->localIndexPair().local()]] == index->attribute());
            attributes[aggregates[index->localIndexPair().local()]] = index->attribute();
          }
        }

        // Build remote index list
        typedef RemoteIndexListModifier<ParallelIndexSet,false> Modifier;
        typedef typename RemoteIndices::RemoteIndex RemoteIndex;
        typedef typename ParallelIndexSet::const_iterator IndexIterator;

        Modifier coarseList = coarseRemote.template getModifier<false,true>(process);

        IndexIterator iend = coarseIndices.end();
        int i=0;
        for(IndexIterator index = coarseIndices.begin(); index != iend; ++index, ++i)
          if(attributes[i] != std::numeric_limits<char>::max()) {
            // remote index is present
            coarseList.insert(RemoteIndex(Attribute(attributes[i]), &(*index)));
          }
      }

      // The number of neighbours should not change!
      assert(coarseRemote.neighbours()==fineRemote.neighbours());

      // snyc the index set and the remote indices to recompute missing
      // indices
      IndicesSyncer<ParallelIndexSet> syncer(coarseIndices, coarseRemote);
      syncer.sync(renumberer);

    }

    template<typename E>
    template<typename Graph, typename VM>
    typename Graph::VertexDescriptor
    IndicesCoarsener<SequentialInformation,E>::coarsen(const SequentialInformation& fineInfo,
                                                       Graph& fineGraph,
                                                       VM& visitedMap,
                                                       AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                                       SequentialInformation& coarseInfo)
    {
      typedef typename Graph::VertexDescriptor Vertex;
      AggregateRenumberer<Graph> renumberer(aggregates);
      typedef typename Graph::VertexIterator Iterator;

      for(Iterator vertex=fineGraph.begin(), endVertex=fineGraph.end();
          vertex != endVertex; ++vertex)
        if(aggregates[*vertex]!=AggregatesMap<Vertex>::ISOLATED &&
           !get(visitedMap, *vertex)) {

          aggregates.template breadthFirstSearch<false>(*vertex, aggregates[*vertex],
                                                        fineGraph, renumberer, visitedMap);
          aggregates[*vertex] = renumberer;
          ++renumberer;
        }

      for(Iterator vertex=fineGraph.begin(), endVertex=fineGraph.end();
          vertex != endVertex; ++vertex)
        put(visitedMap, *vertex, false);

      return renumberer;
    }

  } //namespace Amg
} // namespace Dune
#endif
