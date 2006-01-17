// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "geometry.hh"
#include "grid.hh"
#include <dune/common/exceptions.hh>
#include <dune/grid/common/referenceelements.hh>

namespace Dune {

  // --Entity
  template <int cd, int dim, class GridImp>
  inline ALU3dGridEntity<cd,dim,GridImp> ::
  ALU3dGridEntity(const GridImp  &grid, int level) :
    grid_(grid),
    level_(0),
    gIndex_(-1),
    twist_(0),
    face_(-1),
    item_(0),
    father_(0),
    geo_(),
    builtgeometry_(false),
    localFCoordCalced_ (false)
  {}

  template<int cd, int dim, class GridImp>
  inline void ALU3dGridEntity<cd,dim,GridImp> ::
  reset( int l )
  {
    item_  = 0;
    level_ = l;
    twist_ = 0;
    face_  = -1;
  }

  template<int cd, int dim, class GridImp>
  inline void ALU3dGridEntity<cd,dim,GridImp> ::
  removeElement()
  {
    item_ = 0;
  }

  template<int cd, int dim, class GridImp>
  inline bool ALU3dGridEntity<cd,dim,GridImp> ::
  equals(const ALU3dGridEntity<cd,dim,GridImp> & org) const
  {
    return (item_ == org.item_);
  }

  template<int cd, int dim, class GridImp>
  inline void ALU3dGridEntity<cd,dim,GridImp> ::
  setEntity(const ALU3dGridEntity<cd,dim,GridImp> & org)
  {
    item_   = org.item_;
    gIndex_ = org.gIndex_;
    twist_  = org.twist_;
    level_  = org.level_;
    face_   = org.face_;
    father_ = org.father_;
    builtgeometry_= false;
    localFCoordCalced_ = false;
  }

  template<int cd, int dim, class GridImp>
  inline void ALU3dGridEntity<cd,dim,GridImp> :: setElement(const ElementType & item, int twist , int face )
  {
    item_   = static_cast<const IMPLElementType *> (&item);
    gIndex_ = (*item_).getIndex();
    twist_  = twist;
    level_  = (*item_).level();
    face_   = face;
    builtgeometry_=false;
    localFCoordCalced_ = false;
  }

  template<>
  inline void ALU3dGridEntity<3,3,const ALU3dGrid<3,3,hexa> > ::
  setElement(const ALU3DSPACE HElementType &el, const ALU3DSPACE VertexType &vx)
  {
    item_   = static_cast<const IMPLElementType *> (&vx);
    gIndex_ = (*item_).getIndex();
    level_  = (*item_).level();
    father_ = (&el);
    builtgeometry_=false;
    localFCoordCalced_ = false;
  }

  template<>
  inline void ALU3dGridEntity<3,3,const ALU3dGrid<3,3,tetra> > ::
  setElement(const ALU3DSPACE HElementType &el, const ALU3DSPACE VertexType &vx)
  {
    // * what the heck does this static_cast do!?
    item_   = static_cast<const IMPLElementType *> (&vx);
    gIndex_ = (*item_).getIndex();
    level_  = (*item_).level();
    father_ = (&el);
    builtgeometry_=false;
    localFCoordCalced_ = false;
  }

  template<int cd, int dim, class GridImp>
  inline void ALU3dGridEntity<cd,dim,GridImp> ::
  setGhost(const ALU3DSPACE HBndSegType &ghost)
  {
    // this method only exists, that we don't have to pecialise the
    // Iterators for each codim, this method should not be called otherwise
    // error
    DUNE_THROW(GridError,"This method should not be called!");
  }

  template<int cd, int dim, class GridImp>
  inline int ALU3dGridEntity<cd,dim,GridImp> :: getIndex () const
  {
    return gIndex_;
  }

  template<int cd, int dim, class GridImp>
  inline int ALU3dGridEntity<cd,dim,GridImp> :: level () const
  {
    return level_;
  }

