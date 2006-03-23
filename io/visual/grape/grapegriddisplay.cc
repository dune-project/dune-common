// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
namespace Dune
{

  //****************************************************************
  //
  // --GrapeGridDisplay, GrapeGridDisplay for given grid
  //
  //****************************************************************

  template<class GridType>
  inline GrapeGridDisplay<GridType>::
  GrapeGridDisplay(const GridType &grid, const int myrank ) :
    grid_(grid)
    , leafset_(grid.leafIndexSet())
    , lid_(grid.localIdSet())
    , myRank_(myrank)
    , hmesh_ (0)
  {
    hel_.liter   = 0;
    hel_.enditer = 0;

    GrapeInterface<dim,dimworld>::init();
    if(!hmesh_) hmesh_ = setupHmesh();
  }

  template<class GridType>
  inline GrapeGridDisplay<GridType>::
  GrapeGridDisplay(const GridType &grid ) :
    grid_(grid)
    , leafset_(grid.leafIndexSet())
    , lid_(grid.localIdSet())
    , myRank_(-1)
    , hmesh_ (0)
  {
    GrapeInterface<dim,dimworld>::init();
    if(!hmesh_) hmesh_ = setupHmesh();
  }

  template<class GridType>
  inline GrapeGridDisplay<GridType>::
  ~GrapeGridDisplay()
  {}

  //****************************************************************
  //
  // --GridDisplay, Some Subroutines needed in display
  //
  //****************************************************************
  /** hmesh functionen **/

  template<class GridType>
  template <class EntityPointerType>
  inline int GrapeGridDisplay<GridType>::
  el_update (EntityPointerType * it, DUNE_ELEM * he)
  {
    typedef typename GridType::Traits::template Codim<0>::Entity Entity;
    typedef typename Entity::IntersectionIterator IntersectionIterator;
    typedef typename Entity::Geometry DuneElement;

    enum { dim      = Entity::dimension };
    enum { dimworld = Entity::dimensionworld };

    Entity &en = (*it[0]);

    // only for debuging, becsaue normaly references are != NULL
    if(&en)
    {
      const DuneElement &geometry = en.geometry();

      //if( en.isLeaf() ) he->eindex = lid_.id(en);
      if( en.isLeaf() ) he->eindex = leafset_.index(en);
      else he->eindex = -1;
      he->level  = en.level();

      // if not true, only the macro level is drawn
      he->has_children = 1;

      // know the type
      int geomType = convertToGrapeType ( geometry.type() , dim );
      he->type = geomType;

      {
        // set the vertex coordinates
        double (* vpointer)[3] = he->vpointer;
        for(int i= 0 ; i<geometry.corners(); i++)
        {
          for(int j = 0; j < Entity::dimensionworld ; j++)
          {
            // here the mapping from dune to grape elements is done
            // it's only different for quads and hexas
            vpointer[i][j] = geometry[ mapDune2GrapeVertex(geomType,i) ][j] ;
          }
        }
      } // end set all vertex coordinates

      // store vertex numbers
      for(int i = 0; i< en.template count<dim>(); i++)
      {
        he->vindex[i] = leafset_. template subIndex<dim> (en,i);
      }

      {
        // reset the boundary information
        for(int i=0; i < MAX_EL_FACE; i++) he->bnd[i] = -1;

        IntersectionIterator endnit = en.iend();
        IntersectionIterator nit    = en.ibegin();

        // value < zero otherwise first test fails
        int lastElNum = -1;

        // check all faces for boundary or not
        while ( nit != endnit )
        {
          int num = nit.numberInSelf();
          assert( num >= 0 );
          assert( num < MAX_EL_FACE );

          if(num != lastElNum)
          {
            he->bnd[num] = ( nit.boundary() ) ? nit.boundaryId() : 0;
            if(nit.neighbor()) if(nit.outside()->partitionType() != InteriorEntity )
                he->bnd[num] = 2*(Entity::dimensionworld) + (nit.numberInSelf()+1);
            lastElNum = num;
          }
          ++nit;
        }
      }

      {
        // for this type of element we have to swap the faces
        if(he->type == g_hexahedron)
        {
          int help_bnd [MAX_EL_FACE];
          for(int i=0; i < MAX_EL_FACE; i++) help_bnd[i] = he->bnd[i] ;

          // do the mapping from dune to grape hexa
          he->bnd[0] = help_bnd[4];
          he->bnd[1] = help_bnd[5];
          he->bnd[3] = help_bnd[1];
          he->bnd[4] = help_bnd[3];
          he->bnd[5] = help_bnd[0];
        }
      }

      // for data displaying
      he->actElement = it;
      return 1;

    } // end if(&en)
    else
    {
      he->actElement = 0;
      return 0;
    }
  }

