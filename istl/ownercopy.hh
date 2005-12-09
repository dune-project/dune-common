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
     attribute set with owner/copy semantics.
   */

  // set up communication from known distribution with owner/copy semantics
  template <class GlobalIdType, class LocalIdType, int ownerattribute, int copyattribute>
  class OwnerCopyCommunication
  {
    // used types
    typedef tripel<GlobalIdType,LocalIdType,int> IndexTripel;
    typedef tripel<int,GlobalIdType,int> RemoteIndexTripel;
    enum AttributeSet { owner=ownerattribute, copy=copyattribute };
    typedef ParallelLocalIndex<AttributeSet> LI;
    typedef ParallelIndexSet<GlobalIdType,LI,512> PIS;
    typedef RemoteIndices<PIS> RI;
    typedef RemoteIndexListModifier<PIS,false> RILM;
    typedef typename RI::RemoteIndex RX;
    typedef BufferedCommunicator<PIS> BC;
    typedef Interface<PIS> IF;

    // gather/scatter callback for communcation
    template<typename T>
    struct ForwardGatherScatter
    {
      typedef typename T::value_type V;

      static V gather(const T& a, int i)
      {
        return a[i];
      }

      static void scatter(T& a, V v, int i)
      {
        a[i] = v;
      }
    };

  public:

    // send owner value to
    template<class T>
    void ownerToCopy (T& source, T& dest)
    {
      BC o2c;
      o2c.template build<T>(cif);
      o2c.template forward<ForwardGatherScatter<T> >(source,dest);
      o2c.free();
    }

    // Constructor
    // containers of IndexTripel and RemoteIndexTripel sorted appropriately
    template<class C1, class C2>
    OwnerCopyCommunication (C1& ownindices, C2& othersindices, MPI_Comm comm)
    {
      // Process configuration
      int procs, rank;
      MPI_Comm_size(comm, &procs);
      MPI_Comm_rank(comm, &rank);

      // set up an ISTL index set
      pis.beginResize();
      for (typename C1::iterator i=ownindices.begin(); i!=ownindices.end(); ++i)
      {
        if (i->third==owner)
          pis.add(i->first,LI(i->second,owner,true));
        if (i->third==copy)
          pis.add(i->first,LI(i->second,copy,true));
        std::cout << rank << ": adding index " << i->first << " " << i->second << " " << i->third << std::endl;
      }
      pis.endResize();

      // build remote indices WITHOUT communication
      std::cout << rank << ": build remote indices" << std::endl;
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
            DUNE_THROW(ISTLError,"OwnerOverlapCommunication: global index not in index set");

          // insert entry
          std::cout << rank << ": adding remote index " << i->first << " " << i->second << " " << i->third << std::endl;
          if (i->third==owner)
            modifier.insert(RX(owner,&(*pi)));
          if (i->third==copy)
            modifier.insert(RX(copy,&(*pi)));
        }
      }

      // now set up a communication interface
      EnumItem<AttributeSet,owner> sourceFlags;
      EnumItem<AttributeSet,copy> destFlags;
      cif.build(ri,sourceFlags,destFlags);
    }

    // destructor: free memory in some objects
    ~OwnerCopyCommunication ()
    {
      ri.free();
      cif.free();
    }

  private:
    PIS pis;      // parallel index set
    RI ri;        // remote indices
    IF cif;       // interface
  };




  /** @} end documentation */

} // end namespace

#endif
