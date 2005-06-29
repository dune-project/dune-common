// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "geometry.hh"
#include "grid.hh"

namespace Dune {

  // --Entity
  template <int cd, int dim, class GridImp>
  inline ALU3dGridEntity<cd,dim,GridImp> ::
  ALU3dGridEntity(const GridImp  &grid, int level)
    : grid_(grid), level_(0) , gIndex_(-1)
      , item_(0) , father_(0)
      , geo_(false) , builtgeometry_(false)
      , localFCoordCalced_ (false)
  {}

  template<int cd, int dim, class GridImp>
  inline void ALU3dGridEntity<cd,dim,GridImp> ::
  reset( int l )
  {
    item_  = 0;
    level_ = l;
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
    level_  = org.level_;
    father_ = org.father_;
    builtgeometry_= false;
    localFCoordCalced_ = false;
  }

  template<int cd, int dim, class GridImp>
  inline void ALU3dGridEntity<cd,dim,GridImp> :: setElement(const BSElementType & item)
  {
    item_   = static_cast<const BSIMPLElementType *> (&item);
    gIndex_ = (*item_).getIndex();
    level_  = (*item_).level();
    builtgeometry_=false;
    localFCoordCalced_ = false;
  }

  template<>
  inline void ALU3dGridEntity<3,3,const ALU3dGrid<3,3,hexa> > ::
  setElement(const ALU3DSPACE HElementType &el, const ALU3DSPACE VertexType &vx)
  {
    item_   = static_cast<const BSIMPLElementType *> (&vx);
    gIndex_ = (*item_).getIndex();
    father_ = static_cast<const ALU3DSPACE HElementType *> (&el);
    builtgeometry_=false;
    localFCoordCalced_ = false;
  }

  template<>
  inline void ALU3dGridEntity<3,3,const ALU3dGrid<3,3,tetra> > ::
  setElement(const ALU3DSPACE HElementType &el, const ALU3DSPACE VertexType &vx)
  {
    item_   = static_cast<const BSIMPLElementType *> (&vx);
    gIndex_ = (*item_).getIndex();
    level_  = (*item_).level();
    father_ = static_cast<const ALU3DSPACE HElementType *> (&el);
    builtgeometry_=false;
    localFCoordCalced_ = false;
  }

  template<int cd, int dim, class GridImp>
  inline int ALU3dGridEntity<cd,dim,GridImp> :: index () const
  {
    const Entity en (*this);
    return grid_.levelIndexSet().index(en);
  }

