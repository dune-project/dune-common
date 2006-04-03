// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_AMGTRANSFER_HH
#define DUNE_AMGTRANSFER_HH

#include <dune/istl/paamg/pinfo.hh>
#include <dune/istl/owneroverlapcopy.hh>
#include <dune/istl/paamg/aggregates.hh>
#include <dune/common/exceptions.hh>
#include <dune/istl/communicator.hh>
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
     * @brief Prolongation and restriction for amg.
     */
    template<class V1, class V2, class T>
    class Transfer
    {

    public:
      typedef V1 Vertex;
      typedef V2 Vector;

      static void prolongate(const AggregatesMap<Vertex>& aggregates, Vector& coarse, Vector& fine,
                             typename Vector::field_type damp);

      static void restrict (const AggregatesMap<Vertex>& aggregates, Vector& coarse, const Vector & fine,
                            T& comm);
    };

    template<class V,class B>
    class Transfer<V,BlockVector<B>, SequentialInformation>
    {
    public:
      typedef V Vertex;
      typedef BlockVector<B> Vector;
      static void prolongate(const AggregatesMap<Vertex>& aggregates, Vector& coarse, Vector& fine,
                             typename Vector::field_type damp);

      static void restrict (const AggregatesMap<Vertex>& aggregates, Vector& coarse, const Vector & fine,
                            const SequentialInformation & comm);
    };

    template<class V,class B, class T>
    class Transfer<V,BlockVector<B>,ParallelInformation<T> >
    {
    public:
      typedef V Vertex;
      typedef BlockVector<B> Vector;
      static void prolongate(const AggregatesMap<Vertex>& aggregates, Vector& coarse, Vector& fine,
                             typename Vector::field_type damp);

      static void restrict (const AggregatesMap<Vertex>& aggregates, Vector& coarse, const Vector & fine,
                            ParallelInformation<T>& comm);
    };

    template<class V,class B, class T1, class T2>
    class Transfer<V,BlockVector<B>,OwnerOverlapCopyCommunication<T1,T2> >
    {
    public:
      typedef V Vertex;
      typedef BlockVector<B> Vector;
      static void prolongate(const AggregatesMap<Vertex>& aggregates, Vector& coarse, Vector& fine,
                             typename Vector::field_type damp);

      static void restrict (const AggregatesMap<Vertex>& aggregates, Vector& coarse, const Vector & fine,
                            OwnerOverlapCopyCommunication<T1,T2>& comm);
    };

    template<class V, class B>
    inline void Transfer<V,BlockVector<B>,SequentialInformation>::prolongate(const AggregatesMap<Vertex>& aggregates,
                                                                             Vector& coarse, Vector& fine,
                                                                             typename Vector::field_type damp)
    {
      typedef typename Vector::iterator Iterator;

      Iterator end = fine.end();

      coarse *= damp;

      for(Iterator block=fine.begin(); block != end; ++block) {
        const Vertex& vertex = aggregates[block.index()];
        if(vertex != AggregatesMap<Vertex>::ISOLATED)
          *block += coarse[aggregates[block.index()]];
      }
    }

    template<class V, class B>
    inline void Transfer<V,BlockVector<B>,SequentialInformation>::restrict (const AggregatesMap<Vertex>& aggregates,
                                                                            Vector& coarse,
                                                                            const Vector & fine,
                                                                            const SequentialInformation & comm)
    {
      // Set coarse vector to zero
      coarse=0;

      typedef typename Vector::const_iterator Iterator;
      Iterator end = fine.end();
      for(Iterator block=fine.begin(); block != end; ++block) {
        const Vertex& vertex = aggregates[block.index()];
        if(vertex != AggregatesMap<Vertex>::ISOLATED)
          coarse[vertex] += *block;
      }
    }

    template<class V, class B, class T>
    inline void Transfer<V,BlockVector<B>,ParallelInformation<T> >::prolongate(const AggregatesMap<Vertex>& aggregates,
                                                                               Vector& coarse, Vector& fine,
                                                                               typename Vector::field_type damp)
    {
      Transfer<V,BlockVector<B>,SequentialInformation>::prolongate(aggregates, coarse, fine, damp);
    }

    template<class V, class B, class T>
    inline void Transfer<V,BlockVector<B>,ParallelInformation<T> >::restrict (const AggregatesMap<Vertex>& aggregates,
                                                                              Vector& coarse, const Vector & fine,
                                                                              ParallelInformation<T>& comm)
    {
      Transfer<V,BlockVector<B>,SequentialInformation>::restrict (aggregates, coarse, fine, SequentialInformation());
    }

    template<class V, class B, class T1, class T2>
    inline void Transfer<V,BlockVector<B>,OwnerOverlapCopyCommunication<T1,T2> >::prolongate(const AggregatesMap<Vertex>& aggregates,
                                                                                             Vector& coarse, Vector& fine,
                                                                                             typename Vector::field_type damp)
    {
      Transfer<V,BlockVector<B>,SequentialInformation>::prolongate(aggregates, coarse, fine, damp);
    }

    template<class V, class B, class T1, class T2>
    inline void Transfer<V,BlockVector<B>,OwnerOverlapCopyCommunication<T1,T2> >::restrict (const AggregatesMap<Vertex>& aggregates,
                                                                                            Vector& coarse, const Vector & fine,
                                                                                            OwnerOverlapCopyCommunication<T1,T2>& comm)
    {
      Transfer<V,BlockVector<B>,SequentialInformation>::restrict (aggregates, coarse, fine, SequentialInformation());
      //      comm.project(coarse);
    }
    /** @} */
  }    // namspace Amg
}     // namspace Dune
#endif
