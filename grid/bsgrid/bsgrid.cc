// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BSGRID_CC__
#define __DUNE_BSGRID_CC__

namespace Dune {

  //! singelton holding reference element
  static BSGridElement<3,3> refelem_3d(true);

  template <int dim, int dimworld>
  inline BSGrid<dim,dimworld>::BSGrid(const char* macroTriangFilename
#ifdef _BSGRID_PARALLEL_
                                      , MPI_Comm mpiComm
#endif
                                      )
    : mygrid_ (0) , maxlevel_(0)
#ifdef _BSGRID_PARALLEL_
      , mpAccess_(mpiComm)
#endif
  {
    mygrid_ = new BSSPACE BSGitterImplType (macroTriangFilename
#ifdef _BSGRID_PARALLEL_
                                            , mpAccess_
#endif
                                            );
    assert(mygrid_ != 0);
    mygrid_->printsize();

    postAdapt();
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
        w (*mygrid_) ;
        size_[level][3] = w->size();
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

    //for(int i=0; i<dim+1; i++) globalSize_[i] = -1;

    // set max index of grid
    for(int i=0; i<dim+1; i++)
      globalSize_[i] =  mygrid_->indexManager(i).getMaxIndex();

    /*
       BSGridLeafIterator it    = leafbegin (0);
       BSGridLeafIterator endit = leafend   (0);

       // hier den macIndex vom IndexSet erfragen
       for(; it != endit; ++it)
       {
       if((*it).global_index() > globalSize_[0])
          globalSize_[0] = (*it).global_index();

       BSGridHierarchicIterator<dim,dimworld> hierend = it->hend   (maxlevel());

       for(BSGridHierarchicIterator<dim,dimworld> hierit  = (*it).hbegin (maxlevel()) ;
            hierit != hierend; ++hierit )
       {
        if((*hierit).global_index() > globalSize_[0])
          globalSize_[0] = (*hierit).global_index();
       }
       }
       globalSize_[0]++;
     */
  }

  template <int dim, int dimworld>
  inline int BSGrid<dim,dimworld>::global_size(int codim)
  {
    assert(codim == 0);
    assert(globalSize_[codim] >= 0);
    return globalSize_[codim];
  }

