// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BSGRID_CC__
#define __DUNE_BSGRID_CC__

namespace Dune {

  //! singelton holding reference element
  static BSGridElement<3,3> refelem_3d(true);

  template <int dim, int dimworld>
  inline BSGrid<dim,dimworld>::BSGrid(const char* macroTriangFilename)
    : mygrid_ (0) , maxlevel_(0)
  {
    mygrid_ = new BSSPACE GitterBasisImpl (macroTriangFilename);
    assert(mygrid_ != 0);
    mygrid_->printsize();

    calcExtras();
  }

  template <int dim, int dimworld>
  inline BSGrid<dim,dimworld>::BSGrid() : mygrid_ (0) , maxlevel_(0)
  {
    for(int l=0; l<MAXL; l++)
      for(int i=0; i<dim+1; i++) size_[l][i] = -1;

    for(int i=0; i<dim+1; i++) globalSize_[i] = -1;
  }

  template <int dim, int dimworld>
  inline BSGrid<dim,dimworld>::~BSGrid()
  {
    if(mygrid_) delete mygrid_;
  }

  template <int dim, int dimworld>
  inline int BSGrid<dim,dimworld>::size(int level, int codim) const
  {
    return const_cast<BSGrid<dim,dimworld> *> (this)->size(level,codim);
  }

  template <int dim, int dimworld>
  inline int BSGrid<dim,dimworld>::size(int level, int codim)
  {
    assert(dim == 3);

    // todo level einbauen
    if(size_[level][codim] == -1)
    {
      switch (codim)
      {
      case 0 :
      {
        typename BSSPACE BSLevelIterator<0>::IteratorType
        w (mygrid_->container(),level);
        size_[level][0] = w.size();
        break;
      }
      case 1 :
      {
        typename BSSPACE BSLevelIterator<1>::IteratorType
        w (mygrid_->container(),level);
        size_[level][1] = w.size();
        break;
      }
      case 2 :
      {
        typename BSSPACE BSLevelIterator<2>::IteratorType
        w (mygrid_->container(),level);
        size_[level][2] = w.size();
        break;
      }
      case 3 :
      {
        typename BSSPACE BSLevelIterator<3>::IteratorType
        w (mygrid_->container(),level) ;
        size_[level][3] = w.size();
        break;
      }
      default :
      {
        std::cerr << "BSGrid::size: Wrong codim choosen! \n";
        assert(false);
        abort();
      }

      }
    }
    return size_[level][codim];
  }

  template <int dim, int dimworld>
  inline void BSGrid<dim,dimworld>::calcMaxlevel()
  {
    maxlevel_ = 0;
    assert(mygrid_ != 0);
    typename BSSPACE BSLeafIteratorMaxLevel w (*mygrid_) ;
    for (w->first () ; ! w->done () ; w->next ())
    {
      if(w->item().level() > maxlevel_ ) maxlevel_ = w->item().level();
    }
  }

  template <int dim, int dimworld>
  inline void BSGrid<dim,dimworld>::calcExtras()
  {
    for(int l=0; l<MAXL; l++)
      for(int i=0; i<dim+1; i++) size_[l][i] = -1;

    for(int i=0; i<dim+1; i++) globalSize_[i] = -1;

    //BSGridLevelIterator<0,dim,dimworld,All_Partition> it    = this->template lbegin<0> (0);
    //BSGridLevelIterator<0,dim,dimworld,All_Partition> endit = this->template lend  <0> (0);
    BSGridLeafIterator it    = leafbegin (0);
    BSGridLeafIterator endit = leafend   (0);

    for(; it != endit; ++it)
    {
      BSGridHierarchicIterator<dim,dimworld> hierend = it->hend   (maxlevel());

      for(BSGridHierarchicIterator<dim,dimworld> hierit  = (*it).hbegin (maxlevel()) ;
          hierit != hierend; ++hierit )
      {
        if((*hierit).global_index() > globalSize_[0])
          globalSize_[0] = (*hierit).global_index();
      }
    }
    globalSize_[0]++;
  }
  template <int dim, int dimworld>
  inline int BSGrid<dim,dimworld>::global_size(int codim)
  {
    assert(codim == 0);
    return globalSize_[codim];
  }