  template<class GridType>
  template<PartitionIteratorType pitype>
  inline int GrapeGridDisplay<GridType>::
  first_leaf (DUNE_ELEM * he)
  {
    typedef typename GridType :: template Codim<0> ::
    template Partition<pitype> :: LeafIterator LeafIteratorType;

    he->liter   = 0;
    he->enditer = 0;

    LeafIteratorType * it    = new LeafIteratorType ( grid_.template leafbegin<0, pitype> () );
    LeafIteratorType * endit = new LeafIteratorType ( grid_.template leafend  <0, pitype> () );

    if(it[0] == endit[0])
    {
      he->actElement = 0;
      delete it;
      delete endit;
      return 0;
    }

    he->liter   = (void *) it;
    he->enditer = (void *) endit;
    return el_update(it,he);
  }

  template<class GridType>
  template<PartitionIteratorType pitype>
  inline int GrapeGridDisplay<GridType>::
  next_leaf (DUNE_ELEM * he)
  {
    typedef typename GridType :: template Codim<0> ::
    template Partition<pitype> :: LeafIterator LeafIteratorType;

    LeafIteratorType * it    = (LeafIteratorType *) he->liter;
    LeafIteratorType * endit = (LeafIteratorType *) he->enditer;
    assert( it );
    assert( endit );

    if( ++it[0] != endit[0] )
    {
      return el_update(it,he);
    }
    else
    {
      delete it;
      delete endit;
      he->liter      = 0;
      he->enditer    = 0;
      he->actElement = 0;
    }
    return 0;
  }

  template<class GridType>
  template<PartitionIteratorType pitype>
  inline int GrapeGridDisplay<GridType>::
  first_level (DUNE_ELEM * he, int level)
  {
    he->liter   = 0;
    he->enditer = 0;

    // for leaf level, level has the value -1
    if(level < 0) level = grid_.maxLevel();

    typedef typename GridType :: template Codim<0> ::
    template Partition<pitype> :: LevelIterator LevelIteratorType;

    // class copy constructor
    LevelIteratorType * it    = new LevelIteratorType( grid_.template lbegin<0,pitype> (level) );
    LevelIteratorType * endit = new LevelIteratorType( grid_.template lend<0,pitype>   (level) );

    if(it[0] == endit[0])
    {
      he->actElement = 0;
      delete it;
      delete endit;
      return 0;
    }

    he->liter   = (void *) it;
    he->enditer = (void *) endit;
    return el_update(it,he);
  }


  template<class GridType>
  template<PartitionIteratorType pitype>
  inline int GrapeGridDisplay<GridType>::
  next_level (DUNE_ELEM * he)
  {
    typedef typename GridType :: template Codim<0> ::
    template Partition<pitype> :: LevelIterator LevelIteratorType;

    LevelIteratorType * it    = ((LevelIteratorType *) he->liter);
    LevelIteratorType * endit = ((LevelIteratorType *) he->enditer);

    assert( it );
    assert( endit );
    if( ++it[0] != endit[0] )
    {
      return el_update(it,he);
    }
    else
    {
      delete it;
      delete endit;
      he->liter   = 0;
      he->enditer = 0;
      he->actElement = 0;
      he->hiter = 0;

      // clear all hierachic iterators
      while(!hierList_.empty())
      {
        HierarchicIteratorType * hit = hierList_.back();
        hierList_.pop_back();
        delete hit;
      }
      assert( hierList_.size () == 0 );
    }
    return 0;
  }


  template<class GridType>
  template<class EntityPointerType>
  inline int GrapeGridDisplay<GridType>::
  child_update(EntityPointerType * it, DUNE_ELEM * he)
  {
    typedef typename  GridType :: template Codim<0> :: Entity EntityType;

    EntityType & en = (*it[0]);

    HierarchicIteratorType * hit = (HierarchicIteratorType *) he->hiter;

    EntityType *newEn = (!hit) ? (&en) : (hit[0].operator -> ()) ;

    assert( newEn );

    // if entity is leaf, then no first child
    if( newEn->isLeaf() ) return 0;

    int childLevel = newEn->level() + 1;

    // store former pointer for removal later
    if(hit) hierList_.push_back( hit );

    // create HierarchicIterator with default constructor
    hit = new HierarchicIteratorType ( newEn->hbegin ( childLevel ) );

    assert( hit != 0 );
    if( hit[0] != newEn->hend( childLevel ) )
    {
      he->hiter = (void *) hit;
      return el_update( hit, he);
    }
    else
    {
      hierList_.pop_back();
      delete hit;
      return 0;
    }
  }

