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

#include "mpi.h"

#include "dune/common/tripel.hh"
#include <dune/common/enumset.hh>

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


  // set up communication from known distribution with owner/overlap/copy semantics
  template <class GlobalIdType, class LocalIdType, int ownerattribute, int overlapattribute, int copyattribute>
  class OwnerOverlapCopyCommunication
  {
    // used types
    typedef tripel<GlobalIdType,LocalIdType,int> IndexTripel;
    typedef tripel<int,GlobalIdType,int> RemoteIndexTripel;
    enum AttributeSet { owner=ownerattribute, overlap=overlapattribute, copy=copyattribute };
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
      int procs;
      MPI_Comm_size(comm,&procs);
      if (procs==1) return;
      double res;     // assumes that result is double \todo: template magick to treat complex<...>
      MPI_Allreduce(&result,&res,1,MPI_DOUBLE,MPI_SUM,comm);
      result = res;
      return;
    }

    template<class T1>
    double norm (const T1& x) const
    {
      int rank;
      MPI_Comm_rank(comm,&rank);

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
      {
        result += x[i].two_norm2()*mask[i];
        //                if (mask[i]==1)
        //                      std::cout << rank << ": " << "index=" << i << " value=" << x[i].two_norm2() << std::endl;
      }
      int procs;
      MPI_Comm_size(comm,&procs);
      if (procs==1) return sqrt(result);
      double res;
      MPI_Allreduce(&result,&res,1,MPI_DOUBLE,MPI_SUM,comm);
      return sqrt(res);
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
    template<class C1, class C2>
    OwnerOverlapCopyCommunication (C1& ownindices, C2& othersindices, MPI_Comm comm_)
      : OwnerToAllInterfaceBuilt(false),OwnerOverlapToAllInterfaceBuilt(false)
    {
      // Process configuration
      int procs, rank;
      comm = comm_;
      MPI_Comm_size(comm, &procs);
      MPI_Comm_rank(comm, &rank);

      // set up an ISTL index set
      pis.beginResize();
      for (typename C1::iterator i=ownindices.begin(); i!=ownindices.end(); ++i)
      {
        if (i->third==owner)
          pis.add(i->first,LI(i->second,owner,true));
        if (i->third==overlap)
          pis.add(i->first,LI(i->second,overlap,true));
        if (i->third==copy)
          pis.add(i->first,LI(i->second,copy,true));
        //                std::cout << rank << ": adding index " << i->first << " " << i->second << " " << i->third << std::endl;
      }
      pis.endResize();

      // build remote indices WITHOUT communication
      //          std::cout << rank << ": build remote indices" << std::endl;
      ri.setIndexSets(pis,pis,comm);
      if (othersindices.size()>0)
      {
        typename C2::iterator i=othersindices.begin();
        int p = i->first;
        RILM modifier = ri.template getModifier<false,true>(p);
        typename PIS::const_iterator pi=pis.begin();
        for ( ; i!=othersindices.end(); ++i)
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
          //                      std::cout << rank << ": adding remote index " << i->first << " " << i->second << " " << i->third << std::endl;
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
    MPI_Comm comm;
    PIS pis;
    RI ri;
    mutable IF OwnerToAllInterface;
    mutable bool OwnerToAllInterfaceBuilt;
    mutable IF OwnerOverlapToAllInterface;
    mutable bool OwnerOverlapToAllInterfaceBuilt;
    mutable std::vector<double> mask;
  };




  /** @} end documentation */

} // end namespace

#endif