  template <int dim, int dimworld>
  inline int BSGrid<dim,dimworld>::maxlevel() const
  {
    return maxlevel_;
  }
  template <int dim, int dimworld>
  inline BSSPACE GitterBasisImpl *BSGrid<dim,dimworld>::mygrid() {
    return mygrid_;
  }

  // lbegin methods
  template <int dim, int dimworld>
  template <int codim>
  inline BSGridLevelIterator<codim,dim,dimworld,All_Partition> BSGrid<dim,dimworld>::lbegin(int level) {
    return BSGridLevelIterator<codim,dim,dimworld,All_Partition>(*this,level);
  }

  template <int dim, int dimworld>
  template <int codim>
  inline BSGridLevelIterator<codim,dim,dimworld,All_Partition> BSGrid<dim,dimworld>::lend(int level) {
    return BSGridLevelIterator<codim,dim,dimworld,All_Partition>(*this,level,true);
  }

  // leaf methods
  template <int dim, int dimworld>
  inline typename BSGrid<dim,dimworld>::LeafIterator BSGrid<dim,dimworld>::leafbegin(int level)
  {
    return BSGridLeafIterator(*this,level);
  }
  template <int dim, int dimworld>
  inline typename BSGrid<dim,dimworld>::LeafIterator BSGrid<dim,dimworld>::leafend(int level)
  {
    return BSGridLeafIterator(*this,level,true);
  }

  // global refine
  template <int dim, int dimworld>
  inline bool BSGrid<dim,dimworld>::globalRefine(int anzahl)
  {
    for (; anzahl>0; anzahl--)
    {
      {
        typename BSSPACE BSLeafIteratorMaxLevel w (*mygrid_) ;
        for (w->first () ; ! w->done () ; w->next ())
          w->item ().tagForGlobalRefinement ();
      }
      mygrid_->adapt ();
      maxlevel_++;
    }
    calcExtras();

    return true;
  }

  // adapt grid
  template <int dim, int dimworld>
  inline bool BSGrid<dim,dimworld>::preAdapt()
  {
    return false;
  }

  // adapt grid
  template <int dim, int dimworld>
  inline bool BSGrid<dim,dimworld>::adapt()
  {
    return false;
  }

  // adapt grid
  template <int dim, int dimworld>
  inline void BSGrid<dim,dimworld>::postAdapt()
  {}

  // writeGrid and readGrid
  template <int dim, int dimworld>
  template <FileFormatType ftype>
  inline bool BSGrid<dim,dimworld>::
  writeGrid( const char * filename, bs_ctype time )
  {
    (*mygrid_).duneBackup(filename);

    // write time and maxlevel
    {
      char *extraName = new char[strlen(filename)+20];
      if(!extraName)
      {
        std::cerr << "BSGrid::readGrid: couldn't allocate extraName! \n";
        abort();
      }
      sprintf(extraName,"%s.extra",filename);
      std::ofstream out (extraName);
      if(out)
      {
        out.precision (16);
        out << time << " ";
        out << maxlevel_ << " ";
        out.close();
      }
      else
      {
        std::cerr << "BSGrid::readGrid: couldn't open <" << extraName << ">! \n";
      }
      delete [] extraName;
    }
    return true;
  }

  // writeGrid and readGrid
  template <int dim, int dimworld>
  template <FileFormatType ftype>
  inline bool BSGrid<dim,dimworld>::
  readGrid( const char * filename, bs_ctype & time )
  {
    {
      char *macroName = new char[strlen(filename)+20];
      if(!macroName)
      {
        std::cerr << "BSGrid::readGrid: couldn't allocate macroName! \n";
        abort();
      }
      sprintf(macroName,"%s.macro",filename);

      mygrid_ = new BSSPACE GitterBasisImpl (macroName);

      delete [] macroName;
    }

    assert(mygrid_ != 0);
    (*mygrid_).duneRestore(filename);

    {
      char *extraName = new char[strlen(filename)+20];
      if(!extraName)
      {
        std::cerr << "BSGrid::readGrid: couldn't allocate extraName! \n";
        abort();
      }
      sprintf(extraName,"%s.extra",filename);
      std::ifstream in (extraName);
      if(in)
      {
        in.precision (16);
        in  >> time;
        in  >> maxlevel_;
        in.close();
      }
      else
      {
        std::cerr << "BSGrid::readGrid: couldn't open <" << extraName << ">! \n";
      }
      delete [] extraName;
    }

    calcExtras();

    // set max index of grid
    for(int i=0; i<dim+1; i++)
      mygrid_->indexManager(i).setMaxIndex( globalSize_[i] );

    return true;
  }

