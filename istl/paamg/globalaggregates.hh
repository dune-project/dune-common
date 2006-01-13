// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef GLOBALAGGREGATES_HH
#define GLOBALAGGREGATES_HH

#include "aggregates.hh"
#include <dune/istl/indexset.hh>
#include <dune/istl/communicator.hh>
namespace Dune
{
  namespace Amg
  {

    template<typename T, typename TI>
    struct GlobalAggregatesMap
    {
    public:
      typedef TI ParallelIndexSet;

      typedef typename ParallelIndexSet::GlobalIndex GlobalIndex;

      typedef typename ParallelIndexSet::GlobalIndex IndexedType;

      typedef typename ParallelIndexSet::LocalIndex LocalIndex;

      typedef T Vertex;

      GlobalAggregatesMap(AggregatesMap<Vertex>& aggregates,
                          const GlobalLookupIndexSet<ParallelIndexSet>& indexset)
        : aggregates_(aggregates), indexset_(indexset)
      {}

      inline const GlobalIndex& operator[](std::size_t index) const
      {
        const Vertex& aggregate = aggregates_[index];
        const Dune::IndexPair<GlobalIndex,LocalIndex >* pair = indexset_.pair(aggregate);
        assert(pair!=0);
        return pair->global();
      }


      class Proxy
      {
      public:
        Proxy(const GlobalLookupIndexSet<ParallelIndexSet>& indexset, Vertex& aggregate)
          : indexset_(&indexset), aggregate_(&aggregate)
        {}

        Proxy& operator=(const GlobalIndex& global)
        {
          *aggregate_ = indexset_->operator[](global).local();
          return *this;
        }
      private:
        const GlobalLookupIndexSet<ParallelIndexSet>* indexset_;
        Vertex* aggregate_;
      };

      inline Proxy operator[](std::size_t index)
      {
        return Proxy(indexset_, aggregates_[index]);
      }

      inline void put(const GlobalIndex& global, size_t i)
      {
        aggregates_[i]=indexset_[global].local();

      }

    private:
      AggregatesMap<Vertex>& aggregates_;
      const GlobalLookupIndexSet<ParallelIndexSet>& indexset_;
    };

    template<typename T, typename TI>
    struct AggregatesGatherScatter
    {
      typedef TI ParallelIndexSet;
      typedef typename ParallelIndexSet::GlobalIndex GlobalIndex;

      static const GlobalIndex& gather(const GlobalAggregatesMap<T,TI>& ga, size_t i)
      {
        return ga[i];
      }

      static void scatter(GlobalAggregatesMap<T,TI>& ga, GlobalIndex global, size_t i)
      {
        ga.put(global, i);
      }
    };

    template<typename T, typename O, typename I>
    struct AggregatesPublisher
    {};

    template<typename T, typename O, typename T1>
    struct AggregatesPublisher<T,O,ParallelInformation<T1> >
    {
      typedef T Vertex;
      typedef O OverlapFlags;
      typedef ParallelInformation<T1> ParallelInformation;
      typedef typename ParallelInformation::ParallelIndexSet IndexSet;

      static void publish(AggregatesMap<Vertex>& aggregates,
                          ParallelInformation& pinfo,
                          std::size_t size)
      {
        typedef Dune::Amg::GlobalAggregatesMap<Vertex,IndexSet> GlobalMap;
        pinfo.buildGlobalLookup(size);
        GlobalMap gmap(aggregates, pinfo.globalLookup());
        pinfo.template buildInterface<OverlapFlags>();
        pinfo.template buildCommunicator<GlobalMap>(gmap, gmap);
        pinfo.template communicateForward<AggregatesGatherScatter<Vertex,IndexSet> >(gmap, gmap);
        pinfo.freeCommunicator();
      }

    };

    // forward declaration
    template<class T1, class T2>
    class OwnerOverlapCopyCommunication;

    template<typename T, typename O, typename T1, typename T2>
    struct AggregatesPublisher<T,O,OwnerOverlapCopyCommunication<T1,T2> >
    {
      typedef T Vertex;
      typedef O OverlapFlags;
      typedef OwnerOverlapCopyCommunication<T1,T2> ParallelInformation;
      typedef typename ParallelInformation::ParallelIndexSet IndexSet;

      static void publish(AggregatesMap<Vertex>& aggregates,
                          ParallelInformation& pinfo,
                          std::size_t size)
      {
        typedef Dune::Amg::GlobalAggregatesMap<Vertex,IndexSet> GlobalMap;
        pinfo.buildGlobalLookup(size);
        GlobalMap gmap(aggregates, pinfo.globalLookup());
        pinfo.copyOwnerToAll(gmap,gmap);
      }

    };

    template<typename T, typename O>
    struct AggregatesPublisher<T,O,SequentialInformation>
    {
      typedef T Vertex;
      typedef SequentialInformation ParallelInformation;

      static void publish(AggregatesMap<Vertex>& aggregates,
                          ParallelInformation& pinfo,
                          std::size_t size)
      {}
    };

  } // end Amg namespace


  template<typename T, typename TI>
  struct CommPolicy<Amg::GlobalAggregatesMap<T,TI> >
  {
    typedef Amg::AggregatesMap<T> Type;
    typedef typename Amg::GlobalAggregatesMap<T,TI>::IndexedType IndexedType;
    typedef SizeOne IndexedTypeFlag;
    static int getSize(const Type&, int)
    {
      return 1;
    }
  };

} // end Dune namespace

#endif
