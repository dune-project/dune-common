// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ALU3DGRID_CC__
#define __DUNE_ALU3DGRID_CC__

namespace Dune {

  // singleton holding reference elements
  template<int dim, class GridImp>
  struct ALU3dGridReferenceGeometry
  {
    ALU3dGridMakeableGeometry<dim,dim,GridImp> refelem;
    ALU3dGridReferenceGeometry () : refelem (true) {};
  };

  template <int dim, int dimworld>
  inline ALU3dGrid<dim,dimworld>::ALU3dGrid(const char* macroTriangFilename
#ifdef _ALU3DGRID_PARALLEL_
                                            , MPI_Comm mpiComm
#endif
                                            )
    : mygrid_ (0) , maxlevel_(0)
      , coarsenMark_(false)
#ifdef _ALU3DGRID_PARALLEL_
      , mpAccess_(mpiComm) , myRank_( mpAccess_.myrank() )
#else
      ,  myRank_(-1)
#endif
      , hIndexSet_ (*this,globalSize_)
      , levelIndexSet_(0)
  {
    mygrid_ = new ALU3DSPACE GitterImplType (macroTriangFilename
#ifdef _ALU3DGRID_PARALLEL_
                                             , mpAccess_
#endif
                                             );
    assert(mygrid_ != 0);

#ifdef _ALU3DGRID_PARALLEL_
    loadBalance();
    __MyRank__ = mpAccess_.myrank();
#endif

    mygrid_->printsize();

    postAdapt();
    calcExtras();
  }

#ifdef _ALU3DGRID_PARALLEL_
  template <int dim, int dimworld>
  inline ALU3dGrid<dim,dimworld>::ALU3dGrid(MPI_Comm mpiComm)
    : mygrid_ (0) , maxlevel_(0)
      , coarsenMark_(false)
      , mpAccess_(mpiComm) , myRank_( mpAccess_.myrank() )
      , hIndexSet_ (*this,globalSize_)
      , levelIndexSet_(0)
  {}
#else
  template <int dim, int dimworld>
  inline ALU3dGrid<dim,dimworld>::ALU3dGrid(int myrank)
    : mygrid_ (0) , maxlevel_(0)
      , coarsenMark_(false)
      , myRank_(myrank)
      , hIndexSet_ (*this,globalSize_)
      , levelIndexSet_(0)
  {
    for(int i=0; i<dim+1; i++) globalSize_[i] = 0;
  }
#endif

  template <int dim, int dimworld>
  inline ALU3dGrid<dim,dimworld>::ALU3dGrid(const ALU3dGrid<dim,dimworld> & g)
    : mygrid_ (0) , maxlevel_(0)
      , coarsenMark_(false) , myRank_(-1)
      , hIndexSet_(*this,globalSize_) , levelIndexSet_(0)
  {
    DUNE_THROW(ALU3dGridError,"Do not use copy constructor of ALU3dGrid! \n");
  }

  template <int dim, int dimworld>
  inline ALU3dGrid<dim,dimworld> & ALU3dGrid<dim,dimworld>::operator = (const ALU3dGrid<dim,dimworld> & g)
  {
    DUNE_THROW(ALU3dGridError,"Do not use assignment operator of ALU3dGrid! \n");
    return (*this);
  }

  template <int dim, int dimworld>
  inline ALU3dGrid<dim,dimworld>::~ALU3dGrid()
  {
    if(levelIndexSet_) delete levelIndexSet_;
    if(mygrid_) delete mygrid_;
  }

  template <int dim, int dimworld>
  inline int ALU3dGrid<dim,dimworld>::size(int level, int codim) const
  {
    return levelIndexSet().size(level,codim);
  }

  template <int dim, int dimworld>
  inline void ALU3dGrid<dim,dimworld>::updateStatus()
  {
    calcMaxlevel();
    calcExtras();
  }

  template <int dim, int dimworld>
  inline void ALU3dGrid<dim,dimworld>::calcMaxlevel()
  {
    maxlevel_ = 0;
    assert(mygrid_ != 0);
    ALU3DSPACE BSLeafIteratorMaxLevel w (*mygrid_) ;
    for (w->first () ; ! w->done () ; w->next ())
    {
      if(w->item().level() > maxlevel_ ) maxlevel_ = w->item().level();
    }
  }

  template <int dim, int dimworld>
  inline void ALU3dGrid<dim,dimworld>::calcExtras()
  {
    // set max index of grid
    for(int i=0; i<dim+1; i++)
    {
      globalSize_[i] = (*mygrid_).indexManager(i).getMaxIndex() + 1;
      //std::cout << " global Size " << globalSize_[i] << "\n";
    }

    //std::cout << "proc " << mpAccess_.myrank() << " num el = " << globalSize_[0] << "\n";
    if(levelIndexSet_) (*levelIndexSet_).calcNewIndex();
  }

  template <int dim, int dimworld>
  inline void ALU3dGrid<dim,dimworld>::recalcGlobalSize()
  {
    for(int i=0; i<dim+1; i++) globalSize_[i] = -1;


    // hier den macIndex vom IndexSet erfragen
    LeafIteratorType endit  = leafend   (0);
    for(LeafIteratorType it = leafbegin (0); it != endit; ++it)
    {
      if((*it).globalIndex() > globalSize_[0])
        globalSize_[0] = (*it).globalIndex();

      HierarchicIteratorImp hierend = it->hend (maxlevel());
      for(HierarchicIteratorImp hierit  = (*it).hbegin (maxlevel()) ;
          hierit != hierend; ++hierit )
      {
        if((*hierit).globalIndex() > globalSize_[0])
          globalSize_[0] = (*hierit).globalIndex();
      }
    }
    globalSize_[0]++;
  }

  template <int dim, int dimworld>
  inline int ALU3dGrid<dim,dimworld>::global_size(int codim) const
  {
    assert(globalSize_[codim] >= 0);
    //std::cout << globalSize_[codim] << " Size of cd " << codim << "\n";
    return globalSize_[codim];
  }

  template <int dim, int dimworld>
  inline int ALU3dGrid<dim,dimworld>::maxlevel() const
  {
    return maxlevel_;
  }

  template <int dim, int dimworld>
  inline ALU3DSPACE GitterImplType & ALU3dGrid<dim,dimworld>::myGrid()
  {
    return *mygrid_;
  }

  // lbegin methods
  template <int dim, int dimworld>
  template <int cd, PartitionIteratorType pitype>
  inline typename ALU3dGrid<dim,dimworld>::Traits::template codim<cd>::template partition<pitype>::LevelIterator
  ALU3dGrid<dim,dimworld>::lbegin(int level) const {
    ALU3dGridLevelIterator<cd,pitype,const MyType>  it (*this,level);
    return it;
  }

