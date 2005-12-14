// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_OWNERCOPY_HH
#define DUNE_OWNERCOPY_HH

#include <new>
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <set>

#include "math.h"

// MPI header
#if HAVE_MPI
#include <mpi.h>
#endif


#include "dune/common/tripel.hh"
#include <dune/common/enumset.hh>
#include <dune/common/collectivecommunication.hh>

#include "indexset.hh"
#include "communicator.hh"
#include "remoteindices.hh"
#include "istlexception.hh"

namespace Dune {

  /**
     @addtogroup ISTL
     @{
   */

  /**
   * @brief A class setting up standard communication for a two-valued
     attribute set with owner/overlap/copy semantics.
   */

  class OwnerOverlapCopyAttributeSet
  {
    enum AttributeSet { owner=1, overlap=2, copy=0 };
  };

  template <class GlobalIdType, class LocalIdType>
  class IndexInfoFromGrid
  {
  public:
    typedef tripel<GlobalIdType,LocalIdType,int> IndexTripel;
    typedef tripel<int,GlobalIdType,int> RemoteIndexTripel;

    void addLocalIndex (const IndexTripel& x)
    {
      if (x.third!=OwnerOverlapCopyAttributeSet::owner &&
          x.third!=OwnerOverlapCopyAttributeSet::overlap &&
          x.third!=OwnerOverlapCopyAttributeSet::copy)
        DUNE_THROW(ISTLError,"OwnerOverlapCopyCommunication: global index not in index set");
      localindices.insert(x);
    }
    void addRemoteIndex (const RemoteIndexTripel& x)
    {
      if (x.third!=OwnerOverlapCopyAttributeSet::owner &&
          x.third!=OwnerOverlapCopyAttributeSet::overlap &&
          x.third!=OwnerOverlapCopyAttributeSet::copy)
        DUNE_THROW(ISTLError,"OwnerOverlapCopyCommunication: global index not in index set");
      remoteindices.insert(x);
    }

    const std::set<IndexTripel>& localIndices () const
    {
      return localindices;
    }

    const std::set<RemoteIndexTripel>& remoteIndices () const
    {
      return remoteindices;
    }

    void clear ()
    {
      localindices.clear();
      remoteindices.clear();
    }

  private:
    std::set<IndexTripel> localindices;
    std::set<RemoteIndexTripel> remoteindices;
  };


#if HAVE_MPI

  // set up communication from known distribution with owner/overlap/copy semantics
  template <class GlobalIdType, class LocalIdType>
  class OwnerOverlapCopyCommunication
  {
    // used types
    typedef typename IndexInfoFromGrid<GlobalIdType,LocalIdType>::IndexTripel IndexTripel;
    typedef typename IndexInfoFromGrid<GlobalIdType,LocalIdType>::RemoteIndexTripel RemoteIndexTripel;
    typedef typename std::set<IndexTripel>::const_iterator localindex_iterator;
    typedef typename std::set<RemoteIndexTripel>::const_iterator remoteindex_iterator;
    enum AttributeSet { owner=OwnerOverlapCopyAttributeSet::owner,
                        overlap=OwnerOverlapCopyAttributeSet::overlap,
                        copy=OwnerOverlapCopyAttributeSet::copy };
    typedef ParallelLocalIndex<AttributeSet> LI;
    typedef ParallelIndexSet<GlobalIdType,LI,512> PIS;
    typedef RemoteIndices<PIS> RI;
    typedef RemoteIndexListModifier<PIS,false> RILM;
    typedef typename RI::RemoteIndex RX;
    typedef BufferedCommunicator<PIS> BC;
    typedef Interface<PIS> IF;

    // gather/scatter callback for communcation
    template<typename T>
    struct CopyGatherScatter
    {
      typedef typename CommPolicy<T>::IndexedType V;

      static V gather(const T& a, int i)
      {
        return a[i];
      }

      static void scatter(T& a, V v, int i)
      {
        a[i] = v;
      }
    };
    template<typename T>
    struct AddGatherScatter
    {
      typedef typename CommPolicy<T>::IndexedType V;

      static V gather(const T& a, int i)
      {
        return a[i];
      }

      static void scatter(T& a, V v, int i)
      {
        a[i] += v;
      }
    };

    void buildOwnerOverlapToAllInterface () const
    {
      if (OwnerOverlapToAllInterfaceBuilt)
        OwnerOverlapToAllInterface.free();
      typedef Combine<EnumItem<AttributeSet,owner>,EnumItem<AttributeSet,overlap>,AttributeSet> OwnerOverlapSet;
      typedef Combine<OwnerOverlapSet,EnumItem<AttributeSet,copy>,AttributeSet> AllSet;
      OwnerOverlapSet sourceFlags;
      AllSet destFlags;
      OwnerOverlapToAllInterface.build(ri,sourceFlags,destFlags);
      OwnerOverlapToAllInterfaceBuilt = true;
    }

    void buildOwnerToAllInterface () const
    {
      if (OwnerToAllInterfaceBuilt)
        OwnerToAllInterface.free();
      typedef EnumItem<AttributeSet,owner> OwnerSet;
      typedef Combine<EnumItem<AttributeSet,owner>,EnumItem<AttributeSet,overlap>,AttributeSet> OwnerOverlapSet;
      typedef Combine<OwnerOverlapSet,EnumItem<AttributeSet,copy>,AttributeSet> AllSet;
      OwnerOverlapSet sourceFlags;
      AllSet destFlags;
      OwnerToAllInterface.build(ri,sourceFlags,destFlags);
      OwnerToAllInterfaceBuilt = true;
    }