  template<class GridType>
  template<class EntityPointerType>
  inline int GrapeGridDisplay<GridType>::
  child_n_update(EntityPointerType *it, DUNE_ELEM * he)
  {
    typedef typename  GridType::Traits::template Codim<0>::Entity EntityType;

    EntityType &en = (*it[0]);

    int childLevel = en.level();
    HierarchicIteratorType * hit = (HierarchicIteratorType *) he->hiter;
    assert( hit );

    //HierarchicIteratorType ehit = hit[0]->hend(childLevel);
    HierarchicIteratorType ehit = en.hend(childLevel);
    if( ++hit[0] != ehit )
    {
      return el_update(hit,he);
    }

    hierList_.remove( hit );
    delete hit;
    he->hiter = 0;

    return 0;
  }


  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  first_child(DUNE_ELEM * he)
  {
    typedef typename GridType :: template Codim<0> ::
    EntityPointer EntityPointerType;
    return child_update( ((EntityPointerType *) he->liter) , he);
  }


  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  next_child(DUNE_ELEM * he)
  {
    typedef typename GridType :: template Codim<0> ::
    EntityPointer EntityPointerType;
    return child_n_update( ((EntityPointerType *) he->liter), he);
  }


  template<class GridType>
  inline void * GrapeGridDisplay<GridType>::
  copy_iterator (const void * i)
  {
    std::cerr << "ERROR: copt_iterator not implemented! file = " << __FILE__ << ", line = " << __LINE__ << "\n";
    abort () ;
    return 0 ;
  }

  // checkInside
  template<class GridType> template <class EntityType>
  inline int GrapeGridDisplay<GridType>::
  checkInside(EntityType &en, const double * c)
  {
    enum { dim = EntityType::dimension };

    for(int i=0; i<dim; i++) localVec_[i] = c[i];

    // see hmesh doc page 32, if point is inside, -1 has to be returned
    // otherwise local face , grrrr
    int isInside = (en.geometry().checkInside(localVec_) == true) ? -1 : 0;

    return isInside;
  }

  // check inside
  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  checkWhetherInside(DUNE_ELEM * he, const double * w)
  {
    typedef typename GridType::template Codim<0>::EntityPointer EntityPointerType;
    EntityPointerType * ep = (EntityPointerType *) he->actElement;
    assert( ep );
    return checkInside(*(ep[0]),w);
  }

  // world to local
  template<class GridType> template <class EntityType>
  inline void GrapeGridDisplay<GridType>::
  local_to_world(EntityType &en, const double * c, double * w)
  {
    enum { dim      = EntityType::dimension };
    enum { dimworld = EntityType::dimensionworld };

    for(int i=0; i<dim; i++) localVec_[i] = c[i];

    globalVec_ = en.geometry().global(localVec_);

    for(int i=0; i<dimworld; i++) w[i] = globalVec_[i];
    return;
  }


  template<class GridType>
  inline void GrapeGridDisplay<GridType>::
  local2world (DUNE_ELEM * he, const double * c, double * w)
  {
    typedef typename GridType::template Codim<0>::EntityPointer EntityPointerType;
    EntityPointerType * ep = (EntityPointerType *) he->actElement;
    assert( ep );
    local_to_world(*(ep[0]),c,w);
    return;
  }

  // world to local
  template<class GridType> template <class EntityType>
  inline int GrapeGridDisplay<GridType>::
  world_to_local(EntityType &en, const double * w, double * c)
  {
    enum { dim      = EntityType::dimension };
    enum { dimworld = EntityType::dimensionworld };

    for(int i=0; i<dimworld; i++) globalVec_[i] = w[i];

    localVec_ = en.geometry().local(globalVec_);

    for(int i=0; i<dim; ++i) c[i] = localVec_[i];

    return (en.geometry().checkInside(localVec_) == true) ? -1 : 0;
  }

  // world to local
  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  world2local(DUNE_ELEM * he, const double * w, double * c)
  {
    typedef typename GridType::template Codim<0>::EntityPointer EntityPointerType;
    EntityPointerType * ep = (EntityPointerType *) he->actElement;
    assert( ep );
    return world_to_local(*(ep[0]),w,c);
  }


