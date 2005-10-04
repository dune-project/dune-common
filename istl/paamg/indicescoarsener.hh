// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_AMG_INDICESCOARSENER_HH
#define DUNE_AMG_INDICESCOARSENER_HH

#include <dune/istl/indicessyncer.hh>
#include <vector>
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

    template<typename E,typename T>
    class IndicesCoarsener
    {
    public:
      /**
       * @brief The set of excluded attributes
       */
      typedef E ExcludedAttributes;

      /**
       * @brief The type of the index set.
       */
      typedef T ParallelIndexSet;

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
      typedef RemoteIndices<T> RemoteIndices;

      template<typename Graph, typename VM>
      static void coarsen(const ParallelIndexSet& fineIndices,
                          const RemoteIndices& fineRemote,
                          Graph& fineGraph,
                          VM& visitedMap,
                          AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                          ParallelIndexSet& coarseIndices,
                          RemoteIndices& coarseRemote);

    private:
      template<typename G>
      class AggregateRenumberer
      {
        typedef typename G::VertexDescriptor Vertex;

      public:
        AggregateRenumberer(AggregatesMap<Vertex>& aggregates)
          :  number_(0), isPublic_(false), aggregates_(aggregates)
        {}

        void operator()(const typename G::ConstEdgeIterator& edge)
        {
          aggregates_[edge.target()]=number_;
        }

        size_t operator()(const GlobalIndex& global)
        {
          size_t current = number_;
          ++number_;
          return current;
        }

        void operator++()
        {
          ++number_;
        }

        operator size_t()
        {
          return number_;
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
        size_t number_;
        bool isPublic_;
        Attribute attribute_;
        AggregatesMap<Vertex>& aggregates_;

      };


      template<typename Graph, typename VM>
      static void buildCoarseIndexSet(const ParallelIndexSet& fineIndices,
                                      Graph& fineGraph,
                                      VM& visitedMap,
                                      AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                      ParallelIndexSet& coarseIndices,
                                      AggregateRenumberer<Graph>& renumberer);
      template<typename Graph>
      static void buildCoarseRemoteIndices(const RemoteIndices& fineRemote,
                                           const AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                           ParallelIndexSet& coarseIndices,
                                           RemoteIndices& coarseRemote,
                                           AggregateRenumberer<Graph>& renumberer);

    };

    template<typename E,typename T>
    template<typename Graph, typename VM>
    void IndicesCoarsener<E,T>::coarsen(const ParallelIndexSet& fineIndices,
                                        const RemoteIndices& fineRemote,
                                        Graph& fineGraph,
                                        VM& visitedMap,
                                        AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                        ParallelIndexSet& coarseIndices,
                                        RemoteIndices& coarseRemote)
    {
      AggregateRenumberer<Graph> renumberer(aggregates);
      buildCoarseIndexSet(fineIndices, fineGraph, visitedMap, aggregates, coarseIndices, renumberer);
      buildCoarseRemoteIndices(fineRemote, aggregates, coarseIndices, coarseRemote, renumberer);
    }

    template<typename E,typename T>
    template<typename Graph, typename VM>
    void IndicesCoarsener<E,T>::buildCoarseIndexSet(const ParallelIndexSet& fineIndices,
                                                    Graph& fineGraph,
                                                    VM& visitedMap,
                                                    AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                                    ParallelIndexSet& coarseIndices,
                                                    AggregateRenumberer<Graph>& renumberer)
    {
      typedef typename ParallelIndexSet::const_iterator Iterator;
      typedef typename Graph::VertexDescriptor Vertex;

      Iterator end = fineIndices.end();

      coarseIndices.beginResize();

      bool visited = false;
      for(Iterator index = fineIndices.begin(); index != end; ++index)
        if(fineGraph.getVertexProperties(index->local()).visited()) {
          std::cerr<<*index<<" is visited!"<<std::endl;
          visited=true;
        }

      if(visited)
        throw visited;

      // Setup the coarse index set and renumber the aggregate consecutively
      // ascending from zero according to the minimum global index belonging
      // to the aggregate
      for(Iterator index = fineIndices.begin(); index != end; ++index) {
        if(!ExcludedAttributes::contains(index->local().attribute()) && !fineGraph.getVertexProperties(index->local()).visited()) {
          renumberer.reset();
          renumberer.attribute(index->local().attribute());
          renumberer.isPublic(index->local().isPublic());

          // Reconstruct aggregate and mark vertices as visited
          aggregates.template breadthFirstSearch<false>(index->local(), aggregates[index->local()],
                                                        fineGraph, renumberer, visitedMap);
          aggregates[index->local()] = renumberer;
          coarseIndices.add(index->global(),
                            LocalIndex(renumberer, renumberer.attribute(),
                                       renumberer.isPublic()));
          ++renumberer;
        }
      }

      coarseIndices.endResize();

      assert(coarseIndices.size()==renumberer);
      assert((coarseIndices.begin()+(coarseIndices.size()-1))->local()==renumberer-1);

      // Reset the visited flags
      typedef typename Graph::ConstVertexIterator VertexIterator;
      VertexIterator vend = fineGraph.end();

      for(VertexIterator vertex=fineGraph.begin(); vertex != vend; ++vertex)
        fineGraph.getVertexProperties(*vertex).resetVisited();
    }

    template<typename E, typename T>
    template<typename Graph>
    void IndicesCoarsener<E,T>::buildCoarseRemoteIndices(const RemoteIndices& fineRemote,
                                                         const AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                                         ParallelIndexSet& coarseIndices,
                                                         RemoteIndices& coarseRemote,
                                                         AggregateRenumberer<Graph>& renumberer)
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
        typedef RemoteIndexListModifier<T,false> Modifier;
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


  } //namespace Amg
} // namespace Dune
#endif