  template<int cd, int dim, class GridImp>
  inline PartitionType ALU3dGridEntity<cd,dim,GridImp> ::
  partitionType () const
  {
    return InteriorEntity;
  }

  template<int cd, int dim, class GridImp>
  inline const typename ALU3dGridEntity<cd,dim,GridImp>::Geometry &
  ALU3dGridEntity<cd,dim,GridImp>:: geometry() const
  {
    //assert( (cd == 1) ? (face_ >= 0) : 1 );
    if(!builtgeometry_) builtgeometry_ = geo_.buildGeom(*item_, twist_, face_ );
    return geo_;
  }

  template<int cd, int dim, class GridImp>
  inline typename ALU3dGridEntity<cd,dim,GridImp>::EntityPointer
  ALU3dGridEntity<cd,dim,GridImp>:: ownersFather() const
  {
    assert(cd == dim); // this method only exists for codim == dim
    if( !father_ )
    {
      dwarn << "No Father for given Entity! \n";
      return ALU3dGridEntityPointer<0,GridImp> (grid_,(*father_));
    }
    return ALU3dGridEntityPointer<0,GridImp> (grid_,(*father_));
  }

  template<int cd, int dim, class GridImp>
  inline FieldVector<alu3d_ctype, dim> &
  ALU3dGridEntity<cd,dim,GridImp>:: positionInOwnersFather() const
  {
    assert( cd == dim );
    if(!localFCoordCalced_)
    {
      EntityPointer vati = this->ownersFather();
      localFatherCoords_ = (*vati).geometry().local( this->geometry()[0] );
      localFCoordCalced_ = true;
    }
    return localFatherCoords_;
  }

  // --0Entity
  template <int dim, class GridImp>
  const typename ALU3dGridEntity<0, dim, GridImp>::ReferenceElementType
  ALU3dGridEntity<0, dim, GridImp>::refElem_;

  template<int dim, class GridImp>
  inline ALU3dGridEntity<0,dim,GridImp> ::
  ALU3dGridEntity(const GridImp  &grid, int wLevel)
    : grid_(grid)
      , item_(0)
      //, ghost_(0)
      , isGhost_(false), geo_() , builtgeometry_(false)
      , walkLevel_ (wLevel)
      //, glIndex_(-1)
      //, level_(-1)
      , geoInFather_ ()
      , isLeaf_ (false)
  {  }

  template<int dim, class GridImp>
  inline void ALU3dGridEntity<0,dim,GridImp> ::
  removeElement ()
  {
    item_  = 0;
    //ghost_ = 0;
  }

  template<int dim, class GridImp>
  inline void ALU3dGridEntity<0,dim,GridImp> ::
  reset (int walkLevel )
  {
    assert( walkLevel_ >= 0 );

    item_       = 0;
    //ghost_      = 0;
    isGhost_    = false;
    builtgeometry_ = false;
    walkLevel_     = walkLevel;
    //glIndex_    = -1;
    //level_      = -1;
    isLeaf_     = false;
  }