  template <int dim, int dimworld>
  template <int cd, PartitionIteratorType pitype>
  inline typename ALU3dGrid<dim,dimworld>::Traits::template codim<cd>::template partition<pitype>::LevelIterator
  ALU3dGrid<dim,dimworld>::lend(int level) const {
    ALU3dGridLevelIterator<cd,pitype,const MyType> it (*this,level,true);
    return it;
  }

  // lbegin methods
  template <int dim, int dimworld>
  template <int cd>
  inline typename ALU3dGrid<dim,dimworld>::Traits::template codim<cd>::template partition<All_Partition>::LevelIterator
  ALU3dGrid<dim,dimworld>::lbegin(int level) const {
    ALU3dGridLevelIterator<cd,All_Partition,const MyType> it (*this,level);
    return it;
  }

  template <int dim, int dimworld>
  template <int cd>
  inline typename ALU3dGrid<dim,dimworld>::Traits::template codim<cd>::template partition<All_Partition>::LevelIterator
  ALU3dGrid<dim,dimworld>::lend(int level) const {
    ALU3dGridLevelIterator<cd,All_Partition,const MyType> it (*this,level,true);
    return it;
  }

  // leaf methods
  template <int dim, int dimworld>
  inline typename ALU3dGrid<dim,dimworld>::LeafIteratorType
  ALU3dGrid<dim,dimworld>::leafbegin(int level, PartitionIteratorType pitype) const
  {
    ALU3dGridLeafIterator<const MyType> it ((*this),level,false,pitype);
    return it;
  }
  template <int dim, int dimworld>
  inline typename ALU3dGrid<dim,dimworld>::LeafIteratorType
  ALU3dGrid<dim,dimworld>::leafend(int level, PartitionIteratorType pitype) const
  {
    ALU3dGridLeafIterator<const MyType> it ((*this),level,true,pitype);
    return it;
  }

  // global refine
  template <int dim, int dimworld>
  inline bool ALU3dGrid<dim,dimworld>::
  mark(int ref, const typename Traits::template codim<0>::Entity & ep ) const
  {
    return (this->template getRealEntity<0> (ep)).mark(ref);
  }

  // global refine
  template <int dim, int dimworld>
  inline bool ALU3dGrid<dim,dimworld>::globalRefine(int anzahl)
  {
    bool ref = false;
    for (; anzahl>0; anzahl--)
    {
      LeafIteratorType endit  = leafend   ( maxlevel() );
      for(LeafIteratorType it = leafbegin ( maxlevel() ); it != endit; ++it)
      {
        this->mark(1, (*it) );
      }
      ref = this->adapt();
      if(ref) this->postAdapt();
    }
    if(ref) this->loadBalance();
    return ref;
  }

  // adapt grid
  template <int dim, int dimworld>
  inline void ALU3dGrid<dim,dimworld>:: setCoarsenMark () const
  {
    coarsenMark_ = true;
  }

  // preprocess grid
  template <int dim, int dimworld>
  inline bool ALU3dGrid<dim,dimworld>::preAdapt()
  {
    return coarsenMark_;
  }

  // adapt grid
  template <int dim, int dimworld>
  inline bool ALU3dGrid<dim,dimworld>::adapt()
  {
#ifdef _ALU3DGRID_PARALLEL_
    bool ref = myGrid().duneAdapt(); // adapt grid
#else
    bool ref = myGrid().adapt(); // adapt grid
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
  inline void ALU3dGrid<dim,dimworld>::postAdapt()
  {
#ifdef _ALU3DGRID_PARALLEL_
    for(int l=0; l<= maxlevel(); l++)
    {
      {
        ALU3DSPACE BSLevelIterator<0>::IteratorType w ( myGrid().container() , l ) ;
        for (w.first () ; ! w.done () ; w.next ())
        {
          w.item ().resetRefinedTag();
        }
      }
    }
#else
    {
      ALU3DSPACE BSLeafIteratorMaxLevel w ( myGrid() ) ;
      for (w->first () ; ! w->done () ; w->next ())
      {
        w->item ().resetRefinedTag();
      }
    }
#endif
    coarsenMark_ = false;
  }

  template <int dim, int dimworld>
  inline double ALU3dGrid<dim,dimworld>::communicateValue(double val) const
  {
#ifdef _ALU3DGRID_PARALLEL_
    //std::cout << "communicateValue " << val << " on proc " << mpAccess_.myrank() << " \n";
    double ret = mpAccess_.gmin(val);
    //std::cout << "got " << ret << " on proc " << mpAccess_.myrank() << " \n";
    return ret;
#else
    return val;
#endif
  }

  template <int dim, int dimworld>
  inline double ALU3dGrid<dim,dimworld>::communicateSum(double val) const
  {
#ifdef _ALU3DGRID_PARALLEL_
    //std::cout << "communicateValue " << val << " on proc " << mpAccess_.myrank() << " \n";
    double ret = mpAccess_.gsum(val);
    //std::cout << "got " << ret << " on proc " << mpAccess_.myrank() << " \n";
    return ret;
#else
    return val;
#endif
  }

  template <int dim, int dimworld>
  inline int ALU3dGrid<dim,dimworld>::communicateInt(int val) const
  {
#ifdef _ALU3DGRID_PARALLEL_
    //std::cout << "communicateInt " << val << " on proc " << mpAccess_.myrank() << " \n";
    int ret = mpAccess_.gmin(val);
    //std::cout << "got " << ret << " on proc " << mpAccess_.myrank() << " \n";
    return ret;
#else
    return val;
#endif
  }

  template <int dim, int dimworld>
  inline bool ALU3dGrid<dim,dimworld>::loadBalance()
  {
#ifdef _ALU3DGRID_PARALLEL_
    bool changed = myGrid().duneLoadBalance();
    if(changed)
    {
      calcMaxlevel();             // calculate new maxlevel
      calcExtras();               // reset size and things
    }
    return changed;
#else
    return false;
#endif
  }

  // adapt grid
  template <int dim, int dimworld> template <class DataCollectorType>
  inline bool ALU3dGrid<dim,dimworld>::loadBalance(DataCollectorType & dc)
  {
#ifdef _ALU3DGRID_PARALLEL_
    typedef ALU3dGridEntity<0,dim,GridImp> EntityType;
    EntityType en (*this);

    ALU3DSPACE GatherScatterImpl< ALU3dGrid<dim,dimworld> , EntityType ,
        DataCollectorType > gs(*this,en,dc);

    bool changed = myGrid().duneLoadBalance(gs);

    if(changed)
    {
      calcMaxlevel();             // calculate new maxlevel
      calcExtras();               // reset size and things
    }
    return changed;
#else
    return false;
#endif
  }

  // adapt grid
  template <int dim, int dimworld> template <class DataCollectorType>
  inline bool ALU3dGrid<dim,dimworld>::communicate(DataCollectorType & dc)
  {
#ifdef _ALU3DGRID_PARALLEL_
    typedef ALU3dGridEntity<0,dim,GridImp> EntityType;
    EntityType en (*this);
    ALU3DSPACE GatherScatterImpl< ALU3dGrid<dim,dimworld> , EntityType ,
        DataCollectorType > gs(*this,en,dc);

    myGrid().duneExchangeData(gs);
    return true;
#else
    return false;
#endif
  }

  template <int dim, int dimworld>
  template <FileFormatType ftype>
  inline bool ALU3dGrid<dim,dimworld>::
  writeGrid( const char * filename, alu3d_ctype time ) const
  {
    ALU3DSPACE GitterImplType & mygrd = const_cast<ALU3dGrid<dim,dimworld> &> (*this).myGrid();
    mygrd.duneBackup(filename);
    // write time and maxlevel
    {
      char *extraName = new char[strlen(filename)+20];
      if(!extraName)
      {
        std::cerr << "ALU3dGrid::writeGrid: couldn't allocate extraName! \n";
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
        std::cerr << "ALU3dGrid::writeGrid: couldn't open <" << extraName << ">! \n";
      }
      delete [] extraName;
    }
    return true;
  }

  template <int dim, int dimworld>
  template <FileFormatType ftype>
  inline bool ALU3dGrid<dim,dimworld>::
  readGrid( const char * filename, alu3d_ctype & time )
  {
    {
      assert(filename);
      char *macroName = new char[strlen(filename)+20];
      assert(macroName);

      sprintf(macroName,"%s.macro",filename);
      mygrid_ = new ALU3DSPACE GitterImplType (macroName
#ifdef _ALU3DGRID_PARALLEL_
                                               , mpAccess_
#endif
                                               );

      if(macroName) delete [] macroName;
    }

    assert(mygrid_ != 0);
    myGrid().duneRestore(filename);

    {
      char *extraName = new char[strlen(filename)+20];
      assert(extraName);

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
        std::cerr << "ALU3dGrid::readGrid: couldn't open <" << extraName << ">! \n";
      }
      if(extraName) delete [] extraName;
    }

    calcMaxlevel(); // calculate new maxlevel
    calcExtras();  // calculate indices

    return true;
  }

