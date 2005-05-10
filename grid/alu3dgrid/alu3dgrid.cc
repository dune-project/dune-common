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
      , coarsenMarked_(0) , refineMarked_(0)
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
    //loadBalance();
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
      , coarsenMarked_(0) , refineMarked_(0)
      , mpAccess_(mpiComm) , myRank_( mpAccess_.myrank() )
      , hIndexSet_ (*this,globalSize_)
      , levelIndexSet_(0)
  {}
#else
  template <int dim, int dimworld>
  inline ALU3dGrid<dim,dimworld>::ALU3dGrid(int myrank)
    : mygrid_ (0) , maxlevel_(0)
      , coarsenMarked_(0) , refineMarked_(0)
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
      , coarsenMarked_(0) , refineMarked_(0)
      , myRank_(-1)
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
    assert(mygrid_);
    maxlevel_ = 0;
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
      globalSize_[i] = (*mygrid_).indexManager(i).getMaxIndex();
    }

    //dverb << "proc " << mpAccess_.myrank() << " num el = " << globalSize_[0] << "\n";
    if(levelIndexSet_) (*levelIndexSet_).calcNewIndex();

    coarsenMarked_ = 0;
    refineMarked_  = 0;
  }

  template <int dim, int dimworld>
  inline int ALU3dGrid<dim,dimworld>::global_size(int codim) const
  {
    assert(globalSize_[codim] >= 0);
    //dverb << globalSize_[codim] << " Size of cd " << codim << "\n";
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
    return ALU3dGridLevelIterator<cd,pitype,const MyType> (*this,level);
  }

  template <int dim, int dimworld>
  template <int cd, PartitionIteratorType pitype>
  inline typename ALU3dGrid<dim,dimworld>::Traits::template codim<cd>::template partition<pitype>::LevelIterator
  ALU3dGrid<dim,dimworld>::lend(int level) const {
    return ALU3dGridLevelIterator<cd,pitype,const MyType> (*this,level,true);
  }

  // lbegin methods
  template <int dim, int dimworld>
  template <int cd>
  inline typename ALU3dGrid<dim,dimworld>::Traits::template codim<cd>::template partition<All_Partition>::LevelIterator
  ALU3dGrid<dim,dimworld>::lbegin(int level) const {
    return ALU3dGridLevelIterator<cd,All_Partition,const MyType> (*this,level);
  }

  template <int dim, int dimworld>
  template <int cd>
  inline typename ALU3dGrid<dim,dimworld>::Traits::template codim<cd>::template partition<All_Partition>::LevelIterator
  ALU3dGrid<dim,dimworld>::lend(int level) const {
    return ALU3dGridLevelIterator<cd,All_Partition,const MyType> (*this,level,true);
  }

  // leaf methods
  template <int dim, int dimworld>
  inline typename ALU3dGrid<dim,dimworld>::LeafIteratorType
  ALU3dGrid<dim,dimworld>::leafbegin(int level, PartitionIteratorType pitype) const
  {
    return ALU3dGridLeafIterator<const MyType> ((*this),level,false,pitype);
  }
  template <int dim, int dimworld>
  inline typename ALU3dGrid<dim,dimworld>::LeafIteratorType
  ALU3dGrid<dim,dimworld>::leafend(int level, PartitionIteratorType pitype) const
  {
    return ALU3dGridLeafIterator<const MyType> ((*this),level,true,pitype);
  }

  // global refine
  template <int dim, int dimworld>
  inline bool ALU3dGrid<dim,dimworld>::
  mark(int ref, typename Traits::template codim<0>::EntityPointer & ep )
  {
    return this->mark(ref,*ep);
  }

  // global refine
  template <int dim, int dimworld>
  inline bool ALU3dGrid<dim,dimworld>::
  mark(int ref, const typename Traits::template codim<0>::Entity & ep )
  {
    bool marked = (this->template getRealEntity<0> (ep)).mark(ref);
    if(marked)
    {
      if(ref > 0) refineMarked_ ++ ;
      if(ref < 0) coarsenMarked_ ++ ;
    }
    return marked;
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

  // preprocess grid
  template <int dim, int dimworld>
  inline bool ALU3dGrid<dim,dimworld>::preAdapt()
  {
    return (coarsenMarked_ > 0);
  }

  // adapt grid
  template <int dim, int dimworld>
  inline bool ALU3dGrid<dim,dimworld>::adapt()
  {
#ifdef _ALU3DGRID_PARALLEL_
    bool ref = myGrid().dAdapt(); // adapt grid
#else
    bool ref = myGrid().adapt(); // adapt grid
#endif
    if(ref)
    {
      //maxlevel_++;
      calcMaxlevel();           // calculate new maxlevel
      calcExtras();               // reset size and things
    }
    return ref;
  }

  // adapt grid
  template <int dim, int dimworld>
  template <class DofManagerType, class RestrictProlongOperatorType>
  inline bool ALU3dGrid<dim,dimworld>::
  adapt(DofManagerType & dm, RestrictProlongOperatorType & rpo, bool verbose )
  {
    assert( ((verbose) ? (dverb << "ALU3dGrid :: adapt() new method called!\n", 1) : 1 ) );
    EntityImp f ( *this, this->maxlevel() );
    EntityImp s ( *this, this->maxlevel() );

    int defaultChunk = newElementsChunk_;
    int actChunk     = refineEstimate_ * refineMarked_;

    // guess how many new elements we get
    int newElements = std::max( actChunk , defaultChunk );
    ALU3DSPACE AdaptRestrictProlongImpl<ALU3dGrid<dim,dimworld>, EntityImp, DofManagerType, RestrictProlongOperatorType >
    rp(*this,f,s,dm,rpo, newElements);

    dm.resizeMem( newElements );
    bool ref = myGrid().duneAdapt(rp); // adapt grid

    // if new maxlevel was claculated
    if(rp.maxlevel() >= 0) maxlevel_ = rp.maxlevel();
    assert( ((verbose) ? (dverb << "maxlevel = " << maxlevel_ << "!\n", 1) : 1 ) );

    if(ref)
    {
      calcMaxlevel();
      calcExtras(); // reset size and things
    }

    // check whether we have balance
    loadBalance(dm);
    dm.dofCompress();
    communicate(dm);

    postAdapt();
    assert( ((verbose) ? (dverb << "ALU3dGrid :: adapt() new method finished!\n", 1) : 1 ) );
    return ref;
  }


  // post process grid
  template <int dim, int dimworld>
  inline void ALU3dGrid<dim,dimworld>::postAdapt()
  {
#ifdef _ALU3DGRID_PARALLEL_
    if(mpAccess_.nlinks() < 1)
#endif
    {
      maxlevel_ = 0;
      ALU3DSPACE BSLeafIteratorMaxLevel w ( myGrid() ) ;
      for (w->first () ; ! w->done () ; w->next ())
      {
        if(w->item().level() > maxlevel_ ) maxlevel_ = w->item().level();
        w->item ().resetRefinedTag();
      }
    }
#ifdef _ALU3DGRID_PARALLEL_
    else
    {
      // we have to walk over all hierarchcy because during loadBalance
      // we get newly refined elements, which have to be cleared
      int fakeLevel = maxlevel_;
      maxlevel_ = 0;
      for(int l=0; l<= fakeLevel; l++)
      {
        {
          ALU3DSPACE ALU3dGridLevelIteratorWrapper<0> w ( *this, l ) ;
          for (w.first () ; ! w.done () ; w.next ())
          {
            if(w.item().level() > maxlevel_ ) maxlevel_ = w.item().level();
            w.item ().resetRefinedTag();
          }
        }
      }

      ALU3DSPACE BSLeafIteratorMaxLevel w ( myGrid() ) ;
      for (w->first () ; ! w->done () ; w->next ())
      {
        if(w->item().level() > maxlevel_ ) maxlevel_ = w->item().level();
        w->item ().resetRefinedTag();
      }
    }
#endif
  }

  template <int dim, int dimworld> template <class T>
  inline T ALU3dGrid<dim,dimworld>::globalMin(T val) const
  {
#ifdef _ALU3DGRID_PARALLEL_
    T ret = mpAccess_.gmin(val);
    return ret;
#else
    return val;
#endif
  }
  template <int dim, int dimworld> template <class T>
  inline T ALU3dGrid<dim,dimworld>::globalMax(T val) const
  {
#ifdef _ALU3DGRID_PARALLEL_
    T ret = mpAccess_.gmax(val);
    return ret;
#else
    return val;
#endif
  }
  template <int dim, int dimworld> template <class T>
  inline T ALU3dGrid<dim,dimworld>::globalSum(T val) const
  {
#ifdef _ALU3DGRID_PARALLEL_
    T sum = mpAccess_.gsum(val);
    return sum;
#else
    return val;
#endif
  }
  template <int dim, int dimworld> template <class T>
  inline void ALU3dGrid<dim,dimworld>::globalSum(T * send, int s , T * recv) const
  {
#ifdef _ALU3DGRID_PARALLEL_
    mpAccess_.gsum(send,s,recv);
    return ;
#else
    std::memcpy(recv,send,s*sizeof(T));
    return ;
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
    EntityImp en ( *this, this->maxlevel() );

    ALU3DSPACE GatherScatterImpl< ALU3dGrid<dim,dimworld>, EntityImp, DataCollectorType >
    gs(*this,en,dc);

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
    EntityImp en ( *this, this->maxlevel() );

    ALU3DSPACE GatherScatterImpl< ALU3dGrid<dim,dimworld> , EntityImp ,
        DataCollectorType > gs(*this,en,dc);

    myGrid().duneExchangeData(gs);
    return true;
#else
    return false;
#endif
  }

  template <int dim, int dimworld>
  template <GrapeIOFileFormatType ftype>
  inline bool ALU3dGrid<dim,dimworld>::
  writeGrid( const std::basic_string<char> filename, alu3d_ctype time ) const
  {
    ALU3DSPACE GitterImplType & mygrd = const_cast<ALU3dGrid<dim,dimworld> &> (*this).myGrid();
    mygrd.duneBackup(filename.c_str());
    // write time and maxlevel
    {
      typedef std::ostringstream StreamType;
      StreamType eName;

      eName << filename;
      eName << ".extra";
      const char * extraName = eName.str().c_str();
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
    }
    return true;
  }

  template <int dim, int dimworld>
  template <GrapeIOFileFormatType ftype>
  inline bool ALU3dGrid<dim,dimworld>::
  readGrid( const std::basic_string<char> filename, alu3d_ctype & time )
  {
    {
      typedef std::ostringstream StreamType;
      StreamType mName;

      mName << filename;
      mName << ".macro";
      const char * macroName = mName.str().c_str();

      { //check if file exists
        std::ifstream check ( macroName );
        if( !check )
          DUNE_THROW(ALU3dGridError,"cannot read file " << macroName << "\n");
      }

      mygrid_ = new ALU3DSPACE GitterImplType (macroName
#ifdef _ALU3DGRID_PARALLEL_
                                               , mpAccess_
#endif
                                               );
    }

    assert(mygrid_ != 0);
    myGrid().duneRestore(filename.c_str());

    {
      typedef std::ostringstream StreamType;
      StreamType eName;

      eName << filename;
      eName << ".extra";
      const char * extraName = eName.str().c_str();
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


  template <class GridImp, int codim>
  struct ALU3dGridEntityFactory
  {
    typedef ALU3dGridMakeableEntity<codim,GridImp::dimension,const GridImp> EntityImp;
    template <class EntityProviderType>
    static ALU3dGridMakeableEntity<codim,GridImp::dimension,const GridImp> *
    getNewEntity (const GridImp & grid, EntityProviderType & ep, int level)
    {
      return new EntityImp( grid, level );
    }

    template <class EntityProviderType>
    static void freeEntity( EntityProviderType & ep, EntityImp * e )
    {
      if( e ) delete e;
    }
  };

  template <class GridImp>
  struct ALU3dGridEntityFactory<GridImp,0>
  {
    typedef ALU3dGridMakeableEntity<0,GridImp::dimension,const GridImp> EntityImp;
    template <class EntityProviderType>
    static ALU3dGridMakeableEntity<0,GridImp::dimension,const GridImp> *
    getNewEntity (const GridImp & grid, EntityProviderType & ep, int level)
    {
      return ep.getNewObjectEntity( grid, level);
    }

    template <class EntityProviderType>
    static void freeEntity( EntityProviderType & ep, EntityImp * e )
    {
      ep.freeObjectEntity( e );
    }
  };

  // return Grid type
  template <int dim, int dimworld> template <int cd>
  inline ALU3dGridMakeableEntity<cd,dim,const ALU3dGrid<dim,dimworld> > *
  ALU3dGrid<dim,dimworld>::getNewEntity (int level) const
  {
    return ALU3dGridEntityFactory<MyType,cd>::getNewEntity(*this,entityProvider_,level);
  }

  template <int dim, int dimworld> template <int cd>
  inline void ALU3dGrid<dim,dimworld>::
  freeEntity (ALU3dGridMakeableEntity<cd,dim,const ALU3dGrid<dim,dimworld> > * e) const
  {
    return ALU3dGridEntityFactory<MyType,cd>::freeEntity(entityProvider_, e);
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
    : ALU3dGridEntityPointer<codim,GridImp> ( grid ,level,end)
      , index_(-1)
      , level_(level)
  {
    if(!end)
    {
      IteratorType * it = new IteratorType ( this->grid_ , level_ );
      iter_.store( it );

      (*iter_).first();
      if(!(*iter_).done())
      {
        assert((*iter_).size() > 0);
        index_=0;
        (*(this->entity_)).setElement( (*iter_).item());
      }
    }
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline ALU3dGridLevelIterator<codim,pitype,GridImp> ::
  ALU3dGridLevelIterator(const ALU3dGridLevelIterator<codim,pitype,GridImp> & org )
    : ALU3dGridEntityPointer<codim,GridImp> ( org.grid_,org.level(),(org.index_ < 0) ? true : false )
      , index_( org.index_ )
      , level_( org.level_ )
      , iter_ ( org.iter_ )
  {
    if(index_ >= 0)
    {
      (*(this->entity_)).setElement( (*iter_).item());
    }
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline void ALU3dGridLevelIterator<codim,pitype,GridImp> :: increment ()
  {
    // if assertion is thrown then end iterator was forgotten or didnt stop
    assert(index_ >= 0);

    (*iter_).next();
    index_++;
    if ((*iter_).done())
    {
      index_ = -1;
      this->done();
      return ;
    }

    (*(this->entity_)).setElement( (*iter_).item());
    return ;
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
    : ALU3dGridEntityPointer <0,GridImp> ( grid,level,end)
      , index_(-1)
      , level_(level)
      , pitype_ (pitype)
  {
    if(!end)
    {
      IteratorType * it = new IteratorType ( this->grid_ , level_ );
      iter_.store( it );

      (*iter_).first();
      if(!(*iter_).done()) // else iterator empty
      {
        assert((*iter_).size() > 0);
        index_=0;
        (*(this->entity_)).setElement( (*iter_).item());
      }
    }
  }

  template<class GridImp>
  inline ALU3dGridLeafIterator<GridImp> ::
  ALU3dGridLeafIterator(const ALU3dGridLeafIterator<GridImp> &org)
    : ALU3dGridEntityPointer <0,GridImp> ( org.grid_,org.level(),(org.index_ < 0) ? true : false )
      , index_(org.index_)
      , level_(org.level_)
      , iter_ ( org.iter_ )
      , pitype_(org.pitype_)
  {
    if(index_ >= 0)
    {
      (*(this->entity_)).setElement( iter_->item() );
    }
  }

  template<class GridImp>
  inline void ALU3dGridLeafIterator<GridImp> :: increment ()
  {
    // if assertion is thrown then end iterator was forgotten or didnt stop
    assert(index_  >= 0);

    (*iter_).next();
    index_++;

    if((*iter_).done())
    {
      index_ = -1;
      this->done();
      return ;
    }

    (*(this->entity_)).setElement( (*iter_).item());
    return ;
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
    : ALU3dGridEntityPointer<0,GridImp> ( grid, elem )
      , elem_(elem) , item_(0) , maxlevel_(maxlevel)
  {
    if (!end)
    {
      item_ = const_cast<ALU3DSPACE HElementType *> (this->elem_.down());
      if(item_)
      {
        // we have children and they lie in the disired level range
        if(item_->level() <= maxlevel_)
        {
          (*(this->entity_)).reset( maxlevel_ );
          (*(this->entity_)).setElement(*item_);
        }
        else
        { // otherwise do nothing
          item_ = 0;
        }
      }
      else
      {
        this->done();
      }
    }
  }

  template <class GridImp>
  inline ALU3dGridHierarchicIterator<GridImp> ::
  ALU3dGridHierarchicIterator(const ALU3dGridHierarchicIterator<GridImp> & org)
    : ALU3dGridEntityPointer<0,GridImp> (org.grid_,org.elem_)
      , elem_ (org.elem_) , item_(org.item_) , maxlevel_(org.maxlevel_)
  {
    if(item_)
      (*(this->entity_)).setElement(*item_);
    else
      this->done();
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
    if(!item_)
    {
      this->done();
      return ;
    }

    (*(this->entity_)).setElement(*item_);
    return ;
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
                                ALU3DSPACE HElementType * el, int wLevel,bool end)
    : ALU3dGridEntityPointer<0,GridImp> ( grid , wLevel, end )
  {
    if( !end )
    {
      walkLevel_       = wLevel;
      numberInNeigh_   = -1;
      interSelfGlobal_ = (!end) ? this->grid_.geometryProvider_.getNewObjectEntity( this->grid_ ,wLevel ) : 0;
      bndEntity_       = (!end) ? this->grid_.bndProvider_.getNewObjectEntity( this->grid_ , walkLevel_ ) : 0;
      first(*el,wLevel);
    }
    else
    {
      last();
    }
  }

  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> :: resetBools () const
  {
    needSetup_   = true;
    needNormal_  = true;
    initInterGl_ = false;
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
  inline void ALU3dGridIntersectionIterator<GridImp> :: last ()
  {
    // reset entity pointer for equality
    ALU3dGridEntityPointer<0,GridImp>::done();

    interSelfGlobal_ = 0;
    bndEntity_ = 0;
    item_      = 0;
    index_     = 4;
  }

  // copy constructor
  template<class GridImp>
  inline ALU3dGridIntersectionIterator<GridImp> ::
  ALU3dGridIntersectionIterator(const ALU3dGridIntersectionIterator<GridImp> & org)
    : ALU3dGridEntityPointer<0,GridImp> (org.grid_ , org.walkLevel_ , (org.item_) ? false : true )
  {
    if(org.item_) // else its a end iterator
    {
      walkLevel_       = org.walkLevel_;
      item_            = org.item_;
      neigh_           = org.neigh_;
      ghost_           = org.ghost_;
      index_           = org.index_;
      numberInNeigh_   = org.numberInNeigh_;
      theSituation_    = org.theSituation_;
      daOtherSituation_= org.daOtherSituation_;
      isBoundary_      = org.isBoundary_; // isBoundary_ == true means no neighbour
      isGhost_         = org.isGhost_;
      needSetup_       = true;
      needNormal_      = true;
      initInterGl_     = false;
      interSelfGlobal_ = (org.interSelfGlobal_) ? this->grid_.geometryProvider_.getNewObjectEntity( this->grid_ , walkLevel_ ) : 0;
      bndEntity_       = (org.bndEntity_) ? this->grid_.bndProvider_.getNewObjectEntity( this->grid_ , walkLevel_ ) : 0;
    }
    else
    {
      last();
    }
  }

  template<class GridImp>
  inline ALU3dGridIntersectionIterator<GridImp> :: ~ALU3dGridIntersectionIterator()
  {
    if(interSelfGlobal_) this->grid_.geometryProvider_.freeObjectEntity( interSelfGlobal_ );
    interSelfGlobal_ = 0;

    if(bndEntity_) this->grid_.bndProvider_.freeObjectEntity( bndEntity_ );
    bndEntity_ = 0;
  }


  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> :: checkGhost () const
  {
#ifdef _ALU3DGRID_PARALLEL_
    isGhost_ = false;
    ghost_   = 0;
    if(isBoundary_)
    {
      ALU3DSPACE PLLBndFaceType * bnd =
        dynamic_cast<ALU3DSPACE PLLBndFaceType *> (item_->myneighbour(index_).first);
      if(bnd->bndtype() == ALU3DSPACE ProcessorBoundary_t)
      {
        isBoundary_ = false;
        isGhost_ = true;
      }
    }
#else
    isGhost_ = false;
#endif
  }

  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> :: increment ()
  {
    assert(item_);

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
      // set iterator to end status
      // we cannot call last here, because last only is for end iterators
      this->done();
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

      ALU3DSPACE GEOFaceType * dwn = neighpair_.first->down();
      if( theSituation_ && dwn )
      {
        neighpair_.first = dwn;
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


      ghost_ = static_cast<ALU3DSPACE PLLBndFaceType *> (np.first);
      numberInNeigh_ = np.second;

      // if our level is smaller then the level of the real ghost then go one
      // level up and set the element
      if((*ghost_).ghostLevel() != (*ghost_).level())
      {
        assert(ghost_->ghostLevel() < ghost_->level());
        assert(ghost_->up());

        if(daOtherSituation_)
        {
          neighpair_ = (*item_).myintersection(index_);
          daOtherSituation_ = false;
        }

        ghost_ = static_cast<ALU3DSPACE PLLBndFaceType *> ( ghost_->up() );
        assert(ghost_->level() == ghost_->ghostLevel());
      }

      assert( ghost_->getGhost() );

      //entity_.setGhost( *ghost_ ); // old method
      (*(this->entity_)).setGhost( *(ghost_->getGhost()) );

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

    (*(this->entity_)).setElement(*neigh_);
    ghost_ = 0;
    needSetup_ = false;
  }

  template<class GridImp>
  inline typename ALU3dGridIntersectionIterator<GridImp>::Entity &
  ALU3dGridIntersectionIterator<GridImp>::dereference () const
  {
    if(needSetup_) setNeighbor();
    return ALU3dGridEntityPointer<0,GridImp>::dereference();
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
  inline typename ALU3dGridIntersectionIterator<GridImp>::NormalType
  ALU3dGridIntersectionIterator<GridImp>::
  integrationOuterNormal(const FieldVector<alu3d_ctype, dim-1>& local) const
  {
    return this->outerNormal(local);
  }

  template<class GridImp>
  inline typename ALU3dGridIntersectionIterator<GridImp>::NormalType
  ALU3dGridIntersectionIterator<GridImp>::
  outerNormal(const FieldVector<alu3d_ctype, dim-1>& local) const
  {
    assert(item_ != 0);
    assert( (!needNormal_) ? (dwarn << "WARNING: ALU3dGridIntersectionIterator<GridImp>::outerNormal() called more than once for the same face!\n This is inefficient, store to normal outside! \n", 1) : 1 );

    {
      NormalType outNormal;

      if( boundary() ) {
        // if boundary calc normal normal ;)
        ALU3DSPACE BSGridLinearSurfaceMapping
        LSM (
          (*item_).myvertex(index_,0)->Point(),
          (*item_).myvertex(index_,1)->Point(),
          (*item_).myvertex(index_,2)->Point()
          );
        LSM.normal(outNormal);
      } else {
        if(needSetup_) setNeighbor();

        if (!daOtherSituation_)
        {
          ALU3DSPACE BSGridLinearSurfaceMapping
          LSM (
            (*item_).myvertex(index_,0)->Point(),
            (*item_).myvertex(index_,1)->Point(),
            (*item_).myvertex(index_,2)->Point()
            );
          LSM.normal(outNormal);
        }
        else
        {
          if(neigh_)
          {
            ALU3DSPACE BSGridLinearSurfaceMapping
            LSM (
              (*neigh_).myvertex(numberInNeigh_,2)->Point(),
              (*neigh_).myvertex(numberInNeigh_,1)->Point(),
              (*neigh_).myvertex(numberInNeigh_,0)->Point()
              );
            LSM.normal(outNormal);
          }
          else
          {
            assert(ghost_);
            assert(ghost_->level() != (*item_).level());

            ALU3DSPACE BSGridLinearSurfaceMapping
            LSM (
              (*item_).myvertex(index_,0)->Point(),
              (*item_).myvertex(index_,1)->Point(),
              (*item_).myvertex(index_,2)->Point()
              );
            LSM.normal(outNormal);
            // ghostpair_.second stores the twist of the face
            // multiply by 0.25 because the normal is scaled with the face
            // volume and we have a nonconformity here
            outNormal *= 0.25;
          }
        }
      }
      needNormal_ = false;
      return outNormal;
    }
    assert(false);
    DUNE_THROW(ALU3dGridError,"Error in IntersectionIterator::outerNormal()! \n");
    NormalType tmp;
    return tmp;
  }

  template<class GridImp>
  inline typename ALU3dGridIntersectionIterator<GridImp>::NormalType
  ALU3dGridIntersectionIterator<GridImp>::
  unitOuterNormal(const FieldVector<alu3d_ctype, dim-1>& local) const
  {
    NormalType unitOuterNormal_  = this->outerNormal(local);
    unitOuterNormal_ *= (1.0/unitOuterNormal_.two_norm());
    return unitOuterNormal_;
  }

  template<class GridImp>
  inline const typename ALU3dGridIntersectionIterator<GridImp>::Geometry &
  ALU3dGridIntersectionIterator<GridImp>::intersectionGlobal () const
  {
    if(initInterGl_)
    {
      assert( interSelfGlobal_ );
      return (*interSelfGlobal_);
    }

    if( boundary() )
    {
      assert( interSelfGlobal_ );
      const ALU3DSPACE GEOFaceType & face = *( (*item_).myhface3(index_));
      initInterGl_ = (*interSelfGlobal_).buildGeom(face);
      return (*interSelfGlobal_);
    }

    // in case of neighbor
    if( needSetup_ ) setNeighbor();

    assert( interSelfGlobal_ );
    initInterGl_ = (*interSelfGlobal_).buildGeom( *(neighpair_.first) );
    return (*interSelfGlobal_);
  }

  template<class GridImp>
  inline const typename ALU3dGridIntersectionIterator<GridImp>::BoundaryEntity &
  ALU3dGridIntersectionIterator<GridImp>::boundaryEntity () const
  {
    assert(boundary());
    ALU3DSPACE BNDFaceType * bnd = dynamic_cast<ALU3DSPACE BNDFaceType *> (item_->myneighbour(index_).first);
    int id = bnd->bndtype(); // id's are positive
    (*bndEntity_).setId( -id );
    return (*bndEntity_);
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
  removeElement ()
  {
    item_ = 0;
  }

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
  ALU3dGridEntity<0,dim,GridImp> :: setEntity(const ALU3dGridEntity<0,dim,GridImp> & org)
  {
    item_          = org.item_;
    isGhost_       = org.isGhost_;
    ghost_         = org.ghost_;
    builtgeometry_ = false;
    index_         = org.index_;
    level_         = org.level_;
    glIndex_       = org.glIndex_;
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
    item_= static_cast<ALU3DSPACE IMPLElementType *> (&element);
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
    abort();
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
  inline bool ALU3dGridEntity<0,dim,GridImp> ::
  equals (const ALU3dGridEntity<0,dim,GridImp> &org ) const
  {
    return (item_ == org.item_);
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
  template <int codim> struct IndexWrapper;

  // specialisation for vertices
  template <> struct IndexWrapper<3>
  {
    static inline int subIndex(const ALU3DSPACE IMPLElementType &elem, int i)
    {
      return elem.myvertex(i)->getIndex();
    }
  };

  // specialisation for faces
  template <> struct IndexWrapper<1>
  {
    static inline int subIndex(const ALU3DSPACE IMPLElementType &elem, int i)
    {
      return elem.myhface3(i)->getIndex();
    }
  };

  // specialisation for faces
  template <> struct IndexWrapper<2>
  {
    static inline int subIndex(const ALU3DSPACE IMPLElementType &elem, int i)
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
    return IndexWrapper<cc>::subIndex ( *item_ ,i);
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
    entity (const GridImp & grid, const ALU3DSPACE IMPLElementType & item, int i)
    {
      return ALU3dGridEntityPointer<1,GridImp> (grid, *(item.myhface3(i)) );
    }
  };

  // specialisation for edges
  template <class GridImp, int dim>
  struct SubEntities<GridImp,dim,2>
  {
    static typename ALU3dGridEntity<0,dim,GridImp> :: template codim<2>:: EntityPointer
    entity (const GridImp & grid, const ALU3DSPACE IMPLElementType & item, int i)
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
    entity (const GridImp & grid, const ALU3DSPACE IMPLElementType & item, int i)
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
    // mark for coarsening
    if(ref < 0)
    {
      if(level() <= 0) return false;
      if((*item_).requestrule() == ALU3DSPACE refine_element_t)
      {
        return false;
      }

      (*item_).request( ALU3DSPACE coarse_element_t );
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
  inline void ALU3dGridEntity<3,3,const ALU3dGrid<3,3> > ::
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