  // works like assignment
  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp> :: setEntity(const ALU3dGridEntity<0,dim,GridImp> & org)
  {
    item_          = org.item_;
    isGhost_       = org.isGhost_;
    //ghost_         = org.ghost_;
    builtgeometry_ = false;
    //level_         = org.level_;
    walkLevel_     = org.walkLevel_;
    //glIndex_       = org.glIndex_;
    isLeaf_        = org.isLeaf_;
  }

  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp>::
  setElement(ALU3DSPACE HElementType & element, int, int)
  {
    // int argument (twist) is only a dummy parameter here,
    // needed for consistency reasons
    //if(item_) if(glIndex_ == element.getIndex()) return;

    item_= static_cast<IMPLElementType *> (&element);
    isGhost_ = false;
    //ghost_ = 0;
    builtgeometry_=false;
    //level_   = (*item_).level();
    //glIndex_ = (*item_).getIndex();
    isLeaf_  = ((*item_).down() == 0);

    /*
       std::cout << "Elements[" << glIndex_ << "] child number is " << item_->nChild() << "\n";
       for(int i=0; i<item_->nFaces(); ++i)
       {
       std::cout << item_->myhface4(i)->nChild() << " ";
       }
     */
    /*
       std::cout << "Twist: ";
       for (int i = 0; i < item_->nFaces(); ++i) {
       std::cout << item_->twist(i) << ", ";
       }
       std::cout << std::endl;
     */
  }

  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp> :: setGhost(ALU3DSPACE HBndSegType & ghost)
  {
#ifdef __USE_INTERNAL_FACES__
    // use internal faces as ghost
    typedef typename ALU3dImplTraits<GridImp::elementType>::PLLBndFaceType PLLBndFaceType;
    item_    = 0;
    //ghost_   = static_cast<PLLBndFaceType *> (&ghost);
    //glIndex_ = ghost_->getIndex();
    //level_   = ghost_->level();
#else
    // use element as ghost
    typedef typename ALU3dImplTraits<GridImp::elementType>::IMPLElementType IMPLElementType;
    item_    = static_cast<IMPLElementType *> (ghost.getGhost());
    assert(item_);
    //ghost_   = 0;
    //glIndex_ = item_->getIndex();
    //level_   = item_->level();
    int level_ = item_->level();
#endif
    isGhost_ = true;
    builtgeometry_ = false;

    PLLBndFaceType * dwn =  static_cast<PLLBndFaceType *> (ghost.down());
    if ( ! dwn ) isLeaf_ = true;
    else
    {
      assert( ghost.level() == level_ );
      if(dwn->ghostLevel() == level_) isLeaf_ = true;
      else isLeaf_ = false;
    }
    // check wether ghost is leaf or not, ghost leaf means
    // that this is the ghost that we want in the leaf iterator
    // not necessarily is real leaf element
    // see Intersection Iterator, same story
  }

  template<int dim, class GridImp>
  inline int
  ALU3dGridEntity<0,dim,GridImp> :: level() const
  {
    //return level_;
    assert( item_ );
    return (*item_).level();
  }

  template<int dim, class GridImp>
  inline bool ALU3dGridEntity<0,dim,GridImp> ::
  equals (const ALU3dGridEntity<0,dim,GridImp> &org ) const
  {
    return (item_ == org.item_);
    //return ( (item_ == org.item_) && (ghost_ == org.ghost_) );
  }

  template<int dim, class GridImp>
  inline const typename ALU3dGridEntity<0,dim,GridImp>::Geometry &
  ALU3dGridEntity<0,dim,GridImp> :: geometry () const
  {
    //assert((ghost_ != 0) || (item_ != 0));
    assert(item_ != 0);
#ifdef _ALU3DGRID_PARALLEL_
    if(!builtgeometry_)
    {
      if(item_)
        builtgeometry_ = geo_.buildGeom(*item_);
      /*
         else
         {
          assert(ghost_);
          builtgeometry_ = geo_.buildGhost(*ghost_);
         }
       */
    }
#else
    if(!builtgeometry_) builtgeometry_ = geo_.buildGeom(*item_);
#endif
    return geo_;
  }

  template<int dim, class GridImp>
  inline const typename ALU3dGridEntity<0,dim,GridImp>::Geometry &
  ALU3dGridEntity<0,dim,GridImp> :: geometryInFather () const
  {
    const typename GridImp::template Codim<0> ::
    EntityPointer ep = father();

    geoInFather_.buildGeomInFather( (*ep).geometry() , geometry() );
    return geoInFather_;
  }

  template<int dim, class GridImp>
  inline int ALU3dGridEntity<0,dim,GridImp> :: getIndex() const
  {
    assert( item_ );
    return (*item_).getIndex();
  }

  //********* begin method subIndex ********************
  // partial specialisation of subIndex
  template <class IMPLElemType, ALU3dGridElementType type, int codim>
  struct IndexWrapper {};