  template<int cd, int dim, class GridImp>
  inline int ALU3dGridEntity<cd,dim,GridImp> :: globalIndex () const
  {
    return gIndex_;
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
  inline const typename ALU3dGridEntity<cd,dim,GridImp>::Geometry &
  ALU3dGridEntity<cd,dim,GridImp>:: geometry() const
  {
    if(!builtgeometry_) builtgeometry_ = geo_.buildGeom(*item_);
    return geo_;
  }

  template<int cd, int dim, class GridImp>
  inline typename ALU3dGridEntity<cd,dim,GridImp>::EntityPointer
  ALU3dGridEntity<cd,dim,GridImp>:: ownersFather() const
  {
    assert(cd == dim);
    assert(father_);

    ALU3dGridLevelIterator<cd,All_Partition,const GridImp> vati(grid_,(*father_));
    return vati;
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
  template<int dim, class GridImp>
  inline ALU3dGridEntity<0,dim,GridImp> ::
  ALU3dGridEntity(const GridImp  &grid, int wLevel)
    : grid_(grid)
      , item_(0)
      , ghost_(0), isGhost_(false), geo_(false) , builtgeometry_(false)
      , walkLevel_ (wLevel)
      , glIndex_(-1), level_(-1)
      , geoInFather_ (false)
      , isLeaf_ (false)
  {}

  template<int dim, class GridImp>
  inline void ALU3dGridEntity<0,dim,GridImp> ::
  removeElement ()
  {
    item_  = 0;
    ghost_ = 0;
  }

  template<int dim, class GridImp>
  inline void ALU3dGridEntity<0,dim,GridImp> ::
  reset (int walkLevel )
  {
    assert( walkLevel_ >= 0 );

    item_       = 0;
    ghost_      = 0;
    isGhost_    = false;
    builtgeometry_ = false;
    walkLevel_     = walkLevel;
    glIndex_    = -1;
    level_      = -1;
    isLeaf_     = false;
  }

  // works like assignment
  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp> :: setEntity(const ALU3dGridEntity<0,dim,GridImp> & org)
  {
    item_          = org.item_;
    isGhost_       = org.isGhost_;
    ghost_         = org.ghost_;
    builtgeometry_ = false;
    index_         = org.index_;
    level_         = org.level_;
    walkLevel_     = org.walkLevel_;
    glIndex_       = org.glIndex_;
    isLeaf_        = org.isLeaf_;
  }

  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp> :: setElement(ALU3DSPACE HElementType & element)
  {
    if(item_ != static_cast<IMPLElementType *> (&element) )
    {
      item_= static_cast<IMPLElementType *> (&element);
      isGhost_ = false;
      ghost_ = 0;
      builtgeometry_=false;
      index_   = -1;
      level_   = (*item_).level();
      glIndex_ = (*item_).getIndex();
      isLeaf_  = ((*item_).down() == 0);
    }
  }

  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp> :: setGhost(ALU3DSPACE HElementType & element)
  {
    if(item_ != & element)
    {
      item_= static_cast<IMPLElementType *> (&element);
      isGhost_ = true;
      ghost_ = 0;
      builtgeometry_=false;
      index_   = -1;
      level_   = (*item_).level();
      glIndex_ = (*item_).getIndex();

      // check wether ghost is leaf or not, ghost leaf means
      // that this is the ghost that we want in the leaf iterator
      // not necessarily is real leaf element
      // see Intersection Iterator, same story
      if(!(*item_).down()) isLeaf_ = true;
      else
      {
        if(item_->down()->ghostLevel() == level_) isLeaf_ = true;
        else isLeaf_ = false;
      }
    }
  }

  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp> :: setGhost(ALU3DSPACE HBndSegType & ghost)
  {
    if(ghost_ != & ghost)
    {
      typedef typename ALU3dImplTraits<GridImp::elementType>::PLLBndFaceType PLLBndFaceType;
      item_    = 0;
      ghost_   = static_cast<PLLBndFaceType *> (&ghost);
      isGhost_ = true;
      index_   = -1;
      glIndex_ = ghost_->getIndex();
      level_   = ghost_->level();
      builtgeometry_ = false;

      // check wether ghost is leaf or not, ghost leaf means
      // that this is the ghost that we want in the leaf iterator
      // not necessarily is real leaf element
      // see Intersection Iterator, same story
      PLLBndFaceType * dwn =  static_cast<PLLBndFaceType *> (ghost_->down());
      if ( ! dwn ) isLeaf_ = true;
      else
      {
        if(dwn->ghostLevel() == level_) isLeaf_ = true;
        else isLeaf_ = false;
      }
    }
  }

  template<int dim, class GridImp>
  inline int
  ALU3dGridEntity<0,dim,GridImp> :: level() const
  {
    return level_;
  }

  template<int dim, class GridImp>
  inline bool ALU3dGridEntity<0,dim,GridImp> ::
  equals (const ALU3dGridEntity<0,dim,GridImp> &org ) const
  {
    return ( (item_ == org.item_) && (ghost_ == org.ghost_) );
  }

  template<int dim, class GridImp>
  inline const typename ALU3dGridEntity<0,dim,GridImp>::Geometry &
  ALU3dGridEntity<0,dim,GridImp> :: geometry () const
  {
    assert((ghost_ != 0) || (item_ != 0));
#ifdef _ALU3DGRID_PARALLEL_
    if(!builtgeometry_)
    {
      if(item_)
        builtgeometry_ = geo_.buildGeom(*item_);
      else
      {
        assert(ghost_);
        builtgeometry_ = geo_.buildGhost(*ghost_);
      }
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
    const Geometry & vati   = (*this->father()).geometry();
    const Geometry & myself = this->geometry();

    for(int i=0; i<vati.corners(); i++)
      geoInFather_.getCoordVec(i) = vati.local( myself[i] );

    return geoInFather_;
  }

  template<int dim, class GridImp>
  inline int ALU3dGridEntity<0,dim,GridImp> :: index() const
  {
    const Entity en (*this);
    return grid_.levelIndexSet().index(en);
  }

  template<int dim, class GridImp>
  inline int ALU3dGridEntity<0,dim,GridImp> :: globalIndex() const
  {
    return glIndex_;
  }

  template<int dim, class GridImp>
  inline int ALU3dGridEntity<0,dim,GridImp> :: getIndex() const
  {
    return glIndex_;
  }

  //********* begin method subIndex ********************
  // partial specialisation of subIndex
  template <class IMPLElemType, int codim> struct IndexWrapper;

  // specialisation for vertices
  template <class IMPLElemType> struct IndexWrapper<IMPLElemType,3>
  {
    static inline int subIndex(const IMPLElemType &elem, int i)
    {
      return elem.myvertex(i)->getIndex();
    }
  };

  // specialisation for faces
  template <class IMPLElemType> struct IndexWrapper<IMPLElemType,1>
  {
    static inline int subIndex(const IMPLElemType &elem, int i)
    {
      return elem.myhface3(i)->getIndex();
    }
  };

  // specialisation for faces
  template <class IMPLElemType> struct IndexWrapper<IMPLElemType,2>
  {
    static inline int subIndex(const IMPLElemType &elem, int i)
    {
      dwarn << "method not tested yet. ! in:" << __FILE__ << " line:" << __LINE__ << "\n";
      if(i<3)
        return elem.myhface3(0)->myhedge1(i)->getIndex();
      else
        return elem.myhface3(i-2)->myhedge1(i-3)->getIndex();
    }
  };

  template<int dim, class GridImp>
  template<int cc>
  inline int ALU3dGridEntity<0,dim,GridImp> :: subIndex (int i) const
  {
    assert(cc == dim);
    assert(item_ != 0);
    typedef typename  ALU3dImplTraits<GridImp::elementType>::IMPLElementType IMPLElType;
    return IndexWrapper<IMPLElType,cc>::subIndex ( *item_ ,i);
  }
  //******** end method subIndex *************

  template <class GridImp, int dim, int cc> struct ALU3dGridCount {
    static int count () { return dim+1; }
  };
  template <class GridImp> struct ALU3dGridCount<GridImp,3,2> {
    static int count () { return 6; }
  };

  template<int dim, class GridImp>
  template<int cc>
  inline int ALU3dGridEntity<0,dim,GridImp> :: count () const
  {
    return ALU3dGridCount<GridImp,dim,cc>::count();
  }

  //******** begin method entity ******************
  template <class GridImp, int dim, int cd> struct SubEntities;

  // specialisation for faces
  template <class GridImp, int dim>
  struct SubEntities<GridImp,dim,1>
  {
    static typename ALU3dGridEntity<0,dim,GridImp> :: template codim<1>:: EntityPointer
    entity (const GridImp & grid, const typename ALU3dImplTraits<GridImp::elementType>::IMPLElementType & item, int i)
    {
      return ALU3dGridEntityPointer<1,GridImp> (grid, *(item.myhface3(i)) );
    }
  };

  // specialisation for edges
  template <class GridImp, int dim>
  struct SubEntities<GridImp,dim,2>
  {
    static typename ALU3dGridEntity<0,dim,GridImp> :: template codim<2>:: EntityPointer
    entity (const GridImp & grid, const typename ALU3dImplTraits<GridImp::elementType>::IMPLElementType & item, int i)
    {
      dwarn << "method not tested yet. ! in:" << __FILE__ << " line:" << __LINE__ << "\n";
      if(i<3)
      {
        return ALU3dGridEntityPointer<2,GridImp> (grid, (*(item.myhface3(0)->myhedge1(i))) );
      }
      else
      {
        return ALU3dGridEntityPointer<2,GridImp> (grid, (*(item.myhface3(i-2)->myhedge1(i-3))) );
      }
    }
  };

  // specialisation for vertices
  template <class GridImp, int dim>
  struct SubEntities<GridImp,dim,3>
  {
    static typename ALU3dGridEntity<0,dim,GridImp> :: template codim<3>:: EntityPointer
    entity (const GridImp & grid, const typename ALU3dImplTraits<GridImp::elementType>::IMPLElementType & item, int i)
    {
      return ALU3dGridEntityPointer<3,GridImp> (grid, (*(item.myvertex(i))) );
    }
  };

  template<int dim, class GridImp>
  template<int cc>
  inline typename ALU3dGridEntity<0,dim,GridImp> :: template codim<cc>:: EntityPointer
  ALU3dGridEntity<0,dim,GridImp> :: entity (int i) const
  {
    return SubEntities<GridImp,dim,cc>::entity(grid_,*item_,i);
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
  inline ALU3dGridIntersectionIterator<GridImp> ALU3dGridEntity<0,dim,GridImp> :: ibegin () const
  {
    assert(item_ != 0);
    return ALU3dGridIntersectionIterator<GridImp> (grid_,item_,walkLevel_);
  }

  template<int dim, class GridImp>
  inline ALU3dGridIntersectionIterator<GridImp> ALU3dGridEntity<0,dim,GridImp> :: iend () const
  {
    assert(item_ != 0);
    return ALU3dGridIntersectionIterator<GridImp> (grid_, 0 ,walkLevel_,true);
  }

  template<int dim, class GridImp>
  inline typename ALU3dGridEntity<0,dim,GridImp> :: EntityPointer
  ALU3dGridEntity<0,dim,GridImp> :: father() const
  {
    if(! item_->up() )
    {
      std::cerr << "ALU3dGridEntity<0," << dim << "," << dimworld << "> :: father() : no father of entity globalid = " << globalIndex() << "\n";
      return ALU3dGridEntityPointer<0,GridImp> (grid_, static_cast<ALU3DSPACE HElementType &> (*item_));
    }
    return ALU3dGridEntityPointer<0,GridImp> (grid_, static_cast<ALU3DSPACE HElementType &> (*(item_->up())));
  }

  // Adaptation methods
  template<int dim, class GridImp>
  inline bool ALU3dGridEntity<0,dim,GridImp> :: mark (int ref) const
  {
    // refine_element_t and coarse_element_t are defined in bsinclude.hh
    if(ghost_) return false;

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
    assert((item_ != 0) || (ghost_ != 0));
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
  inline ALU3dGridEntityPointer<codim,GridImp> ::
  ALU3dGridEntityPointer(const GridImp & grid, const MyHElementType &item)
    : grid_(grid)
      , entity_ ( grid_.template getNewEntity<codim> ( item.level() ) )
      , done_ (false)
  {
    assert( entity_ );
    (*entity_).setElement( const_cast<MyHElementType &> (item) );
  }

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointer<codim,GridImp> ::
  ALU3dGridEntityPointer(const GridImp & grid, int level , bool done )
    : grid_(grid)
      , entity_ ( grid_.template getNewEntity<codim> (level) )
      , done_ (done)
  {
    (*entity_).reset( level );
  }

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointer<codim,GridImp> ::
  ALU3dGridEntityPointer(const ALU3dGridEntityPointerType & org)
    : grid_(org.grid_)
      , entity_ ( grid_.template getNewEntity<codim> ( org.entity_->level() ) )
  {
    (*entity_).setEntity( *(org.entity_) );
  }

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointer<codim,GridImp> ::
  ~ALU3dGridEntityPointer()
  {
    grid_.freeEntity( entity_ );
  }

  template<int codim, class GridImp >
  inline void ALU3dGridEntityPointer<codim,GridImp>::done ()
  {
    // sets entity pointer in the status of an end iterator
    (*entity_).removeElement();
    done_ = true;
  }

  template<int codim, class GridImp >
  inline bool ALU3dGridEntityPointer<codim,GridImp>::
  equals (const ALU3dGridEntityPointer<codim,GridImp>& i) const
  {
    return (((*entity_).equals(*(i.entity_))) && (done_ == i.done_));
  }

  template<int codim, class GridImp >
  inline typename ALU3dGridEntityPointer<codim,GridImp>::Entity &
  ALU3dGridEntityPointer<codim,GridImp>::dereference () const
  {
    assert(entity_);
    return (*entity_);
  }

  template<int codim, class GridImp >
  inline int ALU3dGridEntityPointer<codim,GridImp>::level () const
  {
    assert(entity_);
    return (*entity_).level();
  }


  //************************************************************************
  //
  //  --ALU3dGridBoundaryEntity
  //  --BoundaryEntity
  //
  //************************************************************************
  template <class GridImp>
  inline ALU3dGridBoundaryEntity<GridImp>::ALU3dGridBoundaryEntity () :
    _geom (false) , _id(-1) {}

  template <class GridImp>
  inline int ALU3dGridBoundaryEntity<GridImp>::id () const
  {
    return _id;
  }

  template <class GridImp>
  inline bool ALU3dGridBoundaryEntity<GridImp>::hasGeometry () const
  {
    return false;
  }

  template <class GridImp>
  inline const typename ALU3dGridBoundaryEntity<GridImp>::Geometry &
  ALU3dGridBoundaryEntity<GridImp>::geometry () const
  {
    assert(hasGeometry());
    return _geom;
  }

  template <class GridImp>
  inline void ALU3dGridBoundaryEntity<GridImp>::setId ( int id )
  {
    _id = id;
  }

} // end namespace Dune
