// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDINDEXSETS_HH
#define DUNE_ALU3DGRIDINDEXSETS_HH

//- System includes

//- Dune includes
#include <dune/common/stdstreams.hh>
#include <dune/grid/common/grid.hh>
#include <dune/grid/common/indexidset.hh>

//- Local includes
#include "alu3dinclude.hh"

namespace Dune {

  //! HierarchicIndexSet uses LeafIterator tpyes for all codims and partition types
  template <class GridImp>
  struct ALU3dGridHierarchicIteratorTypes
  {
    //! The types of the iterator
    template<int cd>
    struct Codim
    {
      template<PartitionIteratorType pitype>
      struct Partition
      {
        typedef typename GridImp::Traits::template Codim<cd>::template Partition<pitype>::LeafIterator Iterator;
      };
    };
  };

  // Forward declarations
  template <int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGrid;

  template<int cd, int dim, class GridImp>
  class ALU3dGridEntity;

  //! hierarchic index set of ALU3dGrid
  template <int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGridHierarchicIndexSet :
    public IndexSet<ALU3dGrid<dim,dimworld,elType>,
        ALU3dGridHierarchicIndexSet<dim,dimworld,elType>,
        ALU3dGridHierarchicIteratorTypes<ALU3dGrid<dim,dimworld,elType> > >
  {
    typedef ALU3dGrid<dim,dimworld,elType> GridType;
    enum { numCodim = dim+1 }; // i.e. 4

    ALU3dGridHierarchicIndexSet(const GridType & grid) : grid_(grid)
    {}
    friend class ALU3dGrid<dim,dimworld,elType>;

  public:
    typedef typename GridType::Traits::template Codim<0>::Entity EntityCodim0Type;

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
      const ALU3dGridEntity<0,dim,const GridType> & en = (grid_.template getRealEntity<0>(ep));
      return en.template getSubIndex<cd>(i);
    }

    //! return size of indexset, i.e. maxindex+1
    //! for given type, if type is not exisiting within grid 0 is returned
    int size ( int codim , GeometryType type ) const
    {
      assert( grid_.geomTypes(codim).size() == 1 );
      if( type != grid_.geomTypes(codim)[0] ) return 0;
      // return size of hierarchic index set
      return grid_.hierSetSize(codim);
    }

    //! return size of indexset, i.e. maxindex+1
    int size ( int codim ) const
    {
      // return size of hierarchic index set
      return grid_.hierSetSize(codim);
    }

    //! deliver all geometry types used in this grid
    const std::vector<GeometryType>& geomTypes (int codim) const
    {
      return grid_.geomTypes(codim);
    }

    /** @brief Iterator to one past the last entity of given codim for partition type
     */
    template<int cd, PartitionIteratorType pitype>
    typename ALU3dGridHierarchicIteratorTypes<GridType>::template Codim<cd>::
    template Partition<pitype>::Iterator end () const
    {
      return grid_.template leafend<cd,pitype> ();
    }

    /** @brief Iterator to first entity of given codimension and partition type.
     */
    template<int cd, PartitionIteratorType pitype>
    typename ALU3dGridHierarchicIteratorTypes<GridType>::template Codim<cd>::
    template Partition<pitype>::Iterator begin () const
    {
      return grid_.template leafbegin<cd,pitype> ();
    }

  private:
    // our Grid
    const GridType & grid_;
  };

  //! hierarchic index set of ALU3dGrid
  template <int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGridGlobalIdSet :
    public IdSet < ALU3dGrid<dim,dimworld,elType> ,
        ALU3dGridGlobalIdSet<dim,dimworld,elType> ,
        int >
  {
    typedef ALU3dGrid<dim,dimworld,elType> GridType;
    typedef typename GridType :: HierarchicIndexSet HierarchicIndexSetType;

    // this means that only up to 300000000 entities are allowed
    enum { codimMultiplier = 300000000 };
    typedef typename GridType::Traits::template Codim<0>::Entity EntityCodim0Type;

    //! create id set, only allowed for ALU3dGrid
    ALU3dGridGlobalIdSet(const GridType & grid) : hset_(grid.hierarchicIndexSet())
    {
      derr << "WARNING: ALU3dGridGlobalIdSet not ready for parallel computations right now! \n";
      for(int i=0; i<dim+1; i++)
        codimStart_[i] = i*codimMultiplier;
    }

    friend class ALU3dGrid<dim,dimworld,elType>;
  public:
    //! export type of id
    typedef int IdType;

    //! return global id of given entity
    template <class EntityType>
    int id (const EntityType & ep) const
    {
      enum { cd = EntityType :: codimension };
      assert( hset_.size(cd) < codimMultiplier );
      return codimStart_[cd] + hset_.index(ep);
    }

    //! return global id of given entity
    template <int codim>
    int id (const typename GridType:: template Codim<codim> :: Entity & ep) const
    {
      //enum { cd = EntityType :: codimension };
      assert( hset_.size(codim) < codimMultiplier );
      return codimStart_[codim] + hset_.index(ep);
    }

    //! return subId of given entity
    template <int cd>
    int subId (const EntityCodim0Type & ep, int i) const
    {
      assert( cd == dim );
      assert( hset_.size(cd) < codimMultiplier );
      return codimStart_[cd] + hset_.template subIndex<cd>(ep,i);
    }

  private:
    // our Grid
    const HierarchicIndexSetType & hset_;

    // store start of each codim numbers
    int codimStart_[dim+1];
  };

  //! hierarchic index set of ALU3dGrid
  template <int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGridLocalIdSet :
    public IdSet < ALU3dGrid<dim,dimworld,elType> ,
        ALU3dGridLocalIdSet<dim,dimworld,elType> ,
        int >
  {
    typedef ALU3dGrid<dim,dimworld,elType> GridType;
    typedef typename GridType :: HierarchicIndexSet HierarchicIndexSetType;

    // this means that only up to 300000000 entities are allowed
    enum { codimMultiplier = 300000000 };
    typedef typename GridType::Traits::template Codim<0>::Entity EntityCodim0Type;

    // create local id set , only for the grid allowed
    ALU3dGridLocalIdSet(const GridType & grid) : hset_(grid.hierarchicIndexSet())
    {
      for(int i=0; i<dim+1; i++)
        codimStart_[i] = i*codimMultiplier;
    }
    friend class ALU3dGrid<dim,dimworld,elType>;

  public:
    //! export type of id
    typedef int IdType;

    //! return global id of given entity
    template <class EntityType>
    int id (const EntityType & ep) const
    {
      enum { cd = EntityType :: codimension };
      assert( hset_.size(cd) < codimMultiplier );
      return codimStart_[cd] + hset_.index(ep);
    }

    //! return global id of given entity
    template <int codim>
    int id (const typename GridType:: template Codim<codim> :: Entity & ep) const
    {
      //enum { cd = EntityType :: codimension };
      assert( hset_.size(codim) < codimMultiplier );
      return codimStart_[codim] + hset_.index(ep);
    }

    //! return subId of given entity
    template <int cd>
    int subId (const EntityCodim0Type & ep, int i) const
    {
      assert( cd == dim );
      assert( hset_.size(cd) < codimMultiplier );
      return codimStart_[cd] + hset_.template subIndex<cd>(ep,i);
    }

  private:
    // our Grid
    const HierarchicIndexSetType & hset_;

    // store start of each codim numbers
    int codimStart_[dim+1];
  };

} // end namespace Dune

#include "indexsets_imp.cc"

#endif