  // specialisation for vertices
  template <class IMPLElemType, ALU3dGridElementType type>
  struct IndexWrapper<IMPLElemType, type, 3>
  {
    typedef ElementTopologyMapping<type> Topo;

    static inline int subIndex(const IMPLElemType &elem, int i)
    {
      return elem.myvertex( Topo::dune2aluVertex(i) )->getIndex(); // element topo
    }
  };

  // specialisation for faces
  template <class IMPLElemType, ALU3dGridElementType type>
  struct IndexWrapper<IMPLElemType, type , 1>
  {
    static inline int subIndex(const IMPLElemType &elem, int i)
    {
      // is specialised for each element type and uses
      // the dune2aluFace mapping
      return (getFace(elem,i))->getIndex();
    }
  };

  // specialisation for edges
  template <class IMPLElemType, ALU3dGridElementType type>
  struct IndexWrapper<IMPLElemType, type, 2>
  {
    typedef ElementTopologyMapping<type> Topo;

    // return subIndex of given edge
    static inline int subIndex(const IMPLElemType &elem, int i)
    {
      // get hedge1 corresponding to dune reference element and return number
      return elem.myhedge1( Topo::dune2aluEdge(i) )->getIndex();
    }
  };

  // specialisation for elements
  template <class IMPLElemType, ALU3dGridElementType type>
  struct IndexWrapper<IMPLElemType, type, 0>
  {
    static inline int subIndex(const IMPLElemType &elem, int i) {
      // just return the elements index
      return elem.getIndex();
    }
  };

  template<int dim, class GridImp>
  template<int cc>
  inline int ALU3dGridEntity<0,dim,GridImp> :: getSubIndex (int i) const
  {
    assert(item_ != 0);
    typedef typename  ALU3dImplTraits<GridImp::elementType>::IMPLElementType IMPLElType;
    return IndexWrapper<IMPLElType,GridImp::elementType,cc>::subIndex ( *item_, i);
  }

  //******** end method count *************
  template<int dim, class GridImp>
  template<int cc>
  inline int ALU3dGridEntity<0,dim,GridImp> :: count () const
  {
    return refElem_.size(cc);
  }

  //******** begin method entity ******************
  template <class GridImp, int dim, int cd> struct SubEntities {};

  // specialisation for elements
  template <class GridImp, int dim>
  struct SubEntities<GridImp, dim, 0>
  {
    typedef ALU3dGridEntity<0,dim,GridImp> EntityType;

    static typename ALU3dGridEntity<0,dim,GridImp>::template Codim<0>::EntityPointer
    entity (const GridImp & grid,
            const EntityType & en,
            const typename ALU3dImplTraits<GridImp::elementType>::IMPLElementType & item,
            int i) {
      return ALU3dGridEntityPointer<0, GridImp>(grid, item);
    }
  };

  // specialisation for faces
  template <class GridImp, int dim>
  struct SubEntities<GridImp,dim,1>
  {
    typedef ElementTopologyMapping<GridImp::elementType> Topo;
    typedef ALU3dGridEntity<0,dim,GridImp> EntityType;

    static typename ALU3dGridEntity<0,dim,GridImp> :: template Codim<1>:: EntityPointer
    entity (const GridImp& grid,
            const EntityType & en,
            const typename ALU3dImplTraits<GridImp::elementType>::IMPLElementType & item,
            int duneFace)
    {
      int aluFace = Topo::dune2aluFace(duneFace);
      return
        ALU3dGridEntityPointer<1,GridImp>
          (grid,
          *(getFace(item, duneFace)),    // getFace already constains dune2aluFace
          item.twist(aluFace),
          duneFace    // we need the duneFace number here for the buildGeom method
          );
    }
  };