  // return Grid type
  template <int dim, int dimworld>
  inline GridIdentifier BSGrid<dim,dimworld>::type ()
  {
    return BSGrid_Id;
  }

  /*************************************************************************
  #       ######  #    #  ######  #          #     #####  ######  #####
  #       #       #    #  #       #          #       #    #       #    #
  #       #####   #    #  #####   #          #       #    #####   #    #
  #       #       #    #  #       #          #       #    #       #####
  #       #        #  #   #       #          #       #    #       #   #
  ######  ######    ##    ######  ######     #       #    ######  #    #
  *************************************************************************/
  //--LevelIterator
  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline BSGridLevelIterator<codim,dim,dimworld,pitype> ::
  BSGridLevelIterator(BSGrid<dim,dimworld> &grid, int level,bool end)
    : grid_(grid)
      , index_(-1)
      , level_(level)
      , iter_(grid_.mygrid()->container(), level )
  {
    if(!end)
    {
      iter_.first();
      index_=0;
      BSGridEntity<0,dim,dimworld> * obj =
        new BSGridEntity<codim,dim,dimworld> (grid_,iter_.item(),index_);
      objEntity_.store ( obj );
    }
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline BSGridLevelIterator<codim,dim,dimworld,pitype> ::
  BSGridLevelIterator(BSGrid<dim,dimworld> &grid, BSSPACE HElementType &item)
    : grid_(grid)
      , index_(-1)
      , level_(item.level())
      , iter_(grid_.mygrid()->container(), level_ )
  {
    index_=0;
    BSGridEntity<codim,dim,dimworld> * obj = new BSGridEntity<codim,dim,dimworld> (grid_,item,index_);
    // objEntity deletes entity if no refCount is left
    objEntity_.store ( obj );
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline BSGridLevelIterator<codim,dim,dimworld,pitype> &
  BSGridLevelIterator<codim,dim,dimworld,pitype> :: operator ++()
  {
    assert(index_ >= 0);

    iter_.next();
    if (iter_.done())
    {
      index_ = -1;
      return *this;
    }

    objEntity_->setelement(iter_.item(),index_);
    return *this;
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline bool BSGridLevelIterator<codim,dim,dimworld,pitype>::
  operator== (const BSGridLevelIterator<codim,dim,dimworld,pitype>& i) const
  {
    return (index_ == i.index_);
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline bool BSGridLevelIterator<codim,dim,dimworld,pitype>::
  operator!= (const BSGridLevelIterator<codim,dim,dimworld,pitype>& i) const
  {
    return (index_ != i.index_);
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline BSGridEntity<codim,dim,dimworld>&
  BSGridLevelIterator<codim,dim,dimworld,pitype>::operator*()
  {
    return (*objEntity_);
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline BSGridEntity<codim,dim,dimworld>*
  BSGridLevelIterator<codim,dim,dimworld,pitype>::operator->()
  {
    return objEntity_.operator -> ();
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline int BSGridLevelIterator<codim,dim,dimworld,pitype>::level ()
  {
    return level_;
  }

  //*******************************************************************
  //
  //  LEAFITERATOR
  //
  //--LeafIterator
  //*******************************************************************
  template<int dim, int dimworld>
  inline BSGrid<dim,dimworld>::BSGridLeafIterator ::
  BSGridLeafIterator(BSGrid<dim,dimworld> &grid, int level,bool end)
    : index_(-1)
      , level_(level)
      , iter_(grid.mygrid()->container(), level )
  {
    if(!end)
    {
      iter_.first();
      index_=0;
      BSGridEntity<0,dim,dimworld> * obj =
        new BSGridEntity<codim,dim,dimworld> (grid,iter_.item(),index_);
      objEntity_.store ( obj );
    }
  }

  template <int dim, int dimworld>
  inline typename BSGrid<dim,dimworld>::BSGridLeafIterator &
  BSGrid<dim,dimworld>::BSGridLeafIterator :: operator ++()
  {
    assert(index_  >= 0);

    iter_.next();
    index_++;

    if(iter_.done())
    {
      index_ = -1;
      return *this;
    }

    objEntity_->setelement(iter_.item(),index_);
    return *this;
  }

  template <int dim, int dimworld>
  inline bool BSGrid<dim,dimworld>::BSGridLeafIterator ::
  operator== (const BSGridLeafIteratorType & i) const
  {
    return (index_ == i.index_);
  }

  template <int dim, int dimworld>
  inline bool BSGrid<dim,dimworld>::BSGridLeafIterator ::
  operator!= (const BSGridLeafIteratorType & i) const
  {
    return (index_ != i.index_);
  }

  template <int dim, int dimworld>
  inline BSGridEntity<0,dim,dimworld>&
  BSGrid<dim,dimworld>::BSGridLeafIterator :: operator*()
  {
    return (*objEntity_);
  }

  template <int dim, int dimworld>
  inline BSGridEntity<0,dim,dimworld>*
  BSGrid<dim,dimworld>::BSGridLeafIterator :: operator->()
  {
    return objEntity_.operator -> ();
  }

  template <int dim, int dimworld>
  inline int BSGrid<dim,dimworld>::BSGridLeafIterator :: level()
  {
    return level_;
  }


  /************************************************************************************
  #     #
  #     #     #    ######  #####      #     #####  ######  #####
  #     #     #    #       #    #     #       #    #       #    #
  #######     #    #####   #    #     #       #    #####   #    #
  #     #     #    #       #####      #       #    #       #####
  #     #     #    #       #   #      #       #    #       #   #
  #     #     #    ######  #    #     #       #    ######  #    #
  ************************************************************************************/
  // --HierarchicIterator
  template<int dim, int dimworld>
  inline BSGridHierarchicIterator<dim,dimworld> ::
  BSGridHierarchicIterator(BSGrid<dim,dimworld> &grid ,
                           BSSPACE HElementType & elem, int maxlevel ,bool end)
    : grid_(grid), elem_(elem) , maxlevel_(maxlevel), item_(0)
  {
    if (!end)
    {
      item_ = elem_.down();
      if(item_)
      {
        if(item_->level() <= maxlevel_)
        {
          BSGridEntity<0,dim,dimworld> * obj = new BSGridEntity<0,dim,dimworld>(grid_,*item_ ,0);
          // objEntity deletes entity pointer when no refCount is left
          objEntity_.store ( obj );
        }
      }
    }
  }

  template< int dim, int dimworld>
  inline BSSPACE HElementType *
  BSGridHierarchicIterator<dim,dimworld>::goNextElement( BSSPACE HElementType * oldelem )
  {
    // strategy is:
    // - go down as far as possible and then over all children
    // - then go to father and next and down again

    BSSPACE HElementType * nextelem = oldelem->down();
    if(nextelem)
    {
      if(nextelem->level() <= maxlevel_)
        return nextelem;
    }

    nextelem = oldelem->next();
    if(nextelem)
    {
      if(nextelem->level() <= maxlevel_)
        return nextelem;
    }

    nextelem = oldelem->up();
    if(nextelem == &elem_) return 0;

    while( !nextelem->next() )
    {
      nextelem = nextelem->up();
      if(nextelem == &elem_) return 0;
    }

    if(nextelem) nextelem = nextelem->next();

    return nextelem;
  }

  template<int dim, int dimworld>
  inline BSGridHierarchicIterator<dim,dimworld> &
  BSGridHierarchicIterator<dim,dimworld> :: operator ++()
  {
    assert(item_   != 0);

    item_ = goNextElement( item_ );
    if(!item_)
    {
      return *this;
    }

    objEntity_->setelement(*item_,0);
    return *this;
  }

  template< int dim, int dimworld>
  inline bool BSGridHierarchicIterator<dim,dimworld>::
  operator== (const BSGridHierarchicIterator<dim,dimworld>& i) const
  {
    return item_==i.item_;
  }

  template< int dim, int dimworld>
  inline bool BSGridHierarchicIterator<dim,dimworld>::
  operator!= (const BSGridHierarchicIterator<dim,dimworld>& i) const
  {
    return item_!=i.item_;
  }

  template< int dim, int dimworld>
  inline BSGridEntity<0,dim,dimworld>&
  BSGridHierarchicIterator<dim,dimworld>::operator*()
  {
    return (*objEntity_);
  }

  template< int dim, int dimworld>
  inline BSGridEntity<0,dim,dimworld>*
  BSGridHierarchicIterator<dim,dimworld>::operator->()
  {
    return objEntity_.operator -> ();
  }
  /************************************************************************************
  ###
  #     #    #   #####  ######  #####    ####   ######   ####      #     #####
  #     ##   #     #    #       #    #  #       #       #    #     #       #
  #     # #  #     #    #####   #    #   ####   #####   #          #       #
  #     #  # #     #    #       #####        #  #       #          #       #
  #     #   ##     #    #       #   #   #    #  #       #    #     #       #
  ###    #    #     #    ######  #    #   ####   ######   ####      #       #
  ************************************************************************************/
  // --IntersectionIterator
  template<int dim, int dimworld>
  inline BSGridIntersectionIterator<dim,dimworld> ::
  BSGridIntersectionIterator(BSGrid<dim,dimworld> &grid,
                             BSSPACE HElementType *el, int level,bool end) :
    entity_( grid ), item_(el), index_(0) , needSetup_ (true), needNormal_(true)
    , interSelfGlobal_ (false)
  {
    if (end) index_ = 4;
  }
  template<int dim, int dimworld>
  inline void BSGridIntersectionIterator<dim,dimworld> :: first (BSSPACE HElementType & elem)
  {
    item_  = &elem;
    index_ = 0;
    needSetup_ = true;
    needNormal_= true;
  }

  template<int dim, int dimworld>
  inline void BSGridIntersectionIterator<dim,dimworld> :: done ()
  {
    item_  = 0;
    index_ = 4;
  }

  template<int dim, int dimworld>
  inline BSGridIntersectionIterator<dim,dimworld> :: ~BSGridIntersectionIterator() {}

  template<int dim, int dimworld>
  inline BSGridIntersectionIterator<dim,dimworld> &
  BSGridIntersectionIterator<dim,dimworld> :: operator ++()
  {
    index_++;
    needSetup_  = true;
    needNormal_ = true;
    return *this;
  }

  template<int dim, int dimworld>
  inline bool BSGridIntersectionIterator<dim,dimworld> ::
  operator== (const BSGridIntersectionIterator<dim,dimworld>& i) const
  {
    return index_==i.index_;
  }

  template<int dim, int dimworld>
  inline bool BSGridIntersectionIterator<dim,dimworld> ::
  operator!= (const BSGridIntersectionIterator<dim,dimworld>& i) const
  {
    return index_!=i.index_;
  }

  // set new neighbor
  template<int dim, int dimworld>
  inline void BSGridIntersectionIterator<dim,dimworld> ::
  setNeighbor ()
  {
    assert(this->neighbor());

    typename BSSPACE GEOElementType *neigh =
      (static_cast<BSSPACE GEOElementType *> (item_))->myneighbour(index_).first;

    entity_.setelement(*neigh, index_);
    needSetup_ = false;
  }

  template<int dim, int dimworld>
  inline BSGridEntity<0,dim,dimworld>&
  BSGridIntersectionIterator<dim,dimworld> :: operator*()
  {
    if(needSetup_) setNeighbor();
    return entity_;
  }

  template<int dim, int dimworld>
  inline BSGridEntity<0,dim,dimworld>*
  BSGridIntersectionIterator<dim,dimworld> :: operator->()
  {
    if(needSetup_) setNeighbor();
    return &entity_;
  }

  template<int dim, int dimworld>
  inline bool BSGridIntersectionIterator<dim,dimworld> :: boundary ()
  {
    assert(0<=index_ && index_<4);
    assert(item_);
    return (static_cast<BSSPACE GEOElementType *>
            (item_)->myneighbour(index_)).first->isboundary();
  }

  template<int dim, int dimworld>
  inline bool BSGridIntersectionIterator<dim,dimworld>::neighbor ()
  {
    return !(this->boundary());
  }

  template<int dim, int dimworld>
  inline int BSGridIntersectionIterator<dim,dimworld>::number_in_self ()
  {
    return index_;
  }

  template<int dim, int dimworld>
  inline int BSGridIntersectionIterator<dim,dimworld>::number_in_neighbor ()
  {
    return (static_cast<BSSPACE GEOElementType *> (item_)->myneighbour(index_).second);
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,bs_ctype>&
  BSGridIntersectionIterator<dim,dimworld>::
  outer_normal(Vec<dim-1,bs_ctype>& local)
  {
    return this->outer_normal();
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,bs_ctype>&
  BSGridIntersectionIterator<dim,dimworld>::outer_normal()
  {
    if(needNormal_)
    {
      item_->outerNormal(index_,outerNormal_);
      needNormal_ = false;
    }
    return outerNormal_;
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,bs_ctype>&
  BSGridIntersectionIterator<dim,dimworld>::
  unit_outer_normal(Vec<dim-1,bs_ctype>& local)
  {
    return this->unit_outer_normal();
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,bs_ctype>&
  BSGridIntersectionIterator<dim,dimworld>::unit_outer_normal()
  {
    unitOuterNormal_  = this->outer_normal();
    unitOuterNormal_ /= unitOuterNormal_.norm2();
    return unitOuterNormal_;
  }

  template< int dim, int dimworld>
  inline BSGridElement<dim-1,dimworld>&
  BSGridIntersectionIterator<dim,dimworld>::intersection_self_global ()
  {
    const BSSPACE Gitter::Geometric::hface3_GEO & face = *(static_cast<BSSPACE GEOElementType &>(*item_).myhface3(index_));
    bool init = interSelfGlobal_.builtGeom(face);

    return interSelfGlobal_;
  }

  /************************************************************************************
  ######  #    #   #####     #     #####   #   #
  #       ##   #     #       #       #      # #
  #####   # #  #     #       #       #       #
  #       #  # #     #       #       #       #
  #       #   ##     #       #       #       #
  ######  #    #     #       #       #       #
  ************************************************************************************/
  // --0Entity
  template<int dim, int dimworld>
  inline BSGridEntity<0,dim,dimworld> :: BSGridEntity(BSGrid<dim,dimworld> &grid,
                                                      BSSPACE HElementType & element,int index) :
    grid_(grid), item_(&element), builtgeometry_(false), geo_(false), index_(index)
  {}

  template<int dim, int dimworld>
  inline void
  BSGridEntity<0,dim,dimworld> :: setelement(BSSPACE HElementType & element,int index)
  {
    item_=&element;
    builtgeometry_=false;
    index_=index;
  }

  template<int dim, int dimworld>
  inline int
  BSGridEntity<0,dim,dimworld> :: level()
  {
    return item_->level();
  }

  template<int dim, int dimworld>
  inline BSGridElement<dim,dimworld>&
  BSGridEntity<0,dim,dimworld> :: geometry ()
  {
    if(!builtgeometry_) builtgeometry_ = geo_.builtGeom(*item_);

    return geo_;
  }

  template<int dim, int dimworld>
  inline int BSGridEntity<0,dim,dimworld> :: index()
  {
    return index_;
  }

  template<int dim, int dimworld>
  inline int BSGridEntity<0,dim,dimworld> :: global_index()
  {
    assert(item_ != 0);
    return item_->getIndex();
  }

  template<int dim, int dimworld>
  inline bool BSGridEntity<0,dim,dimworld> :: hasChildren()
  {
    return (item_->down() != 0);
  }

  template<int dim, int dimworld>
  inline BSGridHierarchicIterator<dim,dimworld> BSGridEntity<0,dim,dimworld> :: hbegin (int maxlevel)
  {
    return BSGridHierarchicIterator<dim,dimworld>(grid_,*item_,maxlevel);
  }

  template<int dim, int dimworld>
  inline BSGridHierarchicIterator<dim,dimworld> BSGridEntity<0,dim,dimworld> :: hend (int maxlevel)
  {
    return BSGridHierarchicIterator<dim,dimworld> (grid_,*item_,maxlevel,true);
  }

  template<int dim, int dimworld>
  inline BSGridIntersectionIterator<dim,dimworld> BSGridEntity<0,dim,dimworld> :: ibegin ()
  {
    return BSGridIntersectionIterator<dim,dimworld> (grid_,item_,level());
  }

  template<int dim, int dimworld>
  inline BSGridIntersectionIterator<dim,dimworld> BSGridEntity<0,dim,dimworld> :: iend ()
  {
    return BSGridIntersectionIterator<dim,dimworld> (grid_, 0 ,level(),true);
  }

  template<int dim, int dimworld>
  inline void BSGridEntity<0,dim,dimworld> :: ibegin ( BSGridIntersectionIterator<dim,dimworld> &it)
  {
    it.first(*item_);
  }

  template<int dim, int dimworld>
  inline void BSGridEntity<0,dim,dimworld> :: iend ( BSGridIntersectionIterator<dim,dimworld> &it)
  {
    it.done();
  }

  template<int dim, int dimworld>
  inline BSGridLevelIterator<0,dim,dimworld,All_Partition> BSGridEntity<0,dim,dimworld> :: father()
  {
    return BSGridLevelIterator<0,dim,dimworld,All_Partition> (grid_,*(item_->up()));
  }

  // Adaptation methods
  template<int dim, int dimworld>
  inline bool BSGridEntity<0,dim,dimworld> :: mark (int ref)
  {
    return true;
  }

  // Adaptation methods
  template<int dim, int dimworld>
  inline AdaptationState BSGridEntity<0,dim,dimworld> :: state ()
  {
    return NONE;
  }

  /***********************************************************************
  ######  #       ######  #    #  ######  #    #   #####
  #       #       #       ##  ##  #       ##   #     #
  #####   #       #####   # ## #  #####   # #  #     #
  #       #       #       #    #  #       #  # #     #
  #       #       #       #    #  #       #   ##     #
  ######  ######  ######  #    #  ######  #    #     #
  ***********************************************************************/
  template<int dim, int dimworld>
  inline BSGridElement<dim,dimworld> :: BSGridElement(bool makeRefElement)
    : builtinverse_ (false) , builtA_ (false) , builtDetDF_ (false)
  {
    switch (dim) {
    case 0 : eltype_=vertex;break;
    case 1 : eltype_=line;break;
    case 2 : eltype_=triangle;break;
    case 3 : eltype_=tetrahedron;break;
    default : {
      std::cerr << "Wrong element type! \n";
      abort();
    }
    }

    // create reference element
    if(makeRefElement)
    {
      coord_ = 0.0;
      for(int i=1; i<dim+1; i++)
        coord_(i-1,i) = 1.0;
    }
  }

  //   B U I L T G E O M   - - -


  template <int dim, int dimworld>
  inline void BSGridElement<dim,dimworld> :: calcElMatrix ()
  {
    // creat Matrix A (=Df)               INDIZES: col/row
    // Mapping: R^dim -> R^3,  F(x) = A x + p_0
    // columns:    p_1 - p_0  |  p_2 - p_0  |  p_3 - p_0

    for (int i=0; i<dim; i++) A_(i) = coord_(i+1) - coord_(0);
    builtA_ = true;
  }

  template<int dim, int dimworld> //dim = dimworld = 3
  inline void BSGridElement<dim,dimworld> :: buildJacobianInverse()
  {
    if(!builtinverse_)
    {
      if(!builtA_) calcElMatrix();

      // DetDf = integration_element
      detDF_ = ABS( A_.invert(Jinv_) );
      builtinverse_ = builtDetDF_ = true;
    }
  }

  template<> //dim = 2 , dimworld = 3
  inline void BSGridElement<2,3> :: buildJacobianInverse()
  {
    if(!builtinverse_)
    {
      if(!builtA_) calcElMatrix();

      tmpVec_(0) = -0.5 * ( ( coord_(1) (1) - (coord_(1) (0)) ) *
                            ( coord_(2) (2) - (coord_(2) (1)) ) -
                            ( coord_(1) (2) - (coord_(1) (1)) ) *
                            ( coord_(2) (1) - (coord_(2) (0)) )  );
      tmpVec_(1) = -0.5 * ( ( coord_(2) (1) - (coord_(2) (0)) ) *
                            ( coord_(0) (2) - (coord_(0) (1)) ) -
                            ( coord_(2) (2) - (coord_(2) (1)) ) *
                            ( coord_(0) (1) - (coord_(0) (0)) )  );
      tmpVec_(2) = -0.5 * ( ( coord_(0) (1) - (coord_(0) (0)) ) *
                            ( coord_(1) (2) - (coord_(1) (1)) ) -
                            ( coord_(0) (2) - (coord_(0) (1)) ) *
                            ( coord_(1) (1) - (coord_(1) (0)) )  );
      detDF_ = tmpVec_.norm2();
      builtinverse_ = builtDetDF_ = true;
    }
  }
#if 0
  template<> //dim = dimworld = 3
  inline void BSGridElement<1,3> :: buildJacobianInverse()
  {
    // Matrix A anlegen (= Df)
    // Mapping: R^1 -> R^3, F(x) = A x + p_0
    // Spalte:  p_1 - p_0
    A_(0) = coord_(1) - coord_(0);

    /*
       // DetDf = integration_element
       detDF_ = A_.determinant();
       volume_ = detDF_ ;
       A_.invert(Jinv_);
       builtinverse_ = builtA_ = true;
     */
  }
#endif

  template <>
  inline bool BSGridElement<3,3> :: builtGeom(const BSSPACE HElementType & item)
  {
    enum { dim = 3 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    for (int i=0; i<(dim+1); i++)
    {
      for (int j=0; j<dimworld; j++)
      {
        coord_(j,i) = static_cast<const BSSPACE GEOElementType &> (item).myvertex(i)->Point()[j];
      }
    }
    return true;
  }

  template <>
  inline bool BSGridElement<2,3> :: builtGeom(const BSSPACE HFaceType & item)
  {
    enum { dim = 2 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    for (int i=0; i<(dim+1); i++)
    {
      for (int j=0; j<dimworld; j++)
      {
        coord_(j,i) = static_cast<const BSSPACE GEOFaceType &> (item).myvertex(i)->Point()[j];
      }
    }
    buildJacobianInverse();
    return true;
  }

  template<int dim, int dimworld>
  inline ElementType BSGridElement<dim,dimworld> ::type ()
  {
    return eltype_;
  }

  template<int dim, int dimworld>
  inline int BSGridElement<dim,dimworld> ::corners () {
    return dimbary;
  }

  template<int dim, int dimworld>
  inline Vec<dimworld,bs_ctype>& BSGridElement<dim,dimworld> :: operator[] (int i)
  {
    assert((i>=0) && (i < dim+1));
    return coord_(i);
  }


  //   G L O B A L   - - -

  template<int dim, int dimworld> // dim = 1,2,3 dimworld = 3
  inline Vec<dimworld,bs_ctype> BSGridElement<dim,dimworld>::
  global(const Vec<dim,bs_ctype>& local)
  {
    if(!builtA_) calcElMatrix();
    return (A_ * local) + coord_(0);
  }

  template<> // dim = dimworld = 3
  inline Vec<3,bs_ctype> BSGridElement<3,3>::
  local(const Vec<3,bs_ctype>& global)
  {
    if (!builtinverse_) buildJacobianInverse();
    return Jinv_ * ( global - coord_(0));
  }

  template<int dim, int dimworld>
  inline bool BSGridElement<dim,dimworld> :: checkInside(const Vec<dim,bs_ctype>& local)
  {
    bs_ctype sum = 0.0;

    for(int i=0; i<dim; i++)
    {
      sum += local(i);
      if(local(i) < 0.0)
      {
        if(ABS(local(i)) > 1e-15)
        {
          return false;
        }
      }
    }

    if( sum > 1.0 )
    {
      if(sum > (1.0 + 1e-15))
        return false;
    }

    return true;
  }

  template<int dim, int dimworld>
  inline bs_ctype BSGridElement<dim,dimworld> :: integration_element (const Vec<dim,bs_ctype>& local)
  {
    if(builtDetDF_)
      return detDF_;

    if(!builtA_) calcElMatrix();
    detDF_ = A_.determinant();
    builtDetDF_ = true;

    return detDF_;
  }

  //  J A C O B I A N _ I N V E R S E  - - -

  template<> // dim = dimworld = 3
  inline Mat<3,3>& BSGridElement<3,3> :: Jacobian_inverse (const Vec<3,bs_ctype>& local)
  {
    if (!builtinverse_) buildJacobianInverse();
    return Jinv_;
  }

  // print the ElementInformation
  template<int dim, int dimworld>
  inline void BSGridElement<dim,dimworld>::print (std::ostream& ss, int indent)
  {
    ss << "BSGridElement<" << dim << "," << dimworld << "> = {\n";
    for(int i=0; i<corners(); i++)
    {
      ss << " corner " << i << " ";
      ss << ((*this)[i]); ss << "\n";
    }
    ss << "} \n";
  }


  template<int dim, int dimworld>
  inline BSGridElement<dim,dim>& BSGridElement<dim, dimworld> :: refelem () {
    return refelem_3d;
  }

}

#endif