  // return Grid type
  template <int dim, int dimworld>
  inline GridIdentifier ALU3dGrid<dim,dimworld>::type () const
  {
    return ALU3dGrid_Id;
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
  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline ALU3dGridLevelIterator<codim,pitype,GridImp> ::
  ALU3dGridLevelIterator(const GridImp & grid, int level,bool end)
    : grid_(grid)
      , index_(-1)
      , level_(level)
  {
    if(!end)
    {
      IteratorType * it = new IteratorType ( grid_ , level_ );
      iter_.store( it );

      (*iter_).first();
      if(!(*iter_).done())
      {
        assert((*iter_).size() > 0);
        index_=0;
        EntityImp * obj = new EntityImp (grid_,level_);
        (*obj).setElement( (*iter_).item());
        objEntity_.store ( obj );
      }
    }
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline ALU3dGridLevelIterator<codim,pitype,GridImp> ::
  ALU3dGridLevelIterator(const GridImp & grid, const ALU3DSPACE HElementType &item)
    : grid_(grid)
      , index_(-1)
      , level_(item.level())
  {
    index_=0;
    EntityImp * obj = new EntityImp (grid_,level_);
    (*obj).setElement(const_cast<ALU3DSPACE HElementType &> (item));
    // objEntity deletes entity if no refCount is left
    objEntity_.store ( obj );

    IteratorType * it = new IteratorType ( grid_ , level_ );
    iter_.store( it );
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline void ALU3dGridLevelIterator<codim,pitype,GridImp> :: increment ()
  {
    assert(index_ >= 0);

    (*iter_).next();
    index_++;
    if ((*iter_).done())
    {
      index_ = -1;
      return ;
    }

    (*objEntity_).setElement( (*iter_).item() );

    return ;
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline bool ALU3dGridLevelIterator<codim,pitype,GridImp>::
  equals (const ALU3dGridLevelIterator<codim,pitype,GridImp>& i) const
  {
    return (index_ == i.index_);
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline typename ALU3dGridLevelIterator<codim,pitype,GridImp>::Entity &
  ALU3dGridLevelIterator<codim,pitype,GridImp>::dereference () const
  {
    return (*objEntity_);
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline int ALU3dGridLevelIterator<codim,pitype,GridImp>::level () const
  {
    return level_;
  }

  //*******************************************************************
  //
  //  LEAFITERATOR
  //
  //--LeafIterator
  //*******************************************************************
  template<class GridImp>
  inline ALU3dGridLeafIterator<GridImp> ::
  ALU3dGridLeafIterator(const GridImp &grid, int level,
                        bool end, PartitionIteratorType pitype)
    : grid_ ( grid )
      , index_(-1)
      , level_(level)
      , pitype_ (pitype)
  {
    if(!end)
    {
      IteratorType * it = new IteratorType ( grid_ , level_ );
      iter_.store( it );

      (*iter_).first();
      if(!(*iter_).done()) // else iterator empty
      {
        assert((*iter_).size() > 0);
        index_=0;
        EntityImp * obj = new EntityImp (grid,level_);
        (*obj).setElement( (*iter_).item());
        objEntity_.store ( obj );
      }
    }
  }

  template<class GridImp>
  inline void ALU3dGridLeafIterator<GridImp> :: increment ()
  {
    assert(index_  >= 0);

    (*iter_).next();
    index_++;

    if((*iter_).done())
    {
      index_ = -1;
      return ;
    }

    (*objEntity_).setElement( (*iter_).item() );
    return ;
  }

  template<class GridImp>
  inline bool ALU3dGridLeafIterator<GridImp> ::
  equals (const ALU3dGridLeafIteratorType & i) const
  {
    return (index_ == i.index_);
  }

  template<class GridImp>
  inline typename ALU3dGridLeafIterator<GridImp>::Entity &
  ALU3dGridLeafIterator<GridImp> :: dereference () const
  {
    assert( objEntity_.operator -> ());
    return (*objEntity_);
  }

  template<class GridImp>
  inline int ALU3dGridLeafIterator<GridImp> :: level() const
  {
    return level_;
  }

  //*******************************************************************
  //
  //  --EntityPointer
  //  --EnPointer
  //
  //*******************************************************************
  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline ALU3dGridEntityPointer<codim,pitype,GridImp> ::
  ALU3dGridEntityPointer(const GridImp & grid, const ALU3DSPACE HElementType &item)
    : grid_(grid) , entity_ ( grid_.entityProvider_.getNewObjectEntity( grid_, item.level()))
  {
    (*entity_).setElement( const_cast<ALU3DSPACE HElementType &> (item) );
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline ALU3dGridEntityPointer<codim,pitype,GridImp> ::
  ALU3dGridEntityPointer(const GridImp & grid)
    : grid_(grid) , entity_ ( grid_.entityProvider_.getNewObjectEntity( grid_, 0 ))
  {
    assert(false);
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline ALU3dGridEntityPointer<codim,pitype,GridImp> ::
  ~ALU3dGridEntityPointer()
  {
    grid_.entityProvider_.freeObjectEntity ( entity_ );
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline void ALU3dGridEntityPointer<codim,pitype,GridImp> :: increment ()
  {
    // do not increment EntityPointers
    assert(false);
    DUNE_THROW(ALU3dGridError,"Do not increment EntityPointers \n");
    return ;
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline bool ALU3dGridEntityPointer<codim,pitype,GridImp>::
  equals (const ALU3dGridEntityPointer<codim,pitype,GridImp>& i) const
  {
    return (entity_ == (i.entity_));
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline typename ALU3dGridEntityPointer<codim,pitype,GridImp>::Entity &
  ALU3dGridEntityPointer<codim,pitype,GridImp>::dereference () const
  {
    return (*entity_);
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline int ALU3dGridEntityPointer<codim,pitype,GridImp>::level () const
  {
    return (*entity_).level();
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
  template <class GridImp>
  inline ALU3dGridHierarchicIterator<GridImp> ::
  ALU3dGridHierarchicIterator(const GridImp & grid ,
                              const ALU3DSPACE HElementType & elem, int maxlevel ,bool end)
    : grid_(grid), elem_(elem) , item_(0) , maxlevel_(maxlevel)
      //, entity_ ( grid_.entityProvider_.getNewObjectEntity( grid_, maxlevel) )
  {
    if (!end)
    {
      item_ = const_cast<ALU3DSPACE HElementType *> (elem_.down());
      if(item_)
      {
        // we have children and they lie in the disired level range
        if(item_->level() <= maxlevel_)
        {
          //(*entity_).reset( maxlevel_ );
          //(*entity_).setElement(*item_);
          EntityImp * obj = new EntityImp (grid_,maxlevel_);
          (*obj).setElement(*item_);
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
  template <class GridImp>
  inline ALU3dGridHierarchicIterator<GridImp> ::
  ~ALU3dGridHierarchicIterator()
  {
    //grid_.entityProvider_.freeObjectEntity ( entity_ );
  }

  template <class GridImp>
  inline ALU3DSPACE HElementType * ALU3dGridHierarchicIterator<GridImp>::
  goNextElement(ALU3DSPACE HElementType * oldelem )
  {
    // strategy is:
    // - go down as far as possible and then over all children
    // - then go to father and next and down again

    ALU3DSPACE HElementType * nextelem = oldelem->down();
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

  template <class GridImp>
  inline void ALU3dGridHierarchicIterator<GridImp> :: increment ()
  {
    assert(item_   != 0);

    item_ = goNextElement( item_ );
    if(!item_) return ;

    //(*entity_).setElement(*item_);
    (*objEntity_).setElement(*item_);
    return ;
  }

  template <class GridImp>
  inline bool ALU3dGridHierarchicIterator<GridImp>::
  equals(const ALU3dGridHierarchicIterator<GridImp>& i) const
  {
    return item_==i.item_;
  }

  template <class GridImp>
  inline typename ALU3dGridHierarchicIterator<GridImp>::Entity &
  ALU3dGridHierarchicIterator<GridImp>::dereference () const
  {
    //return (*entity_);
    return (*objEntity_);
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
  template<class GridImp>
  inline ALU3dGridIntersectionIterator<GridImp> ::
  ALU3dGridIntersectionIterator(const GridImp & grid,
                                ALU3DSPACE HElementType *el, int wLevel,bool end)
    : grid_ ( grid )
      //, fEntity_ ( grid, wLevel )
      //, entity_( &fEntity_ )
      , entity_( grid_.entityProvider_.getNewObjectEntity( grid_ , wLevel ) )
      , item_(0), neigh_(0), ghost_(0)
      , index_(0) , numberInNeigh_ (-1)
      , theSituation_ (false) , daOtherSituation_ (false)
      , isBoundary_ (true) // isBoundary_ == true means no neighbour
      , isGhost_(false)
      , needSetup_ (true), needNormal_(true)
      , initInterGl_ (false)
      , interSelfGlobal_ (false)
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

  template<class GridImp>
  inline ALU3dGridIntersectionIterator<GridImp> :: ~ALU3dGridIntersectionIterator()
  {
    grid_.entityProvider_.freeObjectEntity( entity_ );
  }

  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> :: resetBools () const
  {
    needSetup_   = true;
    needNormal_  = true;
    initInterGl_ = false;
  }

  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> :: checkGhost () const
  {
#ifdef _ALU3DGRID_PARALLEL_
    isGhost_ = false;
    ghost_   = 0;
    if(isBoundary_)
    {
      typename ALU3DSPACE PLLBndFaceType * bnd =
        dynamic_cast<ALU3DSPACE PLLBndFaceType *> (item_->myneighbour(index_).first);
      if(bnd->bndtype() == ALU3DSPACE ProcessorBoundary_t)
      {
        isBoundary_ = false;
        isGhost_ = true;
      }
    }
#endif
  }

  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> ::
  first (ALU3DSPACE HElementType & elem, int wLevel)
  {
    item_  = static_cast<ALU3DSPACE GEOElementType *> (&elem);
    index_ = 0;
    neigh_ = 0;
    ghost_ = 0;

    neighpair_.first  = 0;
    neighpair_.second = 0;

    // if needed more than once we spare the virtual funtion call
    isBoundary_ = item_->myneighbour(index_).first->isboundary();
    checkGhost();

    theSituation_ = ( (elem.level() < wLevel ) && elem.leaf() );
    daOtherSituation_ = false;

    resetBools();
  }

  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> :: done ()
  {
    item_  = 0;
    index_ = 4;
  }

  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> :: increment ()
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
      return ;
    }

    // if faces are wrong oriented the assertion is thrown
    assert( item_->myneighbour(index_).first );

    // if needed more than once we spare the virtual funtion call
    isBoundary_ = item_->myneighbour(index_).first->isboundary();
    checkGhost();

    resetBools();
    return ;
  }

  template<class GridImp>
  inline bool ALU3dGridIntersectionIterator<GridImp> ::
  equals (const ALU3dGridIntersectionIterator<GridImp>& i) const
  {
    return (item_ == i.item_);
  }

  // set new neighbor
  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> ::
  setNeighbor () const
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

      if( theSituation_ && neighpair_.first->down())
      {
        neighpair_.first = neighpair_.first->down();
        daOtherSituation_ = true;
      }
      else
      {
        daOtherSituation_ = false;
      }
    }

#ifdef _ALU3DGRID_PARALLEL_
    if(isGhost_)
    {
      assert( item_->myneighbour(index_).first->isboundary() );

      ALU3DSPACE NeighbourPairType np = (neighpair_.second < 0) ?
                                        (neighpair_.first->nb.front()) : (neighpair_.first->nb.rear());


      ghost_ = dynamic_cast<ALU3DSPACE PLLBndFaceType *> (np.first);
      numberInNeigh_ = np.second;

      // if our level is smaller then the level of the real ghost then go one
      // level up and set the element
      if(ghost_->ghostLevel() != ghost_->level())
      {
        assert(ghost_->ghostLevel() < ghost_->level());
        assert(ghost_->up());

        if(daOtherSituation_)
        {
          neighpair_ = (*item_).myintersection(index_);
          daOtherSituation_ = false;
        }

        ghost_ = ghost_->up();
        assert(ghost_->level() == ghost_->ghostLevel());
      }

      assert( ghost_->getGhost() );

      //entity_.setGhost( *ghost_ ); // old method
      (*entity_).setGhost( *(ghost_->getGhost()) );

      needSetup_ = false;
      neigh_ = 0;
      return;
    }
#endif
    assert(!isGhost_);

    // same as in method myneighbour of Tetra and Hexa in gitter_sti.hh
    // neighpair_.second is the twist of the face
    ALU3DSPACE NeighbourPairType np = (neighpair_.second < 0) ?
                                      (neighpair_.first->nb.front()) : (neighpair_.first->nb.rear());

    neigh_ = static_cast<ALU3DSPACE GEOElementType *> (np.first);
    numberInNeigh_ = np.second;

    assert(neigh_ != item_);
    assert(neigh_ != 0);

    (*entity_).setElement(*neigh_);
    ghost_ = 0;
    needSetup_ = false;
  }

  template<class GridImp>
  inline typename ALU3dGridIntersectionIterator<GridImp>::Entity &
  ALU3dGridIntersectionIterator<GridImp>::dereference () const
  {
    if(needSetup_) setNeighbor();
    return (*entity_);
  }

  template<class GridImp>
  inline bool ALU3dGridIntersectionIterator<GridImp> :: boundary () const
  {
    return isBoundary_;
  }

  template<class GridImp>
  inline bool ALU3dGridIntersectionIterator<GridImp>::neighbor () const
  {
    return !(this->boundary());
  }

  template<class GridImp>
  inline int ALU3dGridIntersectionIterator<GridImp>::numberInSelf () const
  {
    return index_;
  }

  template<class GridImp>
  inline int ALU3dGridIntersectionIterator<GridImp>::numberInNeighbor () const
  {
    assert(item_ != 0);

    if(needSetup_) setNeighbor();
    return numberInNeigh_;
  }

  template<class GridImp>
  inline const typename ALU3dGridIntersectionIterator<GridImp>::NormalType &
  ALU3dGridIntersectionIterator<GridImp>::
  integrationOuterNormal(const FieldVector<alu3d_ctype, dim-1>& local) const
  {
    return this->outerNormal(local);
  }

  template<class GridImp>
  inline const typename ALU3dGridIntersectionIterator<GridImp>::NormalType &
  ALU3dGridIntersectionIterator<GridImp>::
  outerNormal(const FieldVector<alu3d_ctype, dim-1>& local) const
  {
    assert(item_ != 0);
    if(needNormal_)
    {
      // NOTE: &(outNormal_[0]) is a pointer to the inside vector
      // of the FieldVector class, we need this here, because
      // in ALU3dGrid we dont now the type FieldVector

      if( boundary() || ( !daOtherSituation_ ) )
      {
        // if boundary calc normal normal ;)
        item_->outerNormal(index_, &(outNormal_[0]) );
      }
      else
      {
        if(needSetup_) setNeighbor();

        if(neigh_)
        {
          neigh_->neighOuterNormal(numberInNeigh_, &(outNormal_[0]));
        }
        else
        {
          assert(ghost_);
          assert(ghost_->level() != item_->level());

          // ghostpair_.second stores the twist of the face
          item_->outerNormal(index_, &(outNormal_[0]));
          outNormal_ *= 0.25;
        }
      }
      needNormal_ = false;
    }
    return outNormal_;
  }

  template<class GridImp>
  inline const typename ALU3dGridIntersectionIterator<GridImp>::NormalType &
  ALU3dGridIntersectionIterator<GridImp>::
  unitOuterNormal(const FieldVector<alu3d_ctype, dim-1>& local) const
  {
    unitOuterNormal_  = this->outerNormal(local);
    unitOuterNormal_ *= (1.0/unitOuterNormal_.two_norm());
    return unitOuterNormal_;
  }

  template<class GridImp>
  inline const typename ALU3dGridIntersectionIterator<GridImp>::Geometry &
  ALU3dGridIntersectionIterator<GridImp>::intersectionGlobal () const
  {
    if(initInterGl_) return interSelfGlobal_;

    if( boundary() )
    {
      const ALU3DSPACE GEOFaceType & face = *(item_->myhface3(index_));
      initInterGl_ = interSelfGlobal_.buildGeom(face);
      return interSelfGlobal_;
    }

    // in case of neighbor
    if( needSetup_ ) setNeighbor();

    initInterGl_ = interSelfGlobal_.buildGeom( *(neighpair_.first) );
    return interSelfGlobal_;
  }

  template<class GridImp>
  inline const typename ALU3dGridIntersectionIterator<GridImp>::BoundaryEntity &
  ALU3dGridIntersectionIterator<GridImp>::boundaryEntity () const
  {
    assert(boundary());
    ALU3DSPACE BNDFaceType * bnd = dynamic_cast<ALU3DSPACE BNDFaceType *> (item_->myneighbour(index_).first);
    int id = bnd->bndtype(); // id's are positive
    //if(id == 2)
    //  std::cout << __MyRank__ << "=p: bndid = " << -id << "\n";
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
  template<int dim, class GridImp>
  inline ALU3dGridEntity<0,dim,GridImp> ::
  ALU3dGridEntity(const GridImp  &grid,
                  //ALU3DSPACE HElementType & element,int index,
                  int wLevel)
    : grid_(grid)
      , item_(0)
      , ghost_(0), isGhost_(false), geo_(false) , builtgeometry_(false)
      , walkLevel_ (wLevel)
      , glIndex_(-1), level_(-1)
      , geoInFather_ (false)
  {}

  template<int dim, class GridImp>
  inline void ALU3dGridEntity<0,dim,GridImp> ::
  reset (int walkLevel )
  {
    item_       = 0;
    ghost_      = 0;
    isGhost_    = false;
    builtgeometry_ = false;
    walkLevel_     = walkLevel;
    glIndex_    = -1;
    level_      = -1;
  }



  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp> :: setElement(ALU3DSPACE HElementType & element)
  {
    item_= static_cast<ALU3DSPACE IMPLElementType *> (&element);
    isGhost_ = false;
    ghost_ = 0;
    builtgeometry_=false;
    index_   = -1;
    level_   = (*item_).level();
    glIndex_ = (*item_).getIndex();
  }

  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp> :: setGhost(ALU3DSPACE HElementType & element)
  {
    item_= static_cast<ALU3DSPACE GEOElementType *> (&element);
    isGhost_ = true;
    ghost_ = 0;
    builtgeometry_=false;
    index_   = -1;
    level_   = (*item_).level();
    glIndex_ = (*item_).getIndex();
  }

  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp> :: setGhost(ALU3DSPACE PLLBndFaceType & ghost)
  {
    item_    = 0;
    ghost_   = &ghost;
    isGhost_ = true;
    index_   = -1;
    glIndex_ = ghost.getIndex();
    level_   = ghost.level();
    builtgeometry_ = false;
  }

  template<int dim, class GridImp>
  inline int
  ALU3dGridEntity<0,dim,GridImp> :: level() const
  {
    return level_;
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



  template<int dim, class GridImp>
  template<int cc>
  inline int ALU3dGridEntity<0,dim,GridImp> :: subIndex (int i) const
  {
    assert(cc == dim);
    assert(item_ != 0);
    return IndexWrapper<cc>::subIndex ( *item_ ,i);
  }

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

  template<int dim, class GridImp>
  template<int cc>
  inline typename ALU3dGridEntity<0,dim,GridImp> :: template codim<cc>:: EntityPointer
  ALU3dGridEntity<0,dim,GridImp> :: entity (int i) const
  {
    ALU3dGridEntityPointer<cc,All_Partition,GridImp> ep (grid_);
    return ep;
  }

  template<int dim, class GridImp>
  inline PartitionType ALU3dGridEntity<0,dim,GridImp> ::
  partitionType () const
  {
    return ((isGhost_) ?  GhostEntity : InteriorEntity);
  }

  template<int dim, class GridImp>
  inline bool ALU3dGridEntity<0,dim,GridImp> :: isLeaf() const
  {
    assert(item_ != 0);
    return (item_->down() == 0);
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
      ALU3dGridEntityPointer<0,All_Partition,GridImp> vati (grid_, static_cast<ALU3DSPACE HElementType &> (*item_));
      return vati;
    }

    ALU3dGridEntityPointer<0,All_Partition,GridImp> vati (grid_, static_cast<ALU3DSPACE HElementType &> (*(item_->up())));
    return vati;
  }

  // Adaptation methods
  template<int dim, class GridImp>
  inline bool ALU3dGridEntity<0,dim,GridImp> :: mark (int ref) const
  {
    // refine_element_t and coarse_element_t are defined in bsinclude.hh
    if(ghost_) return false;

    assert(item_ != 0);
    // mark for coarsening
    if(ref < 0)
    {
      if(level() <= 0) return false;
      if((*item_).requestrule() == ALU3DSPACE refine_element_t)
      {
        return false;
      }

      (*item_).request( ALU3DSPACE coarse_element_t );
      grid_.setCoarsenMark();
      return true;
    }

    // mark for refinement
    if(ref > 0)
    {
      (*item_).request( ALU3DSPACE refine_element_t );
      return true;
    }

    return false;
  }


  // Adaptation methods
  template<int dim, class GridImp>
  inline AdaptationState ALU3dGridEntity<0,dim,GridImp> :: state () const
  {
    assert(item_ != 0);
    if((*item_).requestrule() == ALU3DSPACE coarse_element_t)
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
  template <int cd, int dim, class GridImp>
  inline ALU3dGridEntity<cd,dim,GridImp> ::
  ALU3dGridEntity(const GridImp  &grid, int level)
    : grid_(grid), gIndex_(-1)
      , item_(0) , father_(0)
      , geo_(false) , builtgeometry_(false)
      , localFCoordCalced_ (false)
  {}

  template<int cd, int dim, class GridImp>
  inline void ALU3dGridEntity<cd,dim,GridImp> :: setElement(const BSElementType & item)
  {
    item_   = static_cast<const BSIMPLElementType *> (&item);
    gIndex_ = (*item_).getIndex();
    builtgeometry_=false;
    localFCoordCalced_ = false;
  }

  template<>
  inline void ALU3dGridEntity<3,3,const ALU3dGrid<3,3> > ::
  setElement(const ALU3DSPACE HElementType &el, const ALU3DSPACE VertexType &vx)
  {
    item_   = static_cast<const BSIMPLElementType *> (&vx);
    gIndex_ = (*item_).getIndex();
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
    assert(item_);
    return item_->level();
  }

  template<int cd, int dim, class GridImp>
  inline const typename ALU3dGridEntity<cd,dim,GridImp>::Geometry &
  ALU3dGridEntity<cd,dim,GridImp>:: geometry() const
  {
    if(!builtgeometry_) builtgeometry_ = geo_.buildGeom(item_);
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

  /***********************************************************************
  ######  #       ######  #    #  ######  #    #   #####
  #       #       #       ##  ##  #       ##   #     #
  #####   #       #####   # ## #  #####   # #  #     #
  #       #       #       #    #  #       #  # #     #
  #       #       #       #    #  #       #   ##     #
  ######  ######  ######  #    #  ######  #    #     #
  ***********************************************************************/
  // --Geometry
  template<int mydim, int cdim, class GridImp>
  inline ALU3dGridGeometry<mydim,cdim,GridImp> :: ALU3dGridGeometry(bool makeRefElement)
    : builtinverse_ (false) , builtA_ (false) , builtDetDF_ (false)
  {
    // create reference element
    if(makeRefElement)
    {
      coord_ = 0.0;
      for(int i=1; i<mydim+1; i++)
        coord_[i][i-1] = 1.0;
    }
  }

  //   B U I L T G E O M   - - -

  template<int mydim, int cdim, class GridImp>
  inline void ALU3dGridGeometry<mydim,cdim,GridImp> :: calcElMatrix () const
  {
    if(!builtA_)
    {
      // creat Matrix A (=Df)               INDIZES: col/row
      // Mapping: R^dim -> R^3,  F(x) = A x + p_0
      // columns:    p_1 - p_0  |  p_2 - p_0  |  p_3 - p_0

      for (int i=0; i<mydim; i++)
      {
        //FieldVector<alu3d_ctype,cdim> & row = const_cast<FieldMatrix<alu3d_ctype,matdim,matdim> &> (A_)[i];
        //row = coord_[i+1] - coord_[0];
      }
      builtA_ = true;
    }
  }

  // matrix for mapping from reference element to current element
  template<>
  inline void ALU3dGridGeometry<3,3, const ALU3dGrid<3,3> > :: calcElMatrix () const
  {
    if(!builtA_)
    {
      enum { mydim = 3 };
      // creat Matrix A (=Df)               INDIZES: col/row
      // Mapping: R^dim -> R^3,  F(x) = A x + p_0
      // columns:    p_1 - p_0  |  p_2 - p_0  |  p_3 - p_0

      const FieldVector<alu3d_ctype,mydim> & coord0 = coord_[0];
      for (int i=0; i<mydim; i++)
      {
        A_[i][0] = coord_[1][i] - coord0[i];
        A_[i][1] = coord_[2][i] - coord0[i];
        A_[i][2] = coord_[3][i] - coord0[i];
      }
      builtA_ = true;
    }
  }

  //dim = dimworld = 3
  template<int mydim, int cdim, class GridImp>
  inline void ALU3dGridGeometry<mydim,cdim,GridImp> :: buildJacobianInverse() const
  {
    if(!builtinverse_)
    {
      calcElMatrix();

      // DetDf = integrationElement
      detDF_ = std::abs( FMatrixHelp::invertMatrix(A_,Jinv_) );
      builtinverse_ = builtDetDF_ = true;
    }
  }

  template<> //dim = 2 , dimworld = 3
  inline void ALU3dGridGeometry<2,3, const ALU3dGrid<3,3> > :: buildJacobianInverse() const
  {
    if(!builtinverse_)
    {
      enum { dim = 3 };

      //std::cerr << "WARNING: ALU3dGridGeometry::buildJacobianInverse not tested yet! " << __LINE__ <<"\n";
      // create vectors of face
      tmpV_ = coord_[1] - coord_[0];
      tmpU_ = coord_[2] - coord_[1];

      // calculate scaled outer normal
      for(int i=0; i<dim; i++)
      {
        globalCoord_[i] = (  tmpU_[(i+1)%dim] * tmpV_[(i+2)%dim]
                             - tmpU_[(i+2)%dim] * tmpV_[(i+1)%dim] );
      }

      detDF_ = std::abs ( globalCoord_.two_norm() );
      builtinverse_ = builtDetDF_ = true;
    }
  }

  template<> //dim = 1 , dimworld = 3
  inline void ALU3dGridGeometry<1,3, const ALU3dGrid<3,3> > :: buildJacobianInverse() const
  {
    if(!builtinverse_)
    {
      enum { dim = 3 };
      //std::cerr << "WARNING: ALU3dGridGeometry::buildJacobianInverse not tested yet! " << __LINE__ <<"\n";
      // create vectors of face
      globalCoord_ = coord_[1] - coord_[0];
      detDF_ = std::abs ( globalCoord_.two_norm() );
      builtinverse_ = builtDetDF_ = true;
    }
  }

  template<> //dim = 1 , dimworld = 3
  inline void ALU3dGridGeometry<0,3, const ALU3dGrid<3,3> > :: buildJacobianInverse() const
  {
    if(!builtinverse_)
    {
      enum { dim = 3 };
      detDF_ = 1.0;
      builtinverse_ = builtDetDF_ = true;
    }
  }

  template <>
  inline bool ALU3dGridGeometry<3,3, const ALU3dGrid<3,3> > ::
  buildGeom(const ALU3DSPACE IMPLElementType & item)
  {
    enum { dim = 3 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    for (int i=0; i<(dim+1); i++)
    {
      const double (&p)[3] = item.myvertex(i)->Point();
      for (int j=0; j<dimworld; j++)
      {
        coord_[i][j] = p[j];
      }
    }
    return true;
  }

  template <>
  inline bool ALU3dGridGeometry<3,3, const ALU3dGrid<3,3> > :: buildGhost(const ALU3DSPACE PLLBndFaceType & ghost)
  {
    enum { dim = 3 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    ALU3DSPACE GEOFaceType & face = dynamic_cast<ALU3DSPACE GEOFaceType &> (*(ghost.myhface3(0)));

    // here apply the negative twist, because the twist is from the
    // neighbouring elements point of view which is outside of the ghost
    // element
    const int map[3] = { (ghost.twist(0) < 0) ? 2 : 0 , 1 , (ghost.twist(0) < 0) ? 0 : 2 };

    for (int i=0; i<dim; i++) // col is the point vector
    {
      const double (&p)[3] = face.myvertex(map[i])->Point();
      for (int j=0; j<dimworld; j++) // row is the coordinate of the point
      {
        coord_[i][j] = p[j];
      }
    }

    {
      const double (&p)[3] = ghost.oppositeVertex(0);
      for (int j=0; j<dimworld; j++)
      {
        coord_[3][j] = p[j];
      }
    }

    return true;
  }

  template <>
  inline bool ALU3dGridGeometry<2,3, const ALU3dGrid<3,3> > :: buildGeom(const ALU3DSPACE HFaceType & item)
  {
    enum { dim = 2 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    for (int i=0; i<(dim+1); i++)
    {
      const double (&p)[3] = static_cast<const ALU3DSPACE GEOFaceType &> (item).myvertex(i)->Point();
      for (int j=0; j<dimworld; j++)
      {
        coord_[i][j] = p[j];
      }
    }

    buildJacobianInverse();
    return true;
  }

  template <> // for edges
  inline bool ALU3dGridGeometry<1,3, const ALU3dGrid<3,3> > :: buildGeom(const ALU3DSPACE HEdgeType & item)
  {
    enum { dim = 1 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    for (int i=0; i<(dim+1); i++)
    {
      const double (&p)[3] = static_cast<const ALU3DSPACE GEOEdgeType &> (item).myvertex(i)->Point();
      for (int j=0; j<dimworld; j++)
      {
        coord_[i][j] = p[j];
      }
    }

    buildJacobianInverse();
    return true;
  }

  template <> // for Vertices ,i.e. Points
  inline bool ALU3dGridGeometry<0,3, const ALU3dGrid<3,3> > :: buildGeom(const ALU3DSPACE VertexType & item)
  {
    enum { dim = 0 };
    enum { dimworld = 3};

    builtinverse_ = builtA_ = builtDetDF_ = false;

    const double (&p)[3] = static_cast<const ALU3DSPACE GEOVertexType &> (item).Point();
    for (int j=0; j<dimworld; j++) coord_[0][j] = p[j];

    buildJacobianInverse();
    return true;
  }


  template <GeometryType eltype , int dim> struct ALU3dGridElType {
    static GeometryType type () { return unknown; }
  };
  template <GeometryType eltype> struct ALU3dGridElType<eltype,3> {
    static GeometryType type () { return tetrahedron; }
  };
  template <GeometryType eltype> struct ALU3dGridElType<eltype,2> {
    static GeometryType type () { return triangle; }
  };
  template <GeometryType eltype> struct ALU3dGridElType<eltype,1> {
    static GeometryType type () { return line; }
  };
  template <GeometryType eltype> struct ALU3dGridElType<eltype,0> {
    static GeometryType type () { return vertex; }
  };

  template<int mydim, int cdim, class GridImp>
  inline GeometryType ALU3dGridGeometry<mydim,cdim,GridImp> ::type () const
  {
    return ALU3dGridElType<tetrahedron,mydim>::type();
  }

  template<int mydim, int cdim, class GridImp>
  inline int ALU3dGridGeometry<mydim,cdim,GridImp> ::corners () const
  {
    return dimbary;
  }

  template<int mydim, int cdim, class GridImp>
  inline const FieldVector<alu3d_ctype, cdim>&
  ALU3dGridGeometry<mydim,cdim,GridImp> :: operator[] (int i) const
  {
    assert((i>=0) && (i < mydim+1));
    return coord_[i];
  }

  template<int mydim, int cdim, class GridImp>
  inline FieldVector<alu3d_ctype, cdim>&
  ALU3dGridGeometry<mydim,cdim,GridImp> :: getCoordVec (int i)
  {
    assert((i>=0) && (i < mydim+1));
    return coord_[i];
  }


  //   G L O B A L   - - -

  // dim = 1,2,3 dimworld = 3
  template<int mydim, int cdim, class GridImp>
  inline FieldVector<alu3d_ctype, cdim> ALU3dGridGeometry<mydim,cdim,GridImp>::
  global(const FieldVector<alu3d_ctype, mydim>& local) const
  {
    calcElMatrix();

    globalCoord_ = coord_[0];
    A_.umv(local,globalCoord_);
    return globalCoord_;
  }

  template<>
  inline FieldVector<alu3d_ctype, 3> ALU3dGridGeometry<3,3, const ALU3dGrid<3,3> >::
  global(const FieldVector<alu3d_ctype, 3> & local) const
  {
    calcElMatrix();

    globalCoord_ = coord_[0];
    A_.umv(local,globalCoord_);
    return globalCoord_;
  }

  template<> // dim = dimworld = 3
  inline FieldVector<alu3d_ctype, 3>
  ALU3dGridGeometry<3,3,const ALU3dGrid<3,3> > ::
  local(const FieldVector<alu3d_ctype, 3>& global) const
  {
    if (!builtinverse_) buildJacobianInverse();
    enum { dim = 3 };
    for(int i=0; i<dim; i++)
      globalCoord_[i] = global[i] - coord_[0][i];

    FMatrixHelp::multAssign(Jinv_,globalCoord_,localCoord_);
    return localCoord_;
  }

  template<int mydim, int cdim, class GridImp>
  inline bool ALU3dGridGeometry<mydim,cdim,GridImp> ::
  checkInside(const FieldVector<alu3d_ctype, mydim>& local) const
  {
    alu3d_ctype sum = 0.0;

    for(int i=0; i<mydim; i++)
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

  template<int mydim, int cdim, class GridImp>
  inline alu3d_ctype
  ALU3dGridGeometry<mydim,cdim,GridImp> ::integrationElement (const FieldVector<alu3d_ctype, mydim>& local) const
  {
    if(builtDetDF_)
      return detDF_;

    calcElMatrix();

    detDF_ = A_.determinant();

    assert(detDF_ > 0.0);

    builtDetDF_ = true;
    return detDF_;
  }

  //  J A C O B I A N _ I N V E R S E  - - -

  template<> // dim = dimworld = 3
  inline const FieldMatrix<alu3d_ctype,3,3> &
  ALU3dGridGeometry<3,3, const ALU3dGrid<3,3> >:: jacobianInverse (const FieldVector<alu3d_ctype, 3>& local) const
  {
    if (!builtinverse_) buildJacobianInverse();
    return Jinv_;
  }

  // print the ElementInformation
  template<int mydim, int cdim, class GridImp>
  inline void ALU3dGridGeometry<mydim,cdim,GridImp>::print (std::ostream& ss) const
  {
    ss << "ALU3dGridGeometry<" << mydim << "," << cdim << "> = {\n";
    for(int i=0; i<corners(); i++)
    {
      ss << " corner " << i << " ";
      ss << "{" << ((*this)[i]) << "}"; ss << std::endl;
    }
    ss << "} \n";
  }


  //**********************************************************
  //  Reference Element
  //**********************************************************
  template <class GridImp, int dim> struct ALU3dGridRefElem;
  template <class GridImp> struct ALU3dGridRefElem<GridImp,1> {
    static const Dune::Geometry<1,1,GridImp,Dune::ALU3dGridGeometry> & refelem ()
    {
      static ALU3dGridReferenceGeometry<1,GridImp> ref;
      return ref.refelem;
    }
  };

  template <class GridImp> struct ALU3dGridRefElem<GridImp,2> {
    static const Dune::Geometry<2,2,GridImp,Dune::ALU3dGridGeometry> & refelem ()
    {
      static ALU3dGridReferenceGeometry<2,GridImp> ref;
      return ref.refelem;
    }
  };
  template <class GridImp> struct ALU3dGridRefElem<GridImp,3> {
    static const Dune::Geometry<3,3,GridImp,Dune::ALU3dGridGeometry> & refelem ()
    {
      static ALU3dGridReferenceGeometry<3,GridImp> ref;
      return ref.refelem;
    }
  };

  template<int mydim, int cdim, class GridImp>
  inline const Dune::Geometry<mydim,mydim,GridImp,Dune::ALU3dGridGeometry> &
  ALU3dGridGeometry<mydim,cdim,GridImp>:: refelem () {
    return ALU3dGridRefElem<GridImp,mydim>::refelem();
  }

}

#endif
