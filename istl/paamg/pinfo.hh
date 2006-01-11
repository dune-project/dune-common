// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_AMG_PINFO_HH
#define DUNE_AMG_PINFO_HH

#include <dune/common/collectivecommunication.hh>

#ifdef HAVE_MPI

#include <dune/common/mpicollectivecommunication.hh>
#include <dune/istl/mpitraits.hh>
#include <dune/istl/remoteindices.hh>
#include <dune/istl/interface.hh>
#include <dune/istl/communicator.hh>

#endif

#include <dune/istl/solvercategory.hh>
namespace Dune
{
  namespace Amg
  {

#ifdef HAVE_MPI

    template<class T>
    class ParallelInformation
    {
    public:
      typedef T ParallelIndexSet;
      typedef RemoteIndices<ParallelIndexSet> RemoteIndices;
      typedef Interface<ParallelIndexSet> Interface;
      typedef BufferedCommunicator<ParallelIndexSet>Communicator;
      typedef GlobalLookupIndexSet<ParallelIndexSet> GlobalLookupIndexSet;
      typedef CollectiveCommunication<MPI_Comm> MPICommunicator;

      enum {
        category = SolverCategory::overlapping
      };

      ParallelInformation(const MPI_Comm& comm);

      ~ParallelInformation();

      const MPICommunicator& communicator() const;

      template<bool ignorePublic>
      void rebuildRemoteIndices();

      template<typename OverlapFlags>
      void buildInterface();

      template<typename Data>
      void buildCommunicator(const Data& source, const Data& dest);

      void freeCommunicator();

      template<class GatherScatter, class Data>
      void communicateForward(const Data& source, Data& dest);

      template<class GatherScatter, class Data>
      void communicateBackward(Data& source, const Data& dest);

      ParallelIndexSet& indexSet();

      const ParallelIndexSet& indexSet() const;

      RemoteIndices& remoteIndices();

      const RemoteIndices& remoteIndices() const;

      Interface& interface();

      const Interface& interface() const;

      void buildGlobalLookup(std::size_t);

      void freeGlobalLookup();

      const GlobalLookupIndexSet& globalLookup() const;

    private:
      ParallelIndexSet* indexSet_;
      RemoteIndices* remoteIndices_;
      Interface* interface_;
      Communicator* communicator_;
      MPICommunicator mpiCommunicator_;
      GlobalLookupIndexSet* globalLookup_;
    };

#endif

    class SequentialInformation
    {
    public:
      typedef CollectiveCommunication<void*> MPICommunicator;

      enum {
        category = SolverCategory::sequential
      };

      MPICommunicator communicator() const
      {
        return comm_;
      }

      int procs() const
      {
        return 1;
      }

      template<typename T>
      T globalSum(const T& t) const
      {
        return t;
      }

      SequentialInformation(void*)
      {}

      SequentialInformation()
      {}

      SequentialInformation(const SequentialInformation&)
      {}
    private:
      MPICommunicator comm_;
    };

#ifdef HAVE_MPI
    template<class T>
    ParallelInformation<T>::ParallelInformation(const MPI_Comm& comm)
      : indexSet_(new ParallelIndexSet()),
        remoteIndices_(new RemoteIndices(*indexSet_, *indexSet_, comm)),
        interface_(new Interface()), communicator_(new Communicator()),
        mpiCommunicator_(comm), globalLookup_(0)
    {}

    template<class T>
    ParallelInformation<T>::~ParallelInformation()
    {
      delete communicator_;
      delete interface_;
      delete remoteIndices_;
      delete indexSet_;
    }

    template<class T>
    inline const typename ParallelInformation<T>::MPICommunicator&
    ParallelInformation<T>::communicator() const
    {
      return mpiCommunicator_;
    }

    template<class T>
    template<bool ignorePublic>
    inline void ParallelInformation<T>::rebuildRemoteIndices()
    {
      remoteIndices_->template rebuild<ignorePublic>();
    }

    template<class T>
    template<typename OverlapFlags>
    inline void ParallelInformation<T>::buildInterface()
    {
      interface_->build(*remoteIndices_, NegateSet<OverlapFlags>(),
                        OverlapFlags());
    }


    template<class T>
    template<typename Data>
    inline void ParallelInformation<T>::buildCommunicator(const Data& source,
                                                          const Data& dest)
    {
      communicator_->build(source, dest, *interface_);
    }


    template<class T>
    inline void ParallelInformation<T>::freeCommunicator()
    {
      communicator_->free();
    }

    template<class T>
    template<class GatherScatter, class Data>
    inline void ParallelInformation<T>::communicateForward(const Data& source, Data& dest)
    {
      communicator_->template forward<GatherScatter>(source, dest);
    }

    template<class T>
    template<class GatherScatter, class Data>
    inline void ParallelInformation<T>::communicateBackward(Data& source, const Data& dest)
    {
      communicator_->template backward<GatherScatter>(source, dest);
    }

    template<class T>
    typename ParallelInformation<T>::ParallelIndexSet& ParallelInformation<T>::indexSet(){
      return *indexSet_;
    }

    template<class T>
    const typename ParallelInformation<T>::ParallelIndexSet& ParallelInformation<T>::indexSet() const {
      return *indexSet_;
    }

    template<class T>
    typename ParallelInformation<T>::RemoteIndices& ParallelInformation<T>::remoteIndices(){
      return *remoteIndices_;
    }

    template<class T>
    const typename ParallelInformation<T>::RemoteIndices& ParallelInformation<T>::remoteIndices() const {
      return *remoteIndices_;
    }

    template<class T>
    typename ParallelInformation<T>::Interface& ParallelInformation<T>::interface(){
      return *interface_;
    }

    template<class T>
    const typename ParallelInformation<T>::Interface& ParallelInformation<T>::interface() const {
      return *interface_;
    }

    template<class T>
    void ParallelInformation<T>::buildGlobalLookup(std::size_t size)
    {
      globalLookup_ = new GlobalLookupIndexSet(*indexSet_, size);
    }

    template<class T>
    void ParallelInformation<T>::freeGlobalLookup()
    {
      delete globalLookup_;
      globalLookup_=0;
    }

    template<class T>
    const typename ParallelInformation<T>::GlobalLookupIndexSet&
    ParallelInformation<T>::globalLookup() const
    {
      assert(globalLookup_ != 0);
      return *globalLookup_;
    }

#endif

  } // namespace Amg
} //namespace Dune
#endif