  // world to local
  template<class GridType>
  template<PartitionIteratorType pitype>
  inline void GrapeGridDisplay<GridType>::
  selectIterators(DUNE_DAT * dune) const
  {
    if(dune->iteratorType == g_LeafIterator)
    {
      /* the button is pressed */
      dune->first_macro = &IterationMethods<pitype>::fst_leaf;
      dune->next_macro  = &IterationMethods<pitype>::nxt_leaf;

      // if pointer are 0, then nor evaluation is done
      dune->first_child = 0;
      dune->next_child  = 0;

      return ;
    }

    if(dune->iteratorType == g_LevelIterator)
    {
      dune->first_macro = &IterationMethods<pitype>::first_lev;
      dune->next_macro  = &IterationMethods<pitype>::next_lev;

      dune->first_child = 0;
      dune->next_child  = 0;

      return ;
    }

    if(dune->iteratorType == g_HierarchicIterator)
    {
      dune->first_macro = &IterationMethods<pitype>::first_mac;
      dune->next_macro  = &IterationMethods<pitype>::next_lev;

      dune->first_child = &IterationMethods<pitype>::fst_child;
      dune->next_child  = &IterationMethods<pitype>::nxt_child;

      return ;
    }

    // wrong iteratorType here
    assert(false);
    abort();
  }


  // setIterationsMethods
  template<class GridType>
  inline void GrapeGridDisplay<GridType>::
  setIterationMethods(DUNE_DAT * dune) const
  {
    switch(dune->partitionIteratorType)
    {
    case g_All_Partition :            selectIterators<All_Partition> (dune) ;
      return;
    case g_Interior_Partition :       selectIterators<Interior_Partition> (dune) ;
      return;
    case g_InteriorBorder_Partition : selectIterators<InteriorBorder_Partition> (dune) ;
      return;
    case g_Overlap_Partition :        selectIterators<Overlap_Partition> (dune) ;
      return;
    case g_OverlapFront_Partition :   selectIterators<OverlapFront_Partition> (dune) ;
      return;
    case g_Ghost_Partition :          selectIterators<Ghost_Partition> (dune) ;
      return;
    default : assert(false);
      abort();
      return ;
    }
  }

  // check inside
  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  check_inside(DUNE_ELEM * he, const double * w)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].checkWhetherInside(he,w);
  }
  // local to world
  template<class GridType>
  inline void GrapeGridDisplay<GridType>::
  ctow (DUNE_ELEM * he, const double * c, double * w)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    disp[0].local2world(he,c,w);
    return ;
  }

  // world to local
  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  wtoc(DUNE_ELEM * he, const double * w, double * c)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].world2local(he,w,c);
  }

  template<class GridType>
  inline void GrapeGridDisplay<GridType>::
  setIterationModus(DUNE_DAT * dat)
  {
    MyDisplayType * disp = (MyDisplayType *) dat->all->display;
    disp[0].setIterationMethods(dat);
  }

  template<class GridType>
  inline void GrapeGridDisplay<GridType>::display()
  {
    /* call handle mesh in g_hmesh.c */
    GrapeInterface<dim,dimworld>::handleMesh ( hmesh_ );
    return ;
  }

  template<class GridType>
  inline void * GrapeGridDisplay<GridType>::getHmesh()
  {
    if(!hmesh_) hmesh_ = setupHmesh();
    return (void *) hmesh_;
  }

  template<class GridType>
  inline void GrapeGridDisplay<GridType>::
  addMyMeshToTimeScene(void * timescene, double time, int proc)
  {
    GrapeInterface<dim,dimworld>::addHmeshToTimeScene(timescene,time,this->getHmesh(),proc);
  }

  template<class GridType>
  inline const GridType & GrapeGridDisplay<GridType>::getGrid() const
  {
    return grid_;
  }

  template<class GridType>
  inline void * GrapeGridDisplay<GridType>::setupHmesh()
  {
    // default set all coordinates to zero
    for(int i=0; i<MAX_EL_DOF; i++)
      for(int j=0; j<3; j++)
      {
        hel_.vpointer[i][j] = 0.0;
      }

    int maxlevel = grid_.maxLevel();

    int noe = leafset_.size(0);
    int nov = leafset_.size(dim);

    hel_.display = (void *) this;
    hel_.liter   = 0;
    hel_.enditer = 0;

    hel_.hiter    = 0;

    hel_.actElement = NULL;

    DUNE_DAT * dune = &dune_;

    dune->copy         = 0; // no copy at the moment
    dune->wtoc         = wtoc;
    dune->ctow         = ctow;
    dune->check_inside = check_inside;

    // set method to select iterators
    dune->setIterationModus = &setIterationModus;

    dune->all          = &hel_;
    dune->partition    = myRank_;

    dune->iteratorType          = g_LeafIterator;
    dune->partitionIteratorType = g_All_Partition;

    setIterationMethods(dune);

    /* return hmesh with no data */
    return GrapeInterface<dim,dimworld>::hmesh(NULL,noe,nov,maxlevel,NULL,dune);
  }

} // end namespace Dune
