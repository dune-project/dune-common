// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDINDEXSETS_HH
#define DUNE_ALU3DGRIDINDEXSETS_HH

//- System includes

//- Dune includes
#include <dune/common/stdstreams.hh>
#include <dune/grid/common/grid.hh>

//- Local includes
#include "alu3dinclude.hh"

namespace Dune {

  // Forward declarations
  template <int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGrid;

  template<int cd, int dim, class GridImp>
  class ALU3dGridEntity;

  //! hierarchic index set of ALU3dGrid
  template <int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGridHierarchicIndexSet
  {
    typedef ALU3dGrid<dim,dimworld,elType> GridType;
    enum { numCodim = 4 };

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
      assert(cd == dim);
      const ALU3dGridEntity<0,dim,const GridType> & en = (grid_.template getRealEntity<0>(ep));
      return en.template subIndex<cd>(i);
    }

    //! return size of indexset, i.e. maxindex+1
    int size ( int codim ) const
    {
      // return maxIndex of hierarchic index set
      return grid_.global_size(codim);
    }

    //! deliver all geometry types used in this grid
    const std::vector<GeometryType>& geomTypes () const
    {
      return grid_.geomTypes();
    }

  private:
    // our Grid
    const GridType & grid_;
  };

  //! hierarchic index set of ALU3dGrid
  template <int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGridGlobalIdSet
  {
    typedef ALU3dGrid<dim,dimworld,elType> GridType;
    typedef typename GridType :: HierarchicIndexSetType HierarchicIndexSetType;

    // this means that only up to 300000000 entities are allowed
    enum { codimMultiplier = 300000000 };
    typedef typename GridType::Traits::template Codim<0>::Entity EntityCodim0Type;

    //! create id set, only allowed for ALU3dGrid
    ALU3dGridGlobalIdSet(const GridType & grid) : hset_(grid.hierarchicIndexSet())
    {
      derr << "WARNING: ALU3dGridGlobalIdSet not ready for parallel compuations right now! \n";
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
  class ALU3dGridLocalIdSet
  {
    typedef ALU3dGrid<dim,dimworld,elType> GridType;
    typedef typename GridType :: HierarchicIndexSetType HierarchicIndexSetType;

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