  // specialisation for edges
  template <class GridImp, int dim>
  struct SubEntities<GridImp,dim,2>
  {
    typedef ElementTopologyMapping<GridImp::elementType> Topo;
    typedef ALU3dGridEntity<0,dim,GridImp> EntityType;
    typedef typename GridImp::ctype coordType;

    typedef typename GridImp :: ReferenceElementType ReferenceElementType;

    static typename ALU3dGridEntity<0,dim,GridImp> :: template Codim<2>:: EntityPointer
    entity (const GridImp & grid,
            const EntityType & en,
            const typename ALU3dImplTraits<GridImp::elementType>::IMPLElementType & item,
            int i)
    {
      // get reference element
      const ReferenceElementType & refElem = grid.referenceElement();

      // get first local vertex number of edge i
      int localNum = refElem.subEntity(i,2,0,dim);

      // get number of first vertex on edge
      int v = en.template getSubIndex<dim> (localNum);

      // get the hedge object
      const typename ALU3dImplTraits<GridImp::elementType>::GEOEdgeType &
      edge = *(item.myhedge1(Topo::dune2aluEdge(i)));

      int vx = edge.myvertex(0)->getIndex();

      // check whether vertex numbers are equal, otherwise twist is 1
      int twst = (v != vx) ? 1 : 0;
      return ALU3dGridEntityPointer<2,GridImp> (grid, edge, twst );
    }
  };

  // specialisation for vertices
  template <class GridImp, int dim>
  struct SubEntities<GridImp,dim,3>
  {
    typedef ElementTopologyMapping<GridImp::elementType> Topo;
    typedef ALU3dGridEntity<0,dim,GridImp> EntityType;

    static typename ALU3dGridEntity<0,dim,GridImp> :: template Codim<3>:: EntityPointer
    entity (const GridImp & grid,
            const EntityType & en,
            const typename ALU3dImplTraits<GridImp::elementType>::IMPLElementType & item,
            int i)
    {
      return ALU3dGridEntityPointer<3,GridImp>
               (grid, *item.myvertex(Topo::dune2aluVertex(i))); // element topo
    }
  };

  template<int dim, class GridImp>
  template<int cc>
  inline typename ALU3dGridEntity<0,dim,GridImp>::template Codim<cc>:: EntityPointer
  ALU3dGridEntity<0,dim,GridImp> :: entity (int i) const
  {
    return SubEntities<GridImp,dim,cc>::entity(grid_,*this,*item_,i);
  }

  //**** end method entity *********

  template<int dim, class GridImp>
  inline PartitionType ALU3dGridEntity<0,dim,GridImp> ::
  partitionType () const
  {
    return ((isGhost_) ?  GhostEntity : InteriorEntity);
  }

  template<int dim, class GridImp>
  inline bool ALU3dGridEntity<0,dim,GridImp> :: isLeaf() const
  {
    return isLeaf_;
  }
  template<int dim, class GridImp>
  inline ALU3dGridHierarchicIterator<GridImp> ALU3dGridEntity<0,dim,GridImp> :: hbegin (int maxlevel) const
  {
    assert(item_ != 0);
    return ALU3dGridHierarchicIterator<GridImp>(grid_,*item_,maxlevel);
  }

  template<int dim, class GridImp>
  inline ALU3dGridHierarchicIterator<GridImp> ALU3dGridEntity<0,dim,GridImp> :: hend (int maxlevel) const
  {
    assert(item_ != 0);
    return ALU3dGridHierarchicIterator<GridImp> (grid_,*item_,maxlevel,true);
  }

  template<int dim, class GridImp>
  inline typename ALU3dGridEntity<0,dim,GridImp> :: ALU3dGridIntersectionIteratorType
  ALU3dGridEntity<0,dim,GridImp> :: ibegin () const
  {
    assert(item_ != 0);

    // NOTE: normaly here false should be given, which means that we create a non
    // end iterator, but isGhost_ is normaly false. If isGhost_ is true,
    // an end iterator is created,
    // because on ghosts we dont run itersection iterators
    return ALU3dGridIntersectionIteratorType (grid_,*this,walkLevel_, isGhost_ );
  }

