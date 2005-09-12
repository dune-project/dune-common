// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTAGRIDINDEXSETS_HH
#define DUNE_ALBERTAGRIDINDEXSETS_HH

//- System includes

//- Dune includes
#include <dune/common/stdstreams.hh>
#include <dune/grid/common/grid.hh>

//- Local includes

namespace Dune {

  // Forward declarations
  template <int dim, int dimworld> class AlbertaGrid;
  template<int cd, int dim, class GridImp> class AlbertaGridEntity;
  template <class GridType, int dim> struct MarkEdges;

  template <int dim, int dimworld>
  class AlbertaGridHierarchicIndexSet
  {
    typedef AlbertaGrid<dim,dimworld> GridType;
    typedef typename GridType :: Traits :: template Codim<0>::Entity EntityCodim0Type;
    enum { numVecs  = AlbertHelp::numOfElNumVec };
    enum { numCodim = dim + 1 };

    template <int cd>
    struct Codim
    {
      typedef AlbertaGridEntity<cd,dim,const GridType> RealEntityType;
      typedef typename Dune::Entity<cd,dim,const GridType,AlbertaGridEntity> EntityType;
    };

    // all classes that are allowed to call private functions
    friend class AlbertaGrid<dim,dimworld>;
    friend class MarkEdges<GridType,3>;
    friend class MarkEdges<const GridType,3>;

    // only  AlbertaGrid is allowed to create this class
    AlbertaGridHierarchicIndexSet(const GridType & grid) : grid_( grid ) {}
  public:

    //! return index of entity
    template <class EntityType>
    int index (const EntityType & ep) const
    {
      enum { cd = EntityType :: codimension };
      //std::cout << "codim = "<< cd << "\n";
      const AlbertaGridEntity<cd,dim,const GridType> & en = (grid_.template getRealEntity<cd>(ep));
      return getIndex(en.getElInfo()->el, en.getFEVnum(),Int2Type<dim-cd>());
    }

    //! return subIndex of given enitiy's sub entity with codim=cd and number i
    template <int cd>
    int subIndex (const EntityCodim0Type & en, int i) const
    {
      //std::cout << "get Subindex for cd = " << cd << "\n";
      // doesn't work otherwise, but stalls gridcheck
      // assert(cd == dim);
      return getIndex((grid_.template getRealEntity<0>(en)).getElInfo()->el
                      ,i,Int2Type<dim-cd>());
    }

    //! return size of set
    int size (int codim) const
    {
      return grid_.global_size(codim);
    }

    //! return geometry types this set has indices for
    const std::vector< GeometryType > & geomTypes() const
    {
      return grid_.geomTypes();
    }

  private:
    const GridType & grid_;
    const int * elNumVec_[numVecs];

    int nv_[numVecs];
    int dof_[numVecs];

    // update vec pointer of the DOF_INT_VECs, which can change during resize
    void updatePointers(ALBERTA AlbertHelp::DOFVEC_STACK & dofvecs)
    {
      for(int i=0; i<numVecs; i++)
      {
        elNumVec_[i] = (dofvecs.elNumbers[i])->vec;
        assert(elNumVec_[i]);
      }

      setDofIdentifier<0> (dofvecs);
      if(numVecs > 1) setDofIdentifier<1> (dofvecs);
      if(numVecs > 2) setDofIdentifier<2> (dofvecs);
      if(numVecs > 3) setDofIdentifier<3> (dofvecs);

      /*
         for(int m=0; m<numVecs; m++)
         {
         std::cout << "Size of cd["<<m<<"] = " << size(m) << "\n";
         for(int i=0; i<size(m); i++)
          std::cout << elNumVec_[ m ] [i] << " number \n";
         }
         std::cout << "Done setting Pointers of vecs\n";
       */
    }

    template <int cd>
    void setDofIdentifier (ALBERTA AlbertHelp::DOFVEC_STACK & dofvecs)
    {
      const ALBERTA DOF_ADMIN * elAdmin_ = dofvecs.elNumbers[cd]->fe_space->admin;
      // see Albert Doc. , should stay the same

      nv_ [cd] = elAdmin_->n0_dof    [ALBERTA AlbertHelp::AlbertaDofType<cd>::type];
      assert( nv_ [cd] == 0);
      dof_[cd] = elAdmin_->mesh->node[ALBERTA AlbertHelp::AlbertaDofType<cd>::type];
    }

    // codim = 0 means we get from dim-cd = dim
    // this is the method for the element numbers
    int getIndex ( const ALBERTA EL * el, int i , Int2Type<dim> fake ) const
    {
      enum { cd = 0 };
      assert(el);
      return elNumVec_[cd][ el->dof[ dof_[cd] ][nv_[cd]] ];
    }

    enum { cd1 = (dim == 2) ? 1 : 2 };
    // method for face numbers
    // codim = 0 means we get from dim-cd = dim
    int getIndex ( const ALBERTA EL * el, int i , Int2Type<cd1> fake ) const
    {
      enum { cd = 1 };
      assert(el);
      assert( (dim == 2) || (dim == 3));
      // dof_[cd] marks the insertion point form which this dofs start
      // then i is the i-th dof
      return elNumVec_[cd][ el->dof[ dof_[cd] + i ][ nv_[cd] ] ];
    }

    enum { cd2 = (dim > 2) ? 1 : 6 };
    // codim = 0 means we get from dim-cd = dim
    // this method we have only in 3d, for edges
    int getIndex ( const ALBERTA EL * el, int i , Int2Type<cd2> fake ) const
    {
      enum { cd = 2 };
      assert(el);
      // dof_[cd] marks the insertion point form which this dofs start
      // then i is the i-th dof
      return elNumVec_[cd][ el->dof[ dof_[cd] + i ][ nv_[cd] ] ];
    }

    // codim = dim  means we get from dim-cd = 0
    int getIndex ( const ALBERTA EL * el, int i , Int2Type<0> fake ) const
    {
      assert(el);
      return (el->dof[i][0]);
    }

    int getIndex ( const ALBERTA EL * el, int i , Int2Type<-1> fake ) const
    {
      assert(false);
      DUNE_THROW(AlbertaError,"Error, wrong codimension!\n");
      return -1;
    }
  }; // end class AlbertaGridHierarchicIndexSet


  //! hierarchic index set of AlbertaGrid
  template <int dim, int dimworld>
  class AlbertaGridIdSet :
    public IdSet < AlbertaGrid<dim,dimworld> ,
        AlbertaGridIdSet<dim,dimworld> , int >
  {
    typedef const AlbertaGrid<dim,dimworld> GridType;
    typedef typename GridType :: HierarchicIndexSet HierarchicIndexSetType;

    // this means that only up to 300000000 entities are allowed
    enum { codimMultiplier = 300000000 };
    typedef typename GridType::Traits::template Codim<0>::Entity EntityCodim0Type;

    //! create id set, only allowed for AlbertaGrid
    AlbertaGridIdSet(const GridType & grid) : hset_(grid.hierarchicIndexSet())
    {
      derr << "WARNING: AlbertaGridGlobalIdSet not ready for parallel compuations right now! \n";
      for(int i=0; i<dim+1; i++)
        codimStart_[i] = i*codimMultiplier;
    }

    friend class AlbertaGrid<dim,dimworld>;
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
    int id (const typename GridType::template Codim<codim>::Entity& ep) const
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
} // namespace Dune

#endif
