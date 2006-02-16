// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_PANTYPE_INDEXSET_HH
#define DUNE_PANTYPE_INDEXSET_HH

/** \file
    \brief Implements an index set that is consecutive across all GeometryTypes
 */
#include <dune/grid/common/indexidset.hh>
#include <dune/common/geometrytype.hh>
#include <map>

namespace Dune {

  /** \brief Types for the PanTypeIndexSet */
  template <class HostIndexSet>
  struct PanTypeIndexSetTypes
  {
    //! The types
    template<int cd>
    struct Codim
    {
      /** \brief Contains the iterators for traversing the different PartitionTypes of this index set */
      template<PartitionIteratorType pitype>
      struct Partition
      {
        /** \brief Iterator needed to traverse the entities for this index set */
        typedef typename HostIndexSet::template Codim<cd>::template Partition<pitype>::Iterator Iterator;
      };
    };
  };

  /** \brief An index set that is consecutive across different GeometryTypes
   */
  template<class GridImp, class IndexSetType>
  class PanTypeIndexSet : public IndexSet<GridImp,PanTypeIndexSet<GridImp,IndexSetType>,PanTypeIndexSetTypes<IndexSetType> >
  {
    enum {dim = GridImp::dimension};
    typedef IndexSet<GridImp,PanTypeIndexSet<GridImp,IndexSetType>,PanTypeIndexSetTypes<IndexSetType> > Base;

  public:

    /** \brief Constructor with a given grid and index set */
    PanTypeIndexSet(const GridImp& grid, const IndexSetType& indexSet)
      : grid_(&grid), indexSet_(&indexSet)
    {
      update();
    }

    //! get index of an entity
    template<int cd>
    int index (const typename GridImp::Traits::template Codim<cd>::Entity& e) const
    {
      return indexSet_->index(e) + offsets_[cd].find(e.geometry().type())->second;
    }

    //! get index of subEntity of a codim 0 entity
    template<int cd>
    int subIndex (const typename GridImp::Traits::template Codim<0>::Entity& e, int i) const
    {
      //return grid_->getRealImplementation(e).template subIndex<cc>(i);
      const Dune::ReferenceElement<double,dim>& refElement
        = Dune::ReferenceElements<double, dim>::general(e.geometry().type());

      return indexSet_->template subIndex<cd>(e,i)
             + offsets_[cd].find(refElement.type(i,cd))->second;
    }


    //! get number of entities of given codim, type and on this level
    int size (int codim) const {
      return indexSet_->size(codim);
    }

    //! get number of entities of given codim, type and on this level
    int size (int codim, GeometryType type) const
    {
      return indexSet_->size(codim, type);
    }

    /** \brief Deliver all geometry types used in this grid */
    const std::vector<GeometryType>& geomTypes (int codim) const
    {
      return indexSet_->geomTypes(codim);
    }

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Base::template Codim<cd>::template Partition<pitype>::Iterator begin () const
    {
      return indexSet_->template begin<cd,pitype>();
    }

    //! Iterator to one past the last entity of given codim on level for partition type
    template<int cd, PartitionIteratorType pitype>
    typename Base::template Codim<cd>::template Partition<pitype>::Iterator end () const
    {
      return indexSet_->template end<cd,pitype>();
    }

    /** \brief Update the index set

       You need to call this after the host index set has changed.
     */
    void update() {

      for (int i=0; i<=dim; i++) {

        offsets_[i].clear();

        const std::vector<Dune::GeometryType>& geomTypes = indexSet_->geomTypes(i);

        if (geomTypes.size()==0)
          return;

        unsigned int offset = 0;
        for (unsigned int j=0; j<geomTypes.size(); j++) {
          offsets_[i].insert(std::pair<Dune::GeometryType,unsigned int>(geomTypes[j], offset));
          offset += indexSet_->size(i, geomTypes[j]);
        }

      }

    }

  private:
    const GridImp* grid_;
    const IndexSetType* indexSet_;


    std::map<Dune::GeometryType,unsigned int> offsets_[dim+1];

  };


}

#endif