  template<int dim, class GridImp>
  inline typename ALU3dGridEntity<0,dim,GridImp> :: ALU3dGridIntersectionIteratorType
  ALU3dGridEntity<0,dim,GridImp> :: iend () const
  {
    assert(item_ != 0);
    return ALU3dGridIntersectionIteratorType (grid_, *this ,walkLevel_,true);
  }

  template<int dim, class GridImp>
  inline typename ALU3dGridEntity<0,dim,GridImp> :: EntityPointer
  ALU3dGridEntity<0,dim,GridImp> :: father() const
  {
    if(! item_->up() )
    {
      std::cerr << "ALU3dGridEntity<0," << dim << "," << dimworld << "> :: father() : no father of entity globalid = " << getIndex() << "\n";
      return ALU3dGridEntityPointer<0,GridImp> (grid_, static_cast<ALU3DSPACE HElementType &> (*item_));
    }
    return ALU3dGridEntityPointer<0,GridImp> (grid_, static_cast<ALU3DSPACE HElementType &> (*(item_->up())));
  }

  // Adaptation methods
  template<int dim, class GridImp>
  inline bool ALU3dGridEntity<0,dim,GridImp> :: mark (int ref) const
  {
    // refine_element_t and coarse_element_t are defined in bsinclude.hh
    //if(ghost_) return false;

    assert(item_ != 0);

    // if this assertion is thrown then you try to mark a non leaf entity
    // which is leads to unpredictable results
    assert( isLeaf() );

    // mark for coarsening
    if(ref < 0)
    {
      if(level() <= 0) return false;
      if((*item_).requestrule() == refine_element_t)
      {
        return false;
      }

      (*item_).request(coarse_element_t);
      return true;
    }

    // mark for refinement
    if(ref > 0)
    {
      (*item_).request(refine_element_t);
      return true;
    }

    (*item_).request( nosplit_element_t );
    return false;
  }


  // Adaptation methods
  template<int dim, class GridImp>
  inline AdaptationState ALU3dGridEntity<0,dim,GridImp> :: state () const
  {
    //assert((item_ != 0) || (ghost_ != 0));
    assert(item_ != 0);
    if(item_)
    {
      if((*item_).requestrule() == coarse_element_t)
      {
        return COARSEN;
      }

      if(item_->hasBeenRefined())
      {
        return REFINED;
      }

      return NONE;
    }
    // is case of ghosts return none, because they are not considered
    return NONE;
  }


  //*******************************************************************
  //
  //  --EntityPointer
  //  --EnPointer
  //
  //*******************************************************************

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointerBase<codim,GridImp> ::
  ALU3dGridEntityPointerBase(const GridImp & grid,
                             const MyHElementType &item)
    : grid_(grid)
      , item_(const_cast<MyHElementType *> (&item))
      , entity_(0)
  {}

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointerBase<codim,GridImp> ::
  ALU3dGridEntityPointerBase(const GridImp & grid,
                             const HBndSegType & ghostFace )
    : grid_(grid)
      , item_(0)
      , entity_ ( grid_.template getNewEntity<codim> ( ghostFace.level() ))
  {
    // sets entity and item pointer
    updateGhostPointer( const_cast<HBndSegType &> (ghostFace) );
  }