  template <int dim, int dimworld>
  inline int BSGrid<dim,dimworld>::maxlevel() const
  {
    return maxlevel_;
  }
  template <int dim, int dimworld>
  inline BSSPACE BSGitterType *BSGrid<dim,dimworld>::mygrid() {
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
  inline typename BSGrid<dim,dimworld>::LeafIterator BSGrid<dim,dimworld>::leafbegin(int level, PartitionIteratorType pitype)
  {
    return BSGridLeafIterator(*this,level,false,pitype);
  }
  template <int dim, int dimworld>
  inline typename BSGrid<dim,dimworld>::LeafIterator BSGrid<dim,dimworld>::leafend(int level, PartitionIteratorType pitype)
  {
    return BSGridLeafIterator(*this,level,true,pitype);
  }

  // global refine
  template <int dim, int dimworld>
  inline bool BSGrid<dim,dimworld>::globalRefine(int anzahl)
  {
    bool ref = false;
    for (; anzahl>0; anzahl--)
    {
      BSGridLeafIterator endit = leafend   ( maxlevel() );
      for(BSGridLeafIterator it = leafbegin ( maxlevel() ); it != endit; ++it)
      {
        (*it).mark(1);
      }
      ref = adapt();
      if(ref) postAdapt();
    }
    if(ref) loadBalance();
    return ref;
  }

  // adapt grid
  template <int dim, int dimworld>
  inline void BSGrid<dim,dimworld>:: setCoarsenMark ()
  {
    coarsenMark_ = true;
  }

  // preprocess grid
  template <int dim, int dimworld>
  inline bool BSGrid<dim,dimworld>::preAdapt()
  {
    return coarsenMark_;
  }

  // adapt grid
  template <int dim, int dimworld>
  inline bool BSGrid<dim,dimworld>::adapt()
  {
#ifdef _BSGRID_PARALLEL_
    bool ref = mygrid_->duneAdapt(); // adapt grid
#else
    bool ref = mygrid_->adapt(); // adapt grid
#endif
    if(ref)
    {
      calcMaxlevel();             // calculate new maxlevel
      calcExtras();               // reset size and things
    }
    return ref;
  }

  // post process grid
  template <int dim, int dimworld>
  inline void BSGrid<dim,dimworld>::postAdapt()
  {
    {
      typename BSSPACE BSLeafIteratorMaxLevel w (*mygrid_) ;
      for (w->first () ; ! w->done () ; w->next ())
      {
        w->item ().resetRefinedTag();
      }
    }
    coarsenMark_ = false;
  }

  template <int dim, int dimworld>
  inline bool BSGrid<dim,dimworld>::loadBalance()
  {
#ifdef _BSGRID_PARALLEL_
    bool changed = mygrid_->duneLoadBalance();
    if(changed)
    {
      mygrid_->duneExchangeDynamicState();
      calcMaxlevel();             // calculate new maxlevel
      calcExtras();               // reset size and things
    }
#else
    return false;
#endif
  }

  // adapt grid
  template <int dim, int dimworld> template <class DataCollectorType>
  inline bool BSGrid<dim,dimworld>::loadBalance(DataCollectorType & dc)
  {
#ifdef _BSGRID_PARALLEL_
    //std::cout << "Start load balance on proc " << myRank() << "\n";
    //std::cout.flush();

    typedef BSGridEntity<0,dim,dimworld> EntityType;
    EntityType en (*this);

    BSSPACE GatherScatterImpl< EntityType , DataCollectorType > gs(en,dc);

    bool changed = mygrid_->duneLoadBalance(gs);

    if(changed)
    {
      mygrid_->duneExchangeDynamicState(gs);
      calcMaxlevel();             // calculate new maxlevel
      calcExtras();               // reset size and things
    }
#else
    return false;
#endif
  }

  // adapt grid
  template <int dim, int dimworld> template <class DataCollectorType>
  inline bool BSGrid<dim,dimworld>::communicate(DataCollectorType & dc)
  {
#ifdef _BSGRID_PARALLEL_
    typedef BSGridEntity<0,dim,dimworld> EntityType;
    EntityType en (*this);
    BSSPACE GatherScatterImpl< EntityType , DataCollectorType > gs(en,dc);
    mygrid_->duneExchangeDynamicState(gs);
    return true;
#else
    return false;
#endif
  }

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

      mygrid_ = new BSSPACE BSGitterImplType (macroName);

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
      if(iter_.size() > 0)
      {
        index_=0;
        BSGridEntity<0,dim,dimworld> * obj =
          new BSGridEntity<codim,dim,dimworld> (grid_,iter_.item(),index_,level_);
        objEntity_.store ( obj );
      }
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
    BSGridEntity<codim,dim,dimworld> * obj =
      new BSGridEntity<codim,dim,dimworld> (grid_,item,index_,level_);
    // objEntity deletes entity if no refCount is left
    objEntity_.store ( obj );
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline BSGridLevelIterator<codim,dim,dimworld,pitype> &
  BSGridLevelIterator<codim,dim,dimworld,pitype> :: operator ++()
  {
    assert(index_ >= 0);

    iter_.next();
    index_++;
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
    assert(iter_.size() > 0);
    return (*objEntity_);
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline BSGridEntity<codim,dim,dimworld>*
  BSGridLevelIterator<codim,dim,dimworld,pitype>::operator->()
  {
    assert(iter_.size() > 0);
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
  BSGridLeafIterator(BSGrid<dim,dimworld> &grid, int level,bool
                     end, PartitionIteratorType pitype)
    : index_(-1)
      , level_(level)
      , iter_(grid.mygrid()->container(), level )
      , pitype_ (pitype)
  {
    if(!end)
    {
      iter_.first();
      if(iter_.size() > 0)
      {
        index_=0;
        BSGridEntity<0,dim,dimworld> * obj =
          new BSGridEntity<codim,dim,dimworld> (grid,iter_.item(),index_,level_);
        objEntity_.store ( obj );
      }
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
        // we have children and they lie in the disired level range
        if(item_->level() <= maxlevel_)
        {
          BSGridEntity<0,dim,dimworld> * obj =
            new BSGridEntity<0,dim,dimworld>(grid_,*item_ ,0,maxlevel_);
          // objEntity deletes entity pointer when no refCount is left
          objEntity_.store ( obj );
        }
        else
        { // otherwise do nothing
          item_ = 0;
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
                             BSSPACE HElementType *el, int wLevel,bool end) :
    entity_( grid )
    , item_(0), neigh_(0), index_(0)
    , needSetup_ (true), needNormal_(true)
    , interSelfGlobal_ (false)
    , theSituation_ (false) , daOtherSituation_ (false)
    , isBoundary_ (true) // isBoundary_ == true means no neighbour
    , ghost_(false)
  {
    if( !end )
    {
      first(*el,wLevel);
    }
    else
    {
      done();
    }
  }

  template<int dim, int dimworld>
  inline void BSGridIntersectionIterator<dim,dimworld> :: checkGhost ()
  {
#ifdef _BSGRID_PARALLEL_
    ghost_ = false;
    if(isBoundary_)
    {
      typename BSSPACE PLLBndFaceType * bnd = item_->myneighbour(index_).first;
      if(bnd->bndtype() == BSSPACE ProcessorBoundary_t)
      {
        isBoundary_ = false;
        ghost_ = true;
      }
    }
#endif
  }

  template<int dim, int dimworld>
  inline void BSGridIntersectionIterator<dim,dimworld> ::
  first (BSSPACE HElementType & elem, int wLevel)
  {
    item_  = static_cast<BSSPACE GEOElementType *> (&elem);
    index_ = 0;
    neigh_ = 0;
    neighpair_.first  = 0;
    neighpair_.second = 0;

    // if needed more than once we spare the virtual funtion call
    isBoundary_ = item_->myneighbour(index_).first->isboundary();
    checkGhost();

    theSituation_ = ( (elem.level() < wLevel ) && elem.leaf() );
    daOtherSituation_ = false;

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
    assert(item_ != 0);

    if( neighpair_.first && theSituation_ && daOtherSituation_ )
    {
      neighpair_.first = neighpair_.first->next();
    }
    else
    {
      neighpair_.first = 0;
    }

    if( !neighpair_.first )
    {
      index_++;
      neighpair_.first = 0;
    }

    if(index_ > dim)
    {
      item_ = 0;
      return *this;
    }

    // if needed more than once we spare the virtual funtion call
    isBoundary_ = item_->myneighbour(index_).first->isboundary();
    checkGhost();

    needSetup_  = true;
    needNormal_ = true;
    return *this;
  }

  template<int dim, int dimworld>
  inline bool BSGridIntersectionIterator<dim,dimworld> ::
  operator== (const BSGridIntersectionIterator<dim,dimworld>& i) const
  {
    return (item_ == i.item_);
  }

  template<int dim, int dimworld>
  inline bool BSGridIntersectionIterator<dim,dimworld> ::
  operator!= (const BSGridIntersectionIterator<dim,dimworld>& i) const
  {
    return (item_ != i.item_);
  }

  // set new neighbor
  template<int dim, int dimworld>
  inline void BSGridIntersectionIterator<dim,dimworld> ::
  setNeighbor ()
  {
    assert( this->neighbor() );

    if(! neighpair_.first )
    {
      // get the face(index_)  of this element
      neighpair_ = (*item_).myintersection(index_);
      assert(neighpair_.first);

      // the "situation" describes the case we are on a leaf element but the
      // walking level is deeper then or own level. This means the neighbour
      // can have a deeper level and therefor we have on this face not one
      // neighbour, we have all children as neighbours. So we go to the face
      // and then to the chilren of this face which are the all faces of the
      // children on this face. If we went down then we also are allowed to
      // go next otherwise we are not allowe to go next which is described as
      // "da other situation"

      if( theSituation_ && neighpair_.first->down() )
      {
        neighpair_.first = neighpair_.first->down();
        daOtherSituation_ = true;
      }
      else
      {
        daOtherSituation_ = false;
      }
    }

#ifdef _BSGRID_PARALLEL_
    if(ghost_)
    {
      assert( item_->myneighbour(index_).first->isboundary() );

      BSSPACE NeighbourPairType np = (neighpair_.second < 0) ?
                                     (neighpair_.first->nb.front()) : (neighpair_.first->nb.rear());

      BSSPACE PLLBndFaceType * bnd = np.first;
      numberInNeigh_ = np.second;

      // if our level is smaller then the level of the real ghost then go one
      // level up and set the element
      if(bnd->ghostLevel() != bnd->level())
      {
        assert(bnd->ghostLevel() < bnd->level());
        assert(bnd->up());

        bnd = bnd->up();
        assert(bnd->level() == bnd->ghostLevel());
      }

      //std::cout << "Ghostnumber is " << bnd->getIdx() << "\n";
      //int ghidx = bnd->getIndex();
      //BSSPACE logFile  << "GhostNumber  = " << ghidx << "\n";

      entity_.setGhost( *bnd , index_);
      needSetup_ = false;
      return;
    }
#endif

    // same as in method myneighbour of Tetra and Hexa in gitter_sti.hh
    // neighpair_.second is the twist of the face
    BSSPACE NeighbourPairType np = (neighpair_.second < 0) ?
                                   (neighpair_.first->nb.front()) : (neighpair_.first->nb.rear());

    neigh_ = np.first;
    numberInNeigh_ = np.second;

    assert(neigh_ != item_);
    assert(neigh_ != 0);

    entity_.setelement(*neigh_, index_);
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
    return isBoundary_;
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
    assert(item_ != 0);

    if(needSetup_) setNeighbor();
    return numberInNeigh_;
  }

  template< int dim, int dimworld>
  inline FieldVector<bs_ctype, dimworld>&
  BSGridIntersectionIterator<dim,dimworld>::
  outer_normal(FieldVector<bs_ctype, dim-1>& local)
  {
    return this->outer_normal();
  }

  template< int dim, int dimworld>
  inline FieldVector<bs_ctype, dimworld>&
  BSGridIntersectionIterator<dim,dimworld>::outer_normal()
  {
    assert(item_ != 0);
    if(needNormal_)
    {
      if( boundary() || ( !daOtherSituation_ ) )
      {
        // if boundary calc normal normal ;)
        item_->outerNormal(index_,outNormal_);
      }
      else
      {
        if(needSetup_) setNeighbor();
        neigh_->neighOuterNormal(numberInNeigh_,outNormal_);
      }
      needNormal_ = false;
    }
    return outNormal_;
  }

  template< int dim, int dimworld>
  inline FieldVector<bs_ctype, dimworld>&
  BSGridIntersectionIterator<dim,dimworld>::
  unit_outer_normal(FieldVector<bs_ctype, dim-1>& local)
  {
    return this->unit_outer_normal();
  }

  template< int dim, int dimworld>
  inline FieldVector<bs_ctype, dimworld>&
  BSGridIntersectionIterator<dim,dimworld>::unit_outer_normal()
  {
    unitOuterNormal_  = this->outer_normal();
    unitOuterNormal_ /= unitOuterNormal_.two_norm();
    return unitOuterNormal_;
  }

  template< int dim, int dimworld>
  inline BSGridElement<dim-1,dimworld>&
  BSGridIntersectionIterator<dim,dimworld>::intersection_self_global ()
  {
    const BSSPACE Gitter::Geometric::hface3_GEO & face = *(item_->myhface3(index_));
    bool init = interSelfGlobal_.builtGeom(face);

    return interSelfGlobal_;
  }

  template< int dim, int dimworld>
  inline BSGridBoundaryEntity<dim,dimworld>&
  BSGridIntersectionIterator<dim,dimworld>::boundaryEntity ()
  {
    typename BSSPACE BNDFaceType * bnd = item_->myneighbour(index_).first;
    int id = bnd->bndtype(); // id's are positive
    bndEntity_.setId( -id );
    return bndEntity_;
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
                                                      BSSPACE HElementType & element,int index, int wLevel) :
    grid_(grid), item_(static_cast<BSSPACE GEOElementType *> (&element))
    , builtgeometry_(false), geo_(false)
    , index_(index) , walkLevel_ (wLevel) , glIndex_ (element.getIndex())
    , level_ (element.level())
    , ghost_(0)
  {}

  template<int dim, int dimworld>
  inline void
  BSGridEntity<0,dim,dimworld> :: setelement(BSSPACE HElementType & element,int index)
  {
    item_= static_cast<BSSPACE GEOElementType *> (&element);
    builtgeometry_=false;
    index_=index;
    level_ = item_->level();
    glIndex_ = item_->getIndex();
  }

  template<int dim, int dimworld>
  inline void
  BSGridEntity<0,dim,dimworld> :: setGhost(BSSPACE PLLBndFaceType & ghost,int index)
  {
    item_ = 0;
    ghost_ = &ghost;
    index_=index;
    glIndex_ = ghost.getIndex();
    level_ = ghost.level();
    builtgeometry_ = false; //geo_.builtGhost(ghost);
  }

  template<int dim, int dimworld>
  inline int
  BSGridEntity<0,dim,dimworld> :: level()
  {
    return level_;
  }

  template<int dim, int dimworld>
  inline BSGridElement<dim,dimworld>&
  BSGridEntity<0,dim,dimworld> :: geometry ()
  {
#ifdef _BSGRID_PARALLEL_
    if(!builtgeometry_)
    {
      if(item_) builtgeometry_ = geo_.builtGeom(*item_);
      else builtgeometry_ = geo_.builtGhost(*ghost_);
    }
#else
    if(!builtgeometry_) builtgeometry_ = geo_.builtGeom(*item_);
#endif
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
    return glIndex_;
  }

  template<int dim, int dimworld>
  template<int cc>
  inline int BSGridEntity<0,dim,dimworld> :: subIndex (int i)
  {
    assert(cc == dim);
    assert(item_ != 0);
    return IndexWrapper<cc>::subIndex ( *item_ ,i);
  }

  template<int dim, int dimworld>
  inline PartitionType BSGridEntity<0,dim,dimworld> ::
  partitionType () const
  {
    return ((item_) ? InteriorEntity : GhostEntity);
  }


  template<int dim, int dimworld>
  inline bool BSGridEntity<0,dim,dimworld> :: hasChildren()
  {
    assert(item_ != 0);
    return (item_->down() != 0);
  }

  template<int dim, int dimworld>
  inline BSGridHierarchicIterator<dim,dimworld> BSGridEntity<0,dim,dimworld> :: hbegin (int maxlevel)
  {
    assert(item_ != 0);
    return BSGridHierarchicIterator<dim,dimworld>(grid_,*item_,maxlevel);
  }

  template<int dim, int dimworld>
  inline BSGridHierarchicIterator<dim,dimworld> BSGridEntity<0,dim,dimworld> :: hend (int maxlevel)
  {
    assert(item_ != 0);
    return BSGridHierarchicIterator<dim,dimworld> (grid_,*item_,maxlevel,true);
  }

  template<int dim, int dimworld>
  inline BSGridIntersectionIterator<dim,dimworld> BSGridEntity<0,dim,dimworld> :: ibegin ()
  {
    assert(item_ != 0);
    return BSGridIntersectionIterator<dim,dimworld> (grid_,item_,walkLevel_);
  }

  template<int dim, int dimworld>
  inline BSGridIntersectionIterator<dim,dimworld> BSGridEntity<0,dim,dimworld> :: iend ()
  {
    assert(item_ != 0);
    return BSGridIntersectionIterator<dim,dimworld> (grid_, 0 ,walkLevel_,true);
  }

  template<int dim, int dimworld>
  inline void BSGridEntity<0,dim,dimworld> :: ibegin ( BSGridIntersectionIterator<dim,dimworld> &it)
  {
    assert(item_ != 0);
    it.first(*item_,walkLevel_);
  }

  template<int dim, int dimworld>
  inline void BSGridEntity<0,dim,dimworld> :: iend ( BSGridIntersectionIterator<dim,dimworld> &it)
  {
    assert(item_ != 0);
    it.done();
  }

  /*
     template<int dim, int dimworld>
     inline BSGridLevelIterator<0,dim,dimworld,All_Partition>
     BSGridEntity<0,dim,dimworld> :: father()
     {
     if(! item_->up() )
     {
      std::cerr << "BSGridEntity<0," << dim << "," << dimworld << "> :: father() : no father of entity globalid = " << global_index() << "\n";
      return BSGridLevelIterator<0,dim,dimworld,All_Partition> (grid_,*item_);
     }

     return BSGridLevelIterator<0,dim,dimworld,All_Partition> (grid_,*(item_->up()));
     }
   */

  template<int dim, int dimworld>
  inline BSGridEntity<0,dim,dimworld>
  BSGridEntity<0,dim,dimworld> :: newEntity ()
  {
    assert(item_ != 0);
    BSGridEntity<0,dim,dimworld> tmp ( *this );
    return tmp;
  }

  template<int dim, int dimworld>
  inline void
  BSGridEntity<0,dim,dimworld> :: father( BSGridEntity<0,dim,dimworld> & vati )
  {
    assert(item_ != 0);
    vati.setelement( *(item_->up()) , 0 );
  }

  // Adaptation methods
  template<int dim, int dimworld>
  inline bool BSGridEntity<0,dim,dimworld> :: mark (int ref)
  {
    assert(item_ != 0);
    // refine_element_t and coarse_element_t are defined in bsinclude.hh
    if(ref < 0)
    {
      if(level() <= 0) return false;
      if((*item_).requestrule() == BSSPACE refine_element_t)
      {
        return false;
      }

      (*item_).request( BSSPACE coarse_element_t );
      grid_.setCoarsenMark();
      return true;
    }

    if(ref > 0)
    {
      (*item_).request( BSSPACE refine_element_t );
      return true;
    }
    return false;
  }

  // Adaptation methods
  template<int dim, int dimworld>
  inline AdaptationState BSGridEntity<0,dim,dimworld> :: state () const
  {
    assert(item_ != 0);
    if((*item_).requestrule() == BSSPACE coarse_element_t)
    {
      return COARSEN;
    }

    if(item_->hasBeenRefined())
    {
      return REFINED;
    }

    return NONE;
  }


  /************************************************************************************
  ######  #    #   #####     #     #####   #   #
  #       ##   #     #       #       #      # #
  #####   # #  #     #       #       #       #
  #       #  # #     #       #       #       #
  #       #   ##     #       #       #       #
  ######  #    #     #       #       #       #
  ************************************************************************************/
  // --Entity

  template<int codim, int dim, int dimworld>
  inline int BSGridEntity<codim,dim,dimworld> :: global_index ()
  {
    return item_->getIndex();
  }

  /***********************************************************************
  ######  #       ######  #    #  ######  #    #   #####
  #       #       #       ##  ##  #       ##   #     #
  #####   #       #####   # ## #  #####   # #  #     #
  #       #       #       #    #  #       #  # #     #
  #       #       #       #    #  #       #   ##     #
  ######  ######  ######  #    #  ######  #    #     #
  ***********************************************************************/
  // --Element
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
      detDF_ = std::abs( A_.invert(Jinv_) );
      builtinverse_ = builtDetDF_ = true;
    }
  }

  template<> //dim = 2 , dimworld = 3
  inline void BSGridElement<2,3> :: buildJacobianInverse()
  {
    if(!builtinverse_)
    {
      if(!builtA_) calcElMatrix();

      tmpVec_[0] = -0.5 * ( ( coord_(1)[1] - (coord_(1)[0]) ) *
                            ( coord_(2)[2] - (coord_(2)[1]) ) -
                            ( coord_(1)[2] - (coord_(1)[1]) ) *
                            ( coord_(2)[1] - (coord_(2)[0]) )  );
      tmpVec_[1] = -0.5 * ( ( coord_(2)[1] - (coord_(2)[0]) ) *
                            ( coord_(0)[2] - (coord_(0)[1]) ) -
                            ( coord_(2)[2] - (coord_(2)[1]) ) *
                            ( coord_(0)[1] - (coord_(0)[0]) )  );

      tmpVec_[2] = -0.5 * ( ( coord_(0)[1] - (coord_(0)[0]) ) *
                            ( coord_(1)[2] - (coord_(1)[1]) ) -
                            ( coord_(0)[2] - (coord_(0)[1]) ) *
                            ( coord_(1)[1] - (coord_(1)[0]) ) ) ;
      detDF_ = tmpVec_.two_norm();
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
  inline bool BSGridElement<3,3> :: builtGeom(const BSSPACE GEOElementType & item)
  {
    enum { dim = 3 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    for (int i=0; i<(dim+1); i++)
    {
      for (int j=0; j<dimworld; j++)
      {
        coord_(j,i) = (item).myvertex(i)->Point()[j];
      }
    }
    return true;
  }

  template <>
  inline bool BSGridElement<3,3> :: builtGhost(const BSSPACE PLLBndFaceType & ghost)
  {
    enum { dim = 3 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    for (int i=0; i<dim; i++) // col is the point vector
    {
      const double (&p)[3] = (ghost.myhface3(0))->myvertex(i)->Point();
      for (int j=0; j<dimworld; j++) // row is the coordinate of the point
      {
        coord_(j,i) = p[j];
      }
    }

    //std::cout << "get opp vx\n";
    {
      const double (&p)[3] = ghost.oppositeVertex(0);
      for (int j=0; j<dimworld; j++)
      {
        coord_(j,3) = p[j];
      }
    }
    //std::cout << coord_ << "\n";

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
  inline FieldVector<bs_ctype, dimworld>& BSGridElement<dim,dimworld> :: operator[] (int i)
  {
    assert((i>=0) && (i < dim+1));
    return coord_(i);
  }


  //   G L O B A L   - - -

  template<int dim, int dimworld> // dim = 1,2,3 dimworld = 3
  inline FieldVector<bs_ctype, dimworld> BSGridElement<dim,dimworld>::
  global(const FieldVector<bs_ctype, dim>& local)
  {
    if(!builtA_) calcElMatrix();
    return (A_ * local) + coord_(0);
  }

  template<> // dim = dimworld = 3
  inline FieldVector<bs_ctype, 3> BSGridElement<3,3>::
  local(const FieldVector<bs_ctype, 3>& global)
  {
    if (!builtinverse_) buildJacobianInverse();
    return Jinv_ * ( global - coord_(0));
  }

  template<int dim, int dimworld>
  inline bool BSGridElement<dim,dimworld> :: checkInside(const FieldVector<bs_ctype, dim>& local)
  {
    bs_ctype sum = 0.0;

    for(int i=0; i<dim; i++)
    {
      sum += local[i];
      if(local[i] < 0.0)
      {
        if(std::abs(local[i]) > 1e-15)
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
  inline bs_ctype BSGridElement<dim,dimworld> :: integration_element (const FieldVector<bs_ctype, dim>& local)
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
  inline Mat<3,3>& BSGridElement<3,3> :: Jacobian_inverse (const FieldVector<bs_ctype, 3>& local)
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
