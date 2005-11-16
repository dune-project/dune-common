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
    , myIt_(0), myEndIt_ (0) , myLeafIt_(0) , myLeafEndIt_ (0) ,
    hmesh_ (0)
  {
    GrapeInterface<dim,dimworld>::init();
    if(!hmesh_) hmesh_ = setupHmesh();
  }

  template<class GridType>
  inline GrapeGridDisplay<GridType>::
  GrapeGridDisplay(const GridType &grid ) :
    grid_(grid)
    , leafset_(grid.leafIndexSet())
    , lid_(grid.localIdSet())
    , myRank_(-1) ,
    myIt_(0), myEndIt_ (0) , myLeafIt_(0) , myLeafEndIt_ (0) ,
    hmesh_ (0)
  {
    GrapeInterface<dim,dimworld>::init();
    if(!hmesh_) hmesh_ = setupHmesh();
  }


  template<class GridType>
  inline GrapeGridDisplay<GridType>::
  ~GrapeGridDisplay()
  {
    if(myIt_) delete myIt_;
    if(myEndIt_) delete myEndIt_;

    if(myLeafIt_) delete myLeafIt_;
    if(myLeafEndIt_) delete myLeafEndIt_;
  }

  //****************************************************************
  //
  // --GridDisplay, Some Subroutines needed in display
  //
  //****************************************************************

  /** hmesh functionen **/

  template<class GridType>
  template <class GridIteratorType>
  inline int GrapeGridDisplay<GridType>::
  el_update (GridIteratorType *it, DUNE_ELEM * he)
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
        IntersectionIterator endnit = en.iend();
        IntersectionIterator nit    = en.ibegin();

        int facecount = 0;

        // value < zero otherwise first test fails
        int lastElNum = -1;

        // check all faces for boundary or not
        while ( nit != endnit )
        {
          assert( facecount >= 0 );
          assert( facecount < MAX_EL_FACE );

          int num = nit.numberInSelf();
          assert( num >= 0 );
          assert( num < MAX_EL_FACE );

          if(num != lastElNum)
          {
            he->bnd[num] = ( nit.boundary() ) ? -1 : 0;
            facecount++ ;
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
  inline int GrapeGridDisplay<GridType>::
  first_leaf (DUNE_ELEM * he)
  {
    if(myLeafIt_) delete myLeafIt_;
    if(myLeafEndIt_) delete myLeafEndIt_;

    // myIt ist Zeiger auf LevelIteratorType, definiert innerhalb der Klasse
    // rufe default CopyConstructor auf
    int levelOI = he->level_of_interest;
    if(levelOI < 0) levelOI = grid_.maxLevel();

    //myLeafIt_    = new LeafIteratorType ( grid_.template leafbegin<0, All_Partition> (levelOI) );
    //myLeafEndIt_ = new LeafIteratorType ( grid_.template leafend  <0, All_Partition> (levelOI) );
    myLeafIt_    = new LeafIteratorType ( grid_.template leafbegin<0, All_Partition> () );
    myLeafEndIt_ = new LeafIteratorType ( grid_.template leafend  <0, All_Partition> () );

    if(myLeafIt_[0] == myLeafEndIt_[0])
    {
      he->actElement = 0;
      return 0;
    }

    he->liter = (void *) myLeafIt_;
    return el_update(myLeafIt_,he);
  }


  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  next_leaf (DUNE_ELEM * he)
  {
    LeafIteratorType * it = (LeafIteratorType *) he->liter;
    assert (it );
    if( ++it[0] != myLeafEndIt_[0] )
    {
      return el_update(it,he);
    }
    return 0;
  }


  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  first_macro (DUNE_ELEM * he)
  {
    if(myIt_) delete myIt_;
    if(myEndIt_) delete myEndIt_;

    int levelOI = he->level_of_interest;
    if(levelOI < 0) levelOI = grid_.maxLevel();

    // myIt ist Zeiger auf LevelIteratorType, definiert innerhalb der Klasse
    // rufe default CopyConstructor auf

    myIt_    = new LevelIteratorType(grid_.template lbegin<0> (levelOI) );
    myEndIt_ = new LevelIteratorType(grid_.template lend<0>   (levelOI) );

    // funktioniert nur, wenn man im Macro
    // HM_ALL_TEST_IF_PROCEED !tip_element->has_children entfernt

    if(myIt_[0] == myEndIt_[0])
    {
      he->actElement = 0;
      return 0;
    }

    he->liter = (void *) myIt_;
    return el_update(myIt_,he);
  }


  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  next_macro (DUNE_ELEM * he)
  {
    LevelIteratorType *it = ((LevelIteratorType *) he->liter);
    assert ( it );
    if( ++it[0] != myEndIt_[0] )
    {
      return el_update(it,he);
    }
    return 0;
  }

  template<class GridType>
  template<class GridIteratorType>
  inline int GrapeGridDisplay<GridType>::
  child_update(GridIteratorType *it, DUNE_ELEM * he)
  {
    typedef typename  GridIteratorType::Entity EntityType;
    typedef typename  EntityType::HierarchicIterator HIERit;

    EntityType &en = (*it[0]);

    HIERit     *hit    = 0;
    EntityType *newEn  = 0;

    if(he->hiter == 0)
    {
      newEn = &en;
    }
    else
    {
      newEn = &(*(((HIERit *)he->hiter)[0]));
    }

    int childLevel = newEn->level() + 1;

    // Rufe default CopyConstructor auf
    hit = new HIERit (newEn->hbegin ( childLevel ) );
    assert( hit != 0 );
    if( hit[0] != newEn->hend( childLevel ) )
    {
      he->hiter = (void *) hit;
      return el_update( hit, he);
    }
    else
    {
      if(hit) delete hit;
      return 0;
    }
  }

  template<class GridType>
  template<class GridIteratorType>
  inline int GrapeGridDisplay<GridType>::
  child_n_update(GridIteratorType *it, DUNE_ELEM * he)
  {
    typedef typename  GridType::Traits::template Codim<0>::Entity EntityType;
    typedef typename  EntityType::HierarchicIterator HIERit;

    EntityType &en = (*it[0]);

    int childLevel = en.level();
    HIERit *hit = (HIERit *) he->hiter;
    HIERit ehit = hit[0]->hend(childLevel);

    if( ++hit[0] != ehit )
    {
      return el_update(hit,he);
    }

    if(hit) delete hit;
    he->hiter = 0;
    return 0;
  }


  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  first_child(DUNE_ELEM * he)
  {
    return child_update( ((LevelIteratorType *) he->liter), he);
  }


  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  next_child(DUNE_ELEM * he)
  {
    return child_n_update( ((LevelIteratorType *) he->liter), he);
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

    for(int i=0; i<dim; i++)
    {
      localVec_[i] = c[i];
    }

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
    void *iter = he->actElement;
    if(iter == he->liter)
    {
      if(he->isLeafIterator)
      {
        typedef typename GridType::template Codim<0>::LeafIterator LeafIt;
        LeafIt *it = (LeafIt *) he->liter;
        return checkInside(*(it[0]),w);
      }
      else
      {
        typedef typename GridType::Traits::template Codim<0>::LevelIterator LevIt;
        LevIt *it = (LevIt *) he->liter;
        return checkInside(*(it[0]),w);
      }
    }
    else if(iter == he->hiter)
    {
      typedef typename GridType::Traits::template Codim<0>::Entity::HierarchicIterator HierIt;
      HierIt *it = (HierIt *) he->hiter;
      return checkInside(*(it[0]),w);
    }
    else
    {
      std::cerr << "ERROR: No Iterator in checkInside! file = " << __FILE__ << ", line = " << __LINE__ << "\n";
      abort();
      return 0;
    }
  }

  // world to local
  template<class GridType> template <class EntityType>
  inline void GrapeGridDisplay<GridType>::
  local_to_world(EntityType &en, const double * c, double * w)
  {
    enum { dim      = EntityType::dimension };
    enum { dimworld = EntityType::dimensionworld };

    for(int i=0; i<dim; i++)
      localVec_[i] = c[i];

    globalVec_ = en.geometry().global(localVec_);

    for(int i=0; i<dimworld; i++)
      w[i] = globalVec_[i];

    return;
  }


  template<class GridType>
  inline void GrapeGridDisplay<GridType>::
  local2world (DUNE_ELEM * he, const double * c, double * w)
  {
    void *iter = he->actElement;
    if(iter == he->liter)
    {
      if(he->isLeafIterator)
      {
        typedef typename GridType::template Codim<0>::LeafIterator LeafIt;
        LeafIt *it = (LeafIt *) he->liter;
        local_to_world(*(it[0]),c,w);
        return;
      }
      else
      {
        typedef typename GridType::Traits::template Codim<0>::LevelIterator LevIt;
        LevIt *it = (LevIt *) he->liter;
        local_to_world(*(it[0]),c,w);
        return;
      }
    }
    else if(iter == he->hiter)
    {
      typedef typename GridType::Traits::template Codim<0>::Entity::HierarchicIterator HierIt;
      HierIt *it = (HierIt *) he->hiter;
      local_to_world(*(it[0]),c,w);
      return;
    }
    else
    {
      std::cerr << "ERROR: No Iterator in ctow! file = " << __FILE__ << ", line = " << __LINE__ << "\n";
      abort();
      return;
    }
  }

  // world to local
  template<class GridType> template <class EntityType>
  inline int GrapeGridDisplay<GridType>::
  world_to_local(EntityType &en, const double * w, double * c)
  {
    enum { dim      = EntityType::dimension };
    enum { dimworld = EntityType::dimensionworld };

    for(int i=0; i<dimworld; i++)
      globalVec_[i] = w[i];

    localVec_ = en.geometry().local(globalVec_);

    for(int i=0; i<dim; i++) c[i] = localVec_[i];

    return (en.geometry().checkInside(localVec_) == true) ? -1 : 0;
  }
  // world to local
  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  world2local(DUNE_ELEM * he, const double * w, double * c)
  {
    void *iter = he->actElement;
    if(iter == he->liter)
    {
      if(he->isLeafIterator)
      {
        typedef typename GridType::template Codim<0>::LeafIterator LeafIt;
        LeafIt *it = (LeafIt *) he->liter;
        return world_to_local(*(it[0]),w,c);
      }
      else
      {
        typedef typename GridType::Traits::template Codim<0>::LevelIterator LevIt;
        LevIt *it = (LevIt *) he->liter;
        return world_to_local(*(it[0]),w,c);
      }
    }
    else if(iter == he->hiter)
    {
      typedef typename GridType::Traits::template Codim<0>::Entity::HierarchicIterator HierIt;
      HierIt *it = (HierIt *) he->hiter;
      return world_to_local(*(it[0]),w,c);
    }
    else
    {
      std::cerr << "ERROR: No Iterator in wtoc! file = " << __FILE__ << ", line = " << __LINE__ << "\n";
      abort();
      return 0;
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
  inline int GrapeGridDisplay<GridType>::
  first_mac (DUNE_ELEM * he)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].first_macro(he);
  }


  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  next_mac (DUNE_ELEM * he)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].next_macro(he);
  }

  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  fst_leaf (DUNE_ELEM * he)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].first_leaf(he);
  }


  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  nxt_leaf (DUNE_ELEM * he)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].next_leaf(he);
  }

  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  fst_child (DUNE_ELEM * he)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].first_child(he);
  }


  template<class GridType>
  inline int GrapeGridDisplay<GridType>::
  nxt_child (DUNE_ELEM * he)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].next_child(he);
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
    int noe = 0, nov = 0;
    int maxlevel = 0;

    // default set all coordinates to zero
    for(int i=0; i<MAX_EL_DOF; i++)
      for(int j=0; j<3; j++)
      {
        hel_.vpointer[i][j] = 0.0;
      }

    maxlevel = grid_.maxLevel();

    for(unsigned int i=0; i<leafset_.geomTypes(0).size(); i++)
    {
      noe += leafset_.size(0,leafset_.geomTypes(0)[i]);
      nov += leafset_.size(dim,leafset_.geomTypes(dim)[i]);
    }

    hel_.display = (void *) this;
    hel_.liter = NULL;
    hel_.hiter = NULL;
    hel_.actElement = NULL;

    /* return hmesh with no data */
    return GrapeInterface<dim,dimworld>::hmesh(fst_leaf,nxt_leaf,first_mac,next_mac,fst_child,nxt_child,
                                               NULL,check_inside,wtoc,ctow,NULL,noe,nov,maxlevel,myRank_,&hel_,NULL);

  }

} // end namespace Dune
