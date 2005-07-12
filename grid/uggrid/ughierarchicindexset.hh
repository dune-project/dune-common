// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UG_HIERARCHIC_INDEX_SET_HH
#define DUNE_UG_HIERARCHIC_INDEX_SET_HH

namespace Dune {

  //! Hierarchic index set of UGGrid
  template <class GridType>
  class UGGridHierarchicIndexSet
  {
    enum { dim      = GridType::dimension};
    enum { numCodim = GridType::dimension+1 };

  public:
    typedef typename GridType::Traits::template Codim<0>::Entity EntityCodim0Type;

    UGGridHierarchicIndexSet(const GridType & grid) : grid_(grid) {}

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
      return grid_.template getRealEntity<0>(ep).template subIndex<cd>(i);
    }

    /** \brief Return the size of the hierarchic index set, i.e. the largest
        occurring index plus one
        \todo Precompute this value
     */
    int size ( int level, int codim ) const {
      int maxIdx = -1;

      switch (codim) {
      case 0 : {
        for (int i=0; i<=grid_.maxlevel(); i++) {

          typename GridType::Traits::template Codim<0>::LevelIterator eIt    = grid_.template lbegin<0>(i);
          typename GridType::Traits::template Codim<0>::LevelIterator eEndIt = grid_.template lend<0>(i);

          for (; eIt!=eEndIt; ++eIt)
            maxIdx = std::max(maxIdx, index(*eIt));
          //UG_NS<dim>::index(getRealEntity<0>(*eIt).target_) = id++;

        }
        break;
      }

      case dim : {
        for (int i=0; i<=grid_.maxlevel(); i++) {

          typename GridType::Traits::template Codim<dim>::LevelIterator vIt    = grid_.template lbegin<dim>(i);
          typename GridType::Traits::template Codim<dim>::LevelIterator vEndIt = grid_.template lend<dim>(i);

          for (; vIt!=vEndIt; ++vIt)
            maxIdx = std::max(maxIdx, index(*vIt));
          //UG_NS<dim>::index(getRealEntity<dim>(*vIt).target_) = id++;

        }
        break;
      }
      }

      return maxIdx + 1;
    }

  private:
    // our Grid
    const GridType & grid_;
    // size of indexset, managed by grid
    //const int (& size_)[numCodim];
  };

}  // end namespace Dune

#endif