  public:

    template<class T>
    void copyOwnerToAll (const T& source, T& dest) const
    {
      if (!OwnerToAllInterfaceBuilt)
        buildOwnerToAllInterface ();
      BC communicator;
      communicator.template build<T>(OwnerToAllInterface);
      communicator.template forward<CopyGatherScatter<T> >(source,dest);
      communicator.free();
    }

    template<class T>
    void addOwnerOverlapToAll (const T& source, T& dest) const
    {
      if (!OwnerOverlapToAllInterfaceBuilt)
        buildOwnerOverlapToAllInterface ();
      BC communicator;
      communicator.template build<T>(OwnerOverlapToAllInterface);
      communicator.template forward<AddGatherScatter<T> >(source,dest);
      communicator.free();
    }

    template<class T1, class T2>
    void dot (const T1& x, const T1& y, T2& result) const
    {
      // set up mask vector
      if (mask.size()!=x.size())
      {
        mask.resize(x.size());
        for (int i=0; i<mask.size(); i++) mask[i] = 1;
        for (typename PIS::const_iterator i=pis.begin(); i!=pis.end(); ++i)
          if (i->local().attribute()!=owner)
            mask[i->local().local()] = 0;
      }
      result = 0;
      for (int i=0; i<x.size(); i++)
        result += x[i].operator*(y[i])*mask[i];
      result = cc.sum(result);
      return;
    }

    template<class T1>
    double norm (const T1& x) const
    {
      // set up mask vector
      if (mask.size()!=x.size())
      {
        mask.resize(x.size());
        for (int i=0; i<mask.size(); i++) mask[i] = 1;
        for (typename PIS::const_iterator i=pis.begin(); i!=pis.end(); ++i)
          if (i->local().attribute()!=owner)
            mask[i->local().local()] = 0;
      }
      double result = 0;
      for (int i=0; i<x.size(); i++)
        result += x[i].two_norm2()*mask[i];
      return sqrt(cc.sum(result));
    }

    template<class T1>
    void project (T1& x) const
    {
      for (typename PIS::const_iterator i=pis.begin(); i!=pis.end(); ++i)
        if (i->local().attribute()==copy)
          x[i->local().local()] = 0;
    }


    // Constructor
    // containers of IndexTripel and RemoteIndexTripel sorted appropriately
    // size is the size
    OwnerOverlapCopyCommunication (const IndexInfoFromGrid<GlobalIdType,LocalIdType>& indexinfo, MPI_Comm comm_)
      : cc(comm_),OwnerToAllInterfaceBuilt(false),OwnerOverlapToAllInterfaceBuilt(false)
    {
      // set up an ISTL index set
      pis.beginResize();
      for (localindex_iterator i=indexinfo.localIndices().begin(); i!=indexinfo.localIndices().end(); ++i)
      {
        if (i->third==owner)
          pis.add(i->first,LI(i->second,owner,true));
        if (i->third==overlap)
          pis.add(i->first,LI(i->second,overlap,true));
        if (i->third==copy)
          pis.add(i->first,LI(i->second,copy,true));
        //                std::cout << cc.rank() << ": adding index " << i->first << " " << i->second << " " << i->third << std::endl;
      }
      pis.endResize();

      // build remote indices WITHOUT communication
      //          std::cout << cc.rank() << ": build remote indices" << std::endl;
      ri.setIndexSets(pis,pis,cc);
      if (indexinfo.remoteIndices().size()>0)
      {
        remoteindex_iterator i=indexinfo.remoteIndices().begin();
        int p = i->first;
        RILM modifier = ri.template getModifier<false,true>(p);
        typename PIS::const_iterator pi=pis.begin();
        for ( ; i!=indexinfo.remoteIndices().end(); ++i)
        {
          // handle processor change
          if (p!=i->first)
          {
            p = i->first;
            modifier = ri.template getModifier<false,true>(p);
            pi=pis.begin();
          }

          // position to correct entry in parallel index set
          while (pi->global()!=i->second && pi!=pis.end())
            ++pi;
          if (pi==pis.end())
            DUNE_THROW(ISTLError,"OwnerOverlapCopyCommunication: global index not in index set");

          // insert entry
          //                      std::cout << cc.rank() << ": adding remote index " << i->first << " " << i->second << " " << i->third << std::endl;
          if (i->third==owner)
            modifier.insert(RX(owner,&(*pi)));
          if (i->third==overlap)
            modifier.insert(RX(overlap,&(*pi)));
          if (i->third==copy)
            modifier.insert(RX(copy,&(*pi)));
        }
      }
    }

    // destructor: free memory in some objects
    ~OwnerOverlapCopyCommunication ()
    {
      ri.free();
      if (OwnerToAllInterfaceBuilt) OwnerToAllInterface.free();
      if (OwnerOverlapToAllInterfaceBuilt) OwnerOverlapToAllInterface.free();
    }

  private:
    CollectiveCommunication<MPI_Comm> cc;
    PIS pis;
    RI ri;
    mutable IF OwnerToAllInterface;
    mutable bool OwnerToAllInterfaceBuilt;
    mutable IF OwnerOverlapToAllInterface;
    mutable bool OwnerOverlapToAllInterfaceBuilt;
    mutable std::vector<double> mask;
  };

#endif


  /** @} end documentation */

} // end namespace

#endif
