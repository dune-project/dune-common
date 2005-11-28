// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_AMG_RENUMBERER_HH
#define DUNE_AMG_RENUMBERER_HH

namespace Dune
{
  namespace Amg
  {
    template<class G>
    class AggregateRenumberer
    {
    public:
      /** @brief The vertex type */
      typedef typename G::VertexDescriptor Vertex;

      /**
       * @brief Constructor.
       * @param aggregates The aggregate map to renumber.
       */
      AggregateRenumberer(AggregatesMap<Vertex>& aggregates);

      /** @brief Convertion to vertex. */
      operator Vertex() const;

      void operator()(const typename G::ConstEdgeIterator& edge);

      void operator++();

    protected:
      Vertex number_;
      AggregatesMap<Vertex>& aggregates_;
    };

    template<class G>
    AggregateRenumberer<G>::AggregateRenumberer(AggregatesMap<Vertex>& aggregates)
      :  number_(0), aggregates_(aggregates)
    {}

    template<class G>
    AggregateRenumberer<G>::operator Vertex() const
    {
      return number_;
    }

    template<class G>
    void AggregateRenumberer<G>::operator()(const typename G::ConstEdgeIterator& edge)
    {
      aggregates_[edge.target()]=number_;
    }

    template<class G>
    void AggregateRenumberer<G>::operator++()
    {
      ++number_;
    }

    template<class G, class I, class V>
    void renumberAggregates(const G& graph, I index, I endIndex, V& visitedMap,
                            AggregatesMap<typename G::VertexDescriptor>& aggregates)
    {
      AggregateRenumberer<G> renumberer(aggregates);

      for(I index1=index; index1 != endIndex; ++index1)
        if(aggregates[index1.index()]!=AggregatesMap<typename G::VertexDescriptor>::ISOLATED &&
           !get(visitedMap, index1.index())) {

          aggregates.template breadthFirstSearch<false>(index1.index(), aggregates[index1.index()],
                                                        graph, renumberer, visitedMap);
          aggregates[index1.index()] = renumberer;
          ++renumberer;
        }
      for(index; index != endIndex; ++index)
        put(visitedMap, index.index(), false);
    }

  } // namespace AMG
} // namespace Dune
#endif