  // constructor Level,Leaf and HierarchicIterator
  template<int codim, class GridImp >
  inline ALU3dGridEntityPointerBase<codim,GridImp> ::
  ALU3dGridEntityPointerBase(const GridImp & grid, int level )
    : grid_(grid)
      , item_(0)
      , entity_ ( grid_.template getNewEntity<codim> ( level ) )
  {
    // this needs to be called
    // have to investigate why
    (*entity_).reset(level);
  }

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointerBase<codim,GridImp> ::
  ALU3dGridEntityPointerBase(const ALU3dGridEntityPointerType & org)
    : grid_(org.grid_)
      , item_(org.item_)
      , entity_(0)
  {
    if(org.entity_)
    {
      int level = org.entity_->level();
      entity_ = grid_.template getNewEntity<codim> ( level );
      (*entity_).setEntity( *(org.entity_) );
    }
  }

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointerBase<codim,GridImp> ::
  ~ALU3dGridEntityPointerBase()
  {
    if(entity_)
    {
      //(*entity_).removeElement();
      grid_.freeEntity( entity_ );
      //entity_ = 0;
    }
    //item_ = 0;
  }

  template<int codim, class GridImp >
  inline void ALU3dGridEntityPointerBase<codim,GridImp>::done ()
  {
    item_ = 0;
    // sets entity pointer in the status of an empty entity
    if(entity_)
    {
      (*entity_).removeElement();
      grid_.freeEntity( entity_ );
      entity_ = 0;
    }
  }

  template<int codim, class GridImp >
  inline bool ALU3dGridEntityPointerBase<codim,GridImp>::
  equals (const ALU3dGridEntityPointerBase<codim,GridImp>& i) const
  {
    // check equality of underlying items
    return (item_ == i.item_);
  }

  template<int codim, class GridImp >
  inline typename ALU3dGridEntityPointerBase<codim,GridImp>::Entity &
  ALU3dGridEntityPointerBase<codim,GridImp>::dereference () const
  {
    // don't dereference empty entity pointer
    assert( item_ );
    if(!entity_)
    {
      entity_ = grid_.template getNewEntity<codim> ( item_->level() );
      (*entity_).setElement( *item_ );
    }
    assert( item_ == & (*entity_).getItem() );
    return (*entity_);
  }

  template<int codim, class GridImp >
  inline int ALU3dGridEntityPointerBase<codim,GridImp>::level () const
  {
    assert( item_ );
    return item_->level();
  }

  template<int codim, class GridImp >
  inline void ALU3dGridEntityPointerBase<codim,GridImp>::
  updateGhostPointer( ALU3DSPACE HBndSegType & ghostFace )
  {
    assert( entity_ );
    (*entity_).setGhost( ghostFace );
    // inside the method setGhost the method getGhost of the ghostFace is
    // called and set as item
    item_ = const_cast<MyHElementType *> (& ((*entity_).getItem()));
  }

  template<int codim, class GridImp >
  inline void ALU3dGridEntityPointerBase<codim,GridImp>::
  updateEntityPointer( MyHElementType * item )
  {
    item_ = item;
    if( item_ && entity_ )
    {
      (*entity_).setElement( *item_ );
    }
  }

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointer<codim,GridImp> ::
  ALU3dGridEntityPointer(const GridImp & grid,
                         const MyHElementType &item,
                         const int twist,
                         const int duneFace )
    : ALU3dGridEntityPointerBase<codim,GridImp> (grid,item)
      , twist_ (twist)
      , face_(duneFace)
  {
    assert( (codim == 1) ? (face_ >= 0) : 1 );
  }

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointer<codim,GridImp> ::
  ALU3dGridEntityPointer(const ALU3dGridEntityPointerType & org)
    : ALU3dGridEntityPointerBase<codim,GridImp>(org)
      , twist_(org.twist_)
      , face_(org.face_)
  {}

  template<int codim, class GridImp >
  inline typename ALU3dGridEntityPointer<codim,GridImp>::Entity &
  ALU3dGridEntityPointer<codim,GridImp>::dereference () const
  {
    // don't dereference empty entity pointer
    assert( this->item_ );
    if(!this->entity_)
    {
      this->entity_ = this->grid_.template getNewEntity<codim> ( this->level() );
      (*this->entity_).setElement( *this->item_ , twist_ , face_ );
    }
    assert( this->item_ == & (*this->entity_).getItem() );
    return (*this->entity_);
  }

} // end namespace Dune
