// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_AMGTRANSFER_HH
#define DUNE_AMGTRANSFER_HH

#include <dune/istl/paamg/aggregates.hh>
#include <dune/common/exceptions.hh>
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
    template<class V1, class V2>
    class Transfer
    {

    public:
      typedef V1 Vertex;
      typedef V2 Vector;

      static void prolongate(const AggregatesMap<Vertex>& aggregates, Vector& coarse, Vector& fine,
                             typename Vector::field_type damp);
      static void restrict (const AggregatesMap<Vertex>& aggregates, Vector& coarse, const Vector & fine);
    };

    template<class V1, class V2>
    void Transfer<V1,V2>::prolongate(const AggregatesMap<Vertex>& aggregates, Vector& coarse,
                                     Vector& fine, typename Vector::field_type damp)
    {
      DUNE_THROW(NotImplemented, "There is no secialization available for this type of vector!");
    }
    template<class V1, class V2>
    void Transfer<V1,V2>::restrict (const AggregatesMap<Vertex>& aggregates, Vector& coarse,
                                    const Vector & fine)
    {
      DUNE_THROW(NotImplemented, "There is no secialization available for this type of vector!");
    }

    template<class V,class B>
    class Transfer<V,BlockVector<B> >
    {
    public:
      typedef V Vertex;
      typedef BlockVector<B> Vector;
      static void prolongate(const AggregatesMap<Vertex>& aggregates, Vector& coarse, Vector& fine,
                             typename Vector::field_type damp);

      static void restrict (const AggregatesMap<Vertex>& aggregates, Vector& coarse, const Vector & fine);
    };

    template<class V, class B>
    inline void Transfer<V,BlockVector<B> >::prolongate(const AggregatesMap<Vertex>& aggregates,
                                                        Vector& coarse, Vector& fine,
                                                        typename Vector::field_type damp)
    {
      typedef typename Vector::iterator Iterator;

      Iterator end = fine.end();

      coarse *= damp;

      for(Iterator block=fine.begin(); block != end; ++block) {
        *block += coarse[aggregates[block.index()]];
      }
    }

    template<class V, class B>
    inline void Transfer<V,BlockVector<B> >::restrict (const AggregatesMap<Vertex>& aggregates,
                                                       Vector& coarse,
                                                       const Vector & fine)
    {
      // Set coarse vector to zero
      coarse=0;

      typedef typename Vector::const_iterator Iterator;
      Iterator end = fine.end();
      for(Iterator block=fine.begin(); block != end; ++block)
        coarse[aggregates[block.index()]] += *block;
    }
    /** @} */
  } // namspace Amg
} // namspace Dune
#endif
