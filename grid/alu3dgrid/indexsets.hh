// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDINDEXSETS_HH
#define DUNE_ALU3DGRIDINDEXSETS_HH

//- System includes

//- Dune includes

//- Local includes
#include "alu3dinclude.hh"

namespace Dune {

  // Forward declarations
  template <int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGrid;

  //! hierarchic index set of ALU3dGrid
  template <int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGridHierarchicIndexSet
  {
    typedef ALU3dGrid<dim,dimworld,elType> GridType;
    enum { numCodim = 4 };

  public:
    typedef typename GridType::Traits::template codim<0>::Entity EntityCodim0Type;

    ALU3dGridHierarchicIndexSet(const GridType & grid) : grid_(grid)
    {}

    //! return hierarchic index of given entity
    template <class EntityType>
    int index (const EntityType & ep) const
    {
      enum { cd = EntityType :: codimension };
      return (grid_.template getRealEntity<cd>(ep)).getIndex();
    }

    //! return subIndex of given entity
    template <int cd>
    int subIndex (const EntityCodim0Type & ep, int i) const
    {
      assert(cd == dim);
      const ALU3dGridEntity<0,dim,const GridType> & en = (grid_.template getRealEntity<0>(ep));
      return en.template getSubIndex<cd>(i);
    }

    //! return size of indexset, i.e. maxindex+1
    int size ( int codim ) const
    {
      // return maxIndex of hierarchic index set
      return grid_.global_size(codim);
    }

  private:
    // our Grid
    const GridType & grid_;
  };

} // end namespace Dune

#include "indexsets_imp.cc"

#endif
