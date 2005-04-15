// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef INDICESCOARSENER_HH
#define INDICESCOARSENER_HH

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

    template<typename E,typename TG, typename TA, int N>
    class IndicesCoarsener
    {
    public:
      /**
       * @brief The set of excluded attributes
       */
      typedef E ExcludedAttributes;

      /**
       * @brief The type of the global index.
       */
      typedef TG GlobalIndex;

      /**
       * @brief The type of the attribute.
       */
      typedef TA Attribute;

      /**
       * @brief The type of the local index.
       */
      typedef ParallelLocalIndex<TA> LocalIndex;
      /**
       * @brief The type of the index set.
       */
      typedef IndexSet<GlobalIndex,LocalIndex,N> IndexSet;
      /**
       * @brief The type of the remote indices.
       */
      typedef RemoteIndices<GlobalIndex,Attribute,N> RemoteIndices;

      template<typename Graph>
      static void coarsen(const IndexSet& fineIndices,
                          const RemoteIndices& fineRemote,
                          Graph& fineGraph,
                          AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                          IndexSet& coarseIndices,
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

        size_t operator()(const TG& global)
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

        void attribute(const TA& attribute)
        {
          attribute_=attribute;
        }

        TA attribute()
        {
          return attribute_;
        }

      private:
        size_t number_;
        bool isPublic_;
        TA attribute_;
        AggregatesMap<Vertex>& aggregates_;

      };


      template<typename Graph>
      static void buildCoarseIndexSet(const IndexSet& fineIndices,
                                      Graph& fineGraph,
                                      AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                      IndexSet& coarseIndices,
                                      AggregateRenumberer<Graph>& renumberer);
      template<typename Graph>
      static void buildCoarseRemoteIndices(const RemoteIndices& fineRemote,
                                           const AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                           IndexSet& coarseIndices,
                                           RemoteIndices& coarseRemote,
                                           AggregateRenumberer<Graph>& renumberer);

    };

    template<typename E,typename TG, typename TA, int N>
    template<typename Graph>
    void IndicesCoarsener<E,TG,TA,N>::coarsen(const IndexSet& fineIndices,
                                              const RemoteIndices& fineRemote,
                                              Graph& fineGraph,
                                              AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                              IndexSet& coarseIndices,
                                              RemoteIndices& coarseRemote)
    {
      AggregateRenumberer<Graph> renumberer(aggregates);
      buildCoarseIndexSet(fineIndices, fineGraph, aggregates, coarseIndices, renumberer);
      buildCoarseRemoteIndices(fineRemote, aggregates, coarseIndices, coarseRemote, renumberer);
    }

    template<typename E,typename TG, typename TA, int N>
    template<typename Graph>
    void IndicesCoarsener<E,TG,TA,N>::buildCoarseIndexSet(const IndexSet& fineIndices,
                                                          Graph& fineGraph,
                                                          AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                                          IndexSet& coarseIndices,
                                                          AggregateRenumberer<Graph>& renumberer)
    {
      typedef typename IndexSet::const_iterator Iterator;
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
      // starting from zero according to the minimum global index belonging
      // to the aggregate
      for(Iterator index = fineIndices.begin(); index != end; ++index) {
        if(!ExcludedAttributes::contains(index->local().attribute()) && !fineGraph.getVertexProperties(index->local()).visited()) {
          renumberer.reset();
          renumberer.attribute(index->local().attribute());
          renumberer.isPublic(index->local().isPublic());

          // Reconstruct aggregate and mark vertices as visited
          aggregates.template breadthFirstSearch<false>(index->local(), aggregates[index->local()],
                                                        fineGraph, renumberer);
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

    template<typename E, typename TG, typename TA, int N>
    template<typename Graph>
    void IndicesCoarsener<E,TG,TA,N>::buildCoarseRemoteIndices(const RemoteIndices& fineRemote,
                                                               const AggregatesMap<typename Graph::VertexDescriptor>& aggregates,
                                                               IndexSet& coarseIndices,
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
            assert(aggregates[index->localIndexPair().local()]<attributes.size());
            assert(attributes[aggregates[index->localIndexPair().local()]] == std::numeric_limits<char>::max()
                   || attributes[aggregates[index->localIndexPair().local()]] == index->attribute());
            attributes[aggregates[index->localIndexPair().local()]] = index->attribute();
          }
        }

        // Build remote index list
        typedef RemoteIndexListModifier<TG,TA,N,false> Modifier;
        typedef typename RemoteIndices::RemoteIndexType RemoteIndex;
        typedef typename IndexSet::const_iterator IndexIterator;

        Modifier coarseList = coarseRemote.template getModifier<false,true>(process);

        IndexIterator iend = coarseIndices.end();
        int i=0;
        for(IndexIterator index = coarseIndices.begin(); index != iend; ++index, ++i)
          if(attributes[i] != std::numeric_limits<char>::max()) {
            // remote index is present
            coarseList.insert(RemoteIndex(TA(attributes[i]), &(*index)));
          }
      }

      // The number of neighbours should not change!
      assert(coarseRemote.neighbours()==fineRemote.neighbours());

      // snyc the index set and the remote indices to recompute missing
      // indices
      IndicesSyncer<TG,TA,N> syncer(coarseIndices, coarseRemote);
      syncer.sync(renumberer);
    }


  } //namespace Amg
} // namespace Dune
#endif
