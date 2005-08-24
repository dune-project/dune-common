// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// Dune includes
#include <dune/common/stdstreams.hh>

// Local includes
#include "entity.hh"
#include "iterator.hh"
#include "myautoptr.hh"
#include "datahandle.hh"

namespace Dune {

  //--Grid
  //template <int dim, int dimworld, ALU3dGridElementType elType>
  //const ALU3dGridElementType
  //ALU3dGrid<dim, dimworld, elType>::elementType = elType;

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline ALU3dGrid<dim, dimworld, elType>::
  ALU3dGrid(const char* macroTriangFilename
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
      , geomTypes_ (numberOfGeomTypes)
      , hIndexSet_ (*this)
      , globalIdSet_(*this), localIdSet_(*this)
      , levelIndexVec_(MAXL) , leafIndexSet_(0)
  {
    if( myRank_ <= 0 )
    {
      checkMacroGrid ( elType , macroTriangFilename );
    }
    for(unsigned int i=0; i<levelIndexVec_.size(); i++) levelIndexVec_[i] = 0;
    geomTypes_[0] = (elType == tetra) ? simplex : cube;

    mygrid_ = new ALU3DSPACE GitterImplType (macroTriangFilename
#ifdef _ALU3DGRID_PARALLEL_
                                             , mpAccess_
#endif
                                             );
    assert(mygrid_ != 0);

#ifdef _ALU3DGRID_PARALLEL_
    //loadBalance();
    __MyRank__ = mpAccess_.myrank();

    dverb << "************************************************\n";
    dverb << "Created grid on p=" << mpAccess_.myrank() << "\n";
    dverb << "************************************************\n";

#endif

    mygrid_->printsize();


    postAdapt();
    calcExtras();
    std::cout << "Constructor of Grid finished!\n";
  }

#ifdef _ALU3DGRID_PARALLEL_
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline ALU3dGrid<dim, dimworld, elType>::ALU3dGrid(MPI_Comm mpiComm)
    : mygrid_ (0) , maxlevel_(0)
      , coarsenMarked_(0) , refineMarked_(0)
      , mpAccess_(mpiComm) , myRank_( mpAccess_.myrank() )
      , geomTypes_ (numberOfGeomTypes)
      , hIndexSet_ (*this)
      , globalIdSet_(*this), localIdSet_(*this)
      , levelIndexVec_(MAXL) , leafIndexSet_(0)
  {
    for(unsigned int i=0; i<levelIndexVec_.size(); i++) levelIndexVec_[i] = 0;
    geomTypes_[0] = (elType == tetra) ? simplex : cube;
  }
#else
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline ALU3dGrid<dim, dimworld, elType>::ALU3dGrid(int myrank)
    : mygrid_ (0) , maxlevel_(0)
      , coarsenMarked_(0) , refineMarked_(0)
      , myRank_(myrank)
      , geomTypes_ (numberOfGeomTypes)
      , hIndexSet_ (*this)
      , globalIdSet_ (*this)
      , localIdSet_ (*this)
      , levelIndexVec_(MAXL) , leafIndexSet_(0)
  {
    for(unsigned int i=0; i<levelIndexVec_.size(); i++) levelIndexVec_[i] = 0;
    geomTypes_[0] = (elType == tetra) ? simplex : cube;
  }
#endif

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline ALU3dGrid<dim, dimworld, elType>::ALU3dGrid(const ALU3dGrid<dim, dimworld, elType> & g)
    : mygrid_ (0) , maxlevel_(0)
      , coarsenMarked_(0) , refineMarked_(0)
      , myRank_(-1)
      , geomTypes_ (numberOfGeomTypes)
      , hIndexSet_(*this)
      , globalIdSet_ (*this)
      , localIdSet_ (*this)
      , levelIndexVec_(MAXL) , leafIndexSet_(0)
  {
    geomTypes_[0] = (elType == tetra) ? simplex : cube;
    for(unsigned int i=0; i<levelIndexVec_.size(); i++) levelIndexVec_[i] = 0;
    DUNE_THROW(GridError,"Do not use copy constructor of ALU3dGrid! \n");
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline ALU3dGrid<dim, dimworld, elType> & ALU3dGrid<dim, dimworld, elType>::operator = (const ALU3dGrid<dim, dimworld, elType> & g)
  {
    DUNE_THROW(GridError,"Do not use assignment operator of ALU3dGrid! \n");
    return (*this);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline ALU3dGrid<dim, dimworld, elType>::~ALU3dGrid()
  {
    for(unsigned int i=0; i<levelIndexVec_.size(); i++)
    {
      if(levelIndexVec_[i]) delete levelIndexVec_[i];
    }
    if(leafIndexSet_) delete leafIndexSet_;
    if(mygrid_) delete mygrid_;
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline int ALU3dGrid<dim, dimworld, elType>::size(int level, int codim) const
  {
    return levelIndexSet(level).size(codim);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline void ALU3dGrid<dim, dimworld, elType>::updateStatus()
  {
    calcMaxlevel();
    calcExtras();
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline void ALU3dGrid<dim, dimworld, elType>::calcMaxlevel()
  {
    assert(mygrid_);
    maxlevel_ = 0;
    ALU3DSPACE BSLeafIteratorMaxLevel w (*mygrid_) ;
    for (w->first () ; ! w->done () ; w->next ())
    {
      if(w->item().level() > maxlevel_ ) maxlevel_ = w->item().level();
    }
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline void ALU3dGrid<dim, dimworld, elType>::calcExtras()
  {
    for(unsigned int i=0; i<levelIndexVec_.size(); i++)
    {
      if(levelIndexVec_[i]) (*(levelIndexVec_[i])).calcNewIndex();
    }

    coarsenMarked_ = 0;
    refineMarked_  = 0;
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline int ALU3dGrid<dim, dimworld, elType>::global_size(int codim) const
  {
    // return actual size of hierarchical index set
    // this is always up to date
    // maxIndex is the largest index used + 1
    return (*mygrid_).indexManager(codim).getMaxIndex();
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline const typename ALU3dGrid<dim, dimworld, elType>::LeafIndexSetType &
  ALU3dGrid<dim, dimworld, elType>::leafIndexSet() const
  {
    if(!leafIndexSet_) leafIndexSet_ = new LeafIndexSetType ( *this );
    return *leafIndexSet_;
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline typename ALU3dGrid<dim, dimworld, elType>::LeafIndexSetType &
  ALU3dGrid<dim, dimworld, elType>::leafIndexSet()
  {
    if(!leafIndexSet_) leafIndexSet_ = new LeafIndexSetType ( *this );
    return *leafIndexSet_;
  }


  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline const typename ALU3dGrid<dim, dimworld, elType>::LevelIndexSetType &
  ALU3dGrid<dim, dimworld, elType>::levelIndexSet( int level ) const
  {
    if( (level < 0) && (level >= MAXL) )
      DUNE_THROW(GridError,"Only " << MAXL << "levels allowed for this grid!\n");

    if( levelIndexVec_[level] == 0 )
      levelIndexVec_[level] = new LevelIndexSetType ( *this , level );
    return *(levelIndexVec_[level]);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline int ALU3dGrid<dim, dimworld, elType>::maxlevel() const
  {
    return maxlevel_;
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline ALU3DSPACE GitterImplType & ALU3dGrid<dim, dimworld, elType>::myGrid()
  {
    return *mygrid_;
  }

  // lbegin methods
  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int cd, PartitionIteratorType pitype>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template Codim<cd>::template Partition<pitype>::LevelIterator
  ALU3dGrid<dim, dimworld, elType>::lbegin(int level) const {
    assert( level >= 0 );
    return ALU3dGridLevelIterator<cd,pitype,const MyType> (*this,level);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int cd, PartitionIteratorType pitype>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template Codim<cd>::template Partition<pitype>::LevelIterator
  ALU3dGrid<dim, dimworld, elType>::lend(int level) const {
    assert( level >= 0 );
    return ALU3dGridLevelIterator<cd,pitype,const MyType> (*this,level,true);
  }

  // lbegin methods
  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int cd>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template Codim<cd>::template Partition<All_Partition>::LevelIterator
  ALU3dGrid<dim, dimworld, elType>::lbegin(int level) const {
    assert( level >= 0 );
    return ALU3dGridLevelIterator<cd,All_Partition,const MyType> (*this,level);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int cd>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template Codim<cd>::template Partition<All_Partition>::LevelIterator
  ALU3dGrid<dim, dimworld, elType>::lend(int level) const {
    assert( level >= 0 );
    return ALU3dGridLevelIterator<cd,All_Partition,const MyType> (*this,level,true);
  }

  // leaf methods
  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int cd, PartitionIteratorType pitype>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template Codim<cd>::template Partition<pitype>::LeafIterator
  ALU3dGrid<dim, dimworld, elType>::leafbegin(int level) const
  {
    assert( level >= 0 );
    return ALU3dGridLeafIterator<cd, pitype, const MyType> ((*this),level, false,
#ifdef _ALU3DGRID_PARALLEL_
                                                            mpAccess_.nlinks()
#else
                                                            1
#endif
                                                            );
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int cd>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template Codim<cd>::LeafIterator
  ALU3dGrid<dim, dimworld, elType>::leafbegin(int level) const {
    return leafbegin<cd, All_Partition>(level);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int codim, PartitionIteratorType pitype>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
  ALU3dGrid<dim, dimworld, elType>::leafbegin() const {
    return leafbegin<codim, pitype>(maxlevel_);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int cd, PartitionIteratorType pitype>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template Codim<cd>::template Partition<pitype>::LeafIterator
  ALU3dGrid<dim, dimworld, elType>::leafend(int level) const
  {
    assert( level >= 0 );
    return ALU3dGridLeafIterator<cd, pitype, const MyType> ((*this),
                                                            level,
                                                            true,
#ifdef _ALU3DGRID_PARALLEL_
                                                            mpAccess_.nlinks()
#else
                                                            1
#endif
                                                            );
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int codim>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template Codim<codim>::LeafIterator
  ALU3dGrid<dim, dimworld, elType>::leafend(int level) const {
    return leafend<codim, All_Partition>(level);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int codim, PartitionIteratorType pitype>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
  ALU3dGrid<dim, dimworld, elType>::leafend() const {
    return leafend<codim, pitype>(maxlevel_);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int codim>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template Codim<codim>::LeafIterator
  ALU3dGrid<dim, dimworld, elType>::leafbegin() const {
    return leafbegin<codim, All_Partition>(maxlevel_);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int codim>
  inline typename ALU3dGrid<dim, dimworld, elType>::Traits::template Codim<codim>::LeafIterator
  ALU3dGrid<dim, dimworld, elType>::leafend() const {
    return leafend<codim, All_Partition>(maxlevel_);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  typename ALU3dGrid<dim, dimworld, elType>::LeafIteratorType
  ALU3dGrid<dim, dimworld, elType>::leafbegin(int level) const {
    return leafbegin<0, All_Partition>(level);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  typename ALU3dGrid<dim, dimworld, elType>::LeafIteratorType
  ALU3dGrid<dim, dimworld, elType>::leafend(int level) const {
    return leafend<0, All_Partition>(level);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  typename ALU3dGrid<dim, dimworld, elType>::LeafIteratorType
  ALU3dGrid<dim, dimworld, elType>::leafbegin() const {
    return leafbegin<0, All_Partition>(maxlevel_);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  typename ALU3dGrid<dim, dimworld, elType>::LeafIteratorType
  ALU3dGrid<dim, dimworld, elType>::leafend() const {
    return leafend<0, All_Partition>(maxlevel_);
  }

  // global refine
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline bool ALU3dGrid<dim,dimworld, elType>::
  mark(int ref, typename Traits::template Codim<0>::EntityPointer & ep )
  {
    return this->mark(ref,*ep);
  }

  // global refine
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline bool ALU3dGrid<dim,dimworld, elType>::
  mark(int ref, const typename Traits::template Codim<0>::Entity & ep )
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
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline bool ALU3dGrid<dim, dimworld, elType>::globalRefine(int numberOfRefines)
  {
    bool ref = false;
    for (int count = numberOfRefines; count>0; count--)
    {
      LeafIteratorType endit  = leafend   ( maxlevel() );
      for(LeafIteratorType it = leafbegin ( maxlevel() ); it != endit; ++it)
      {
        this->mark(1, (*it) );
      }
      ref = this->adapt();
      if(ref) this->postAdapt();
    }

    // important that loadbalance is called on each processor
    // so dont put any if statements arround here
    this->loadBalance();

    return ref;
  }

  // preprocess grid
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline bool ALU3dGrid<dim, dimworld, elType>::preAdapt()
  {
    return (coarsenMarked_ > 0);
  }

  // adapt grid
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline bool ALU3dGrid<dim, dimworld, elType>::adapt()
  {
#ifdef _ALU3DGRID_PARALLEL_
    bool ref = false;
    if(leafIndexSet_)
    {
      EntityImp f ( *this, this->maxlevel() );
      EntityImp s ( *this, this->maxlevel() );

      ALU3DSPACE AdaptRestrictProlongImpl<ALU3dGrid<dim, dimworld, elType>,
          EntityImp, LeafIndexSetType, LeafIndexSetType >
      rp(*this,f,s, *leafIndexSet_  ,*leafIndexSet_);

      ref = myGrid().duneAdapt(rp); // adapt grid
      leafIndexSet_->compress();
    }
    else
      ref = myGrid().dAdapt(); // adapt grid
#else
    bool ref = myGrid().adapt(); // adapt grid
#endif
    if(ref)
    {
      // calcs maxlevel and other extras
      updateStatus();
    }
    return ref;
  }

  // adapt grid
  // --adapt
  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <class DofManagerType, class RestrictProlongOperatorType>
  inline bool ALU3dGrid<dim, dimworld, elType>::
  adapt(DofManagerType & dm, RestrictProlongOperatorType & rpo, bool verbose )
  {
    assert( ((verbose) ? (dverb << "ALU3dGrid :: adapt() new method called!\n", 1) : 1 ) );

    EntityImp f ( *this, this->maxlevel() );
    EntityImp s ( *this, this->maxlevel() );

    if(leafIndexSet_)
    {
      if( ! dm.checkIndexSetExists( *leafIndexSet_ ))
      {
        std::cout << "Add LeafIndexSet to DofManager! \n";
        dm.addIndexSet( *this , *leafIndexSet_ );
      }
    }

    typedef typename DofManagerType :: IndexSetRestrictProlongType IndexSetRPType;
    typedef CombinedAdaptProlongRestrict < IndexSetRPType,RestrictProlongOperatorType > COType;
    COType tmprpop ( dm.indexSetRPop() , rpo );

    int defaultChunk = newElementsChunk_;
    int actChunk     = refineEstimate_ * refineMarked_;

    // guess how many new elements we get
    int newElements = std::max( actChunk , defaultChunk );
    ALU3DSPACE AdaptRestrictProlongImpl<ALU3dGrid<dim, dimworld, elType>,
        EntityImp, DofManagerType, COType >
    rp(*this,f,s,dm,tmprpop);

    dm.resizeMem( newElements );
    bool ref = myGrid().duneAdapt(rp); // adapt grid

    // if new maxlevel was claculated
    if(rp.maxlevel() >= 0) maxlevel_ = rp.maxlevel();
    assert( ((verbose) ? (dverb << "maxlevel = " << maxlevel_ << "!\n", 1) : 1 ) );

    if(ref)
    {
      calcMaxlevel();
      calcExtras();     // reset size and things
    }

    // check whether we have balance
    loadBalance(dm);
    dm.dofCompress();

    //communicate(dm);

    postAdapt();
    assert( ((verbose) ? (dverb << "ALU3dGrid :: adapt() new method finished!\n", 1) : 1 ) );
    return ref;
  }


  // post process grid
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline void ALU3dGrid<dim, dimworld, elType>::postAdapt()
  {
#ifndef _ALU3DGRID_PARALLEL_
    //  if(mpAccess_.nlinks() < 1)
    //#endif
    {
      maxlevel_ = 0;
      ALU3DSPACE BSLeafIteratorMaxLevel w ( myGrid() ) ;
      for (w->first () ; ! w->done () ; w->next ())
      {
        if(w->item().level() > maxlevel_ ) maxlevel_ = w->item().level();
        w->item ().resetRefinedTag();

        // note, resetRefinementRequest sets the request to coarsen
        w->item ().resetRefinementRequest();
      }
    }
    //#ifdef _ALU3DGRID_PARALLEL_
#else
    //  else
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

        // note, resetRefinementRequest sets the request to coarsen
        w->item ().resetRefinementRequest();
      }
    }
#endif
  }

  template <int dim, int dimworld, ALU3dGridElementType elType> template <class T>
  inline T ALU3dGrid<dim, dimworld, elType>::globalMin(T val) const
  {
#ifdef _ALU3DGRID_PARALLEL_
    T ret = mpAccess_.gmin(val);
    return ret;
#else
    return val;
#endif
  }
  template <int dim, int dimworld, ALU3dGridElementType elType> template <class T>
  inline T ALU3dGrid<dim, dimworld, elType>::globalMax(T val) const
  {
#ifdef _ALU3DGRID_PARALLEL_
    T ret = mpAccess_.gmax(val);
    return ret;
#else
    return val;
#endif
  }
  template <int dim, int dimworld, ALU3dGridElementType elType> template <class T>
  inline T ALU3dGrid<dim, dimworld, elType>::globalSum(T val) const
  {
#ifdef _ALU3DGRID_PARALLEL_
    T sum = mpAccess_.gsum(val);
    return sum;
#else
    return val;
#endif
  }
  template <int dim, int dimworld, ALU3dGridElementType elType> template <class T>
  inline void ALU3dGrid<dim, dimworld, elType>::globalSum(T * send, int s , T * recv) const
  {
#ifdef _ALU3DGRID_PARALLEL_
    mpAccess_.gsum(send,s,recv);
    return ;
#else
    std::memcpy(recv,send,s*sizeof(T));
    return ;
#endif
  }


  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline bool ALU3dGrid<dim, dimworld, elType>::loadBalance()
  {
#ifdef _ALU3DGRID_PARALLEL_
    bool changed = myGrid().duneLoadBalance();
    if(changed)
    {
      dverb << "Grid was balanced on p = " << myRank() << std::endl;
      calcMaxlevel();                 // calculate new maxlevel
      calcExtras();                   // reset size and things
    }
    return changed;
#else
    return false;
#endif
  }

  // adapt grid
  template <int dim, int dimworld, ALU3dGridElementType elType> template <class DataCollectorType>
  inline bool ALU3dGrid<dim, dimworld, elType>::loadBalance(DataCollectorType & dc)
  {
#ifdef _ALU3DGRID_PARALLEL_
    EntityImp en     ( *this, this->maxlevel() );
    EntityImp father ( *this, this->maxlevel() );
    EntityImp son    ( *this, this->maxlevel() );

    if(leafIndexSet_)
    {
      if( ! dc.checkIndexSetExists( *leafIndexSet_ ))
      {
        std::cout << "Add LeafIndexSet to DofManager! \n";
        dc.addIndexSet( *this , *leafIndexSet_ );
      }
    }

    ALU3DSPACE GatherScatterImpl< ALU3dGrid<dim, dimworld, elType>, EntityImp, DataCollectorType >
    gs(*this,en,dc);

    ALU3DSPACE LoadBalanceRestrictProlongImpl < MyType , EntityImp,
        DataCollectorType > idxop ( *this, father , son , dc );

    int defaultChunk = newElementsChunk_;

    bool changed = myGrid().duneLoadBalance(gs,idxop);
    int memSize = std::max( idxop.newElements(), defaultChunk );
    dc.resizeMem( memSize );

    if(changed)
    {
      dverb << "Grid was balanced on p = " << myRank() << std::endl;
      calcMaxlevel();               // calculate new maxlevel
      calcExtras();                 // reset size and things
    }

    // checken, ob wir das hier wirklich brauchen
    myGrid().duneExchangeData(gs);
    return changed;
#else
    return false;
#endif
  }

  // adapt grid
  template <int dim, int dimworld, ALU3dGridElementType elType> template <class DataCollectorType>
  inline bool ALU3dGrid<dim, dimworld, elType>::communicate(DataCollectorType & dc)
  {
#ifdef _ALU3DGRID_PARALLEL_
    EntityImp en ( *this, this->maxlevel() );

    ALU3DSPACE GatherScatterExchange < ALU3dGrid<dim, dimworld, elType> , EntityImp ,
        DataCollectorType > gs(*this,en,dc);

    myGrid().duneExchangeData(gs);
    return true;
#else
    return false;
#endif
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline bool ALU3dGrid<dim, dimworld, elType>::
  writeGrid_Ascii(const std::string filename, alu3d_ctype time ) const
  {
    ALU3DSPACE GitterImplType & mygrd =
      const_cast<ALU3dGrid<dim, dimworld, elType> &> (*this).myGrid();
    std::fstream file ( filename.c_str() , std::ios::out);
    if(file)
    {
      typedef typename ALU3dImplTraits<elType> :: BNDFaceType BNDFaceType;
      typedef typename ALU3dImplTraits<elType> :: IMPLElementType IMPLElementType;
      typedef typename ALU3dImplTraits<elType> :: HasFaceType HasFaceType;

      file << "!" << elType2Name( elType ) << endl;
      {
        ALU3DSPACE LeafIterator < ALU3DSPACE VertexType > vx (mygrd) ;
        file << endl;

        // write coordinates of the vertices
        int vxsize = vx->size();
        file << vxsize << endl;
        Array < double[3] > vxvec ( vxsize );

        for( vx->first(); !vx->done() ; vx->next() )
        {
          const double (&p)[3] = vx->item().Point();
          int vxidx = vx->item().getIndex();
          double (&v)[3] = vxvec[vxidx];
          for(int i=0; i<3; i++) v[i] = p[i];
        }

        for(int i=0; i<vxsize; i++)
        {
          file << vxvec[i][0] << " " << vxvec[i][1] << " " << vxvec[i][2] << endl;
        }
      }

      file << endl;
      // write element vertices
      {
        const int novx = (elType == tetra) ? 4 : 8;
        ALU3DSPACE LeafIterator < ALU3DSPACE HElementType > el (mygrd) ;
        file << el->size() << endl;
        for( el->first(); !el->done() ; el->next() )
        {
          IMPLElementType & item = static_cast<IMPLElementType &> (el->item());
          for(int i=0; i<novx; i++)
          {
            const int vxnum = item.myvertex(i)->getIndex();
            file << vxnum << " ";
          }
          file << endl;
        }
      }

      // write boundary faces
      {
        file << endl;
        const int nofaces  = (elType == tetra) ? 4 : 6;
        int bndfaces = 0;
        ALU3DSPACE LeafIterator < ALU3DSPACE HElementType > el (mygrd) ;
        for( el->first(); !el->done() ; el->next() )
        {
          IMPLElementType & item = static_cast<IMPLElementType &> (el->item());
          for(int i=0; i<nofaces; i++)
          {
            std::pair < HasFaceType * , int > nbpair = item.myneighbour(i);
            if(nbpair.first->isboundary())
            {
              bndfaces++;
            }
          }
        }
        file << bndfaces << endl;
      }
      // write boundary faces
      {
        const int bndvxnum = (elType == tetra) ? 3 : 4;
        const int nofaces  = (elType == tetra) ? 4 : 6;
        ALU3DSPACE LeafIterator < ALU3DSPACE HElementType > el (mygrd) ;
        for( el->first(); !el->done() ; el->next() )
        {
          IMPLElementType & item = static_cast<IMPLElementType &> (el->item());
          for(int i=0; i<nofaces; i++)
          {
            std::pair < HasFaceType * , int > nbpair = item.myneighbour(i);
            if(nbpair.first->isboundary())
            {
              BNDFaceType * face = static_cast<BNDFaceType *> (nbpair.first);
              file << -(face->bndtype()) << " " << bndvxnum << " ";
              for(int j=0; j<bndvxnum; j++)
              {
                int vxnum = face->myvertex(0,j)->getIndex();
                file << vxnum << " ";
              }
              file << endl;
            }
          }
        }
      }

      {
        ALU3DSPACE LeafIterator < ALU3DSPACE VertexType > vx (mygrd) ;
        file << endl;

        // write coordinates of the vertices
        int vxnum = 0;
        for( vx->first(); !vx->done() ; vx->next() )
        {
          file << vxnum << " -1" << endl;
          vxnum++;
        }
      }
    }
    return true;
  }


  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <GrapeIOFileFormatType ftype>
  inline bool ALU3dGrid<dim, dimworld, elType>::
  writeGrid(const std::string filename, alu3d_ctype time ) const
  {
    switch(ftype)
    {
    case xdr  : return writeGrid_Xdr(filename,time);
    case ascii : return writeGrid_Ascii(filename,time);
    default : derr << "Wrong file type in writeGrid method~ \n";
    }
    return false;
  }


  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline bool ALU3dGrid<dim, dimworld, elType>::
  writeGrid_Xdr(const std::string filename, alu3d_ctype time ) const
  {
    ALU3DSPACE GitterImplType & mygrd = const_cast<ALU3dGrid<dim, dimworld, elType> &> (*this).myGrid();
    mygrd.duneBackup(filename.c_str());

    // write time and maxlevel
    {
      std::string extraName(filename);
      extraName += ".extra";
      std::ofstream out (extraName.c_str());
      if(out)
      {
        out.precision (16);
        out << time << " ";
        out << maxlevel_ << " ";
        out.close();
      }
      else
      {
        derr << "ALU3dGrid::writeGrid: couldn't open <" << extraName << ">! \n";
      }
    }
    return true;
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <GrapeIOFileFormatType ftype>
  inline bool ALU3dGrid<dim,dimworld, elType>::
  readGrid( const std::string filename, alu3d_ctype & time )
  {
    {
      typedef std::ostringstream StreamType;
      std::string mName(filename);
      mName += ".macro";
      const char * macroName = mName.c_str();

      { //check if file exists
        std::ifstream check ( macroName );
        if( !check )
          DUNE_THROW(GridError,"cannot read file " << macroName << "\n");
        check.close();
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
      std::string extraName (filename);
      extraName += ".extra";
      std::ifstream in (extraName.c_str());
      if(in)
      {
        in.precision (16);
        in  >> time;
        in  >> maxlevel_;
        in.close();
      }
      else
      {
        derr << "ALU3dGrid::readGrid: couldn't open <" << extraName << ">! \n";
      }
    }

    calcMaxlevel();  // calculate new maxlevel
    calcExtras();    // calculate indices

    return true;
  }

  // return Grid type
  template <int dim, int dimworld, ALU3dGridElementType elType>
  inline GridIdentifier ALU3dGrid<dim, dimworld, elType>::type () const
  {
    return ALU3dGrid_Id;
  }

  // return Grid type
  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int cd>
  inline ALU3dGridMakeableEntity<cd,dim,const ALU3dGrid<dim,dimworld,elType> > *
  ALU3dGrid<dim,dimworld,elType>::getNewEntity (int level) const
  {
    return ALU3dGridEntityFactory<MyType,cd>::getNewEntity(*this,entityProvider_,level);
  }

  template <int dim, int dimworld, ALU3dGridElementType elType>
  template <int cd>
  inline void ALU3dGrid<dim,dimworld,elType>::
  freeEntity (ALU3dGridMakeableEntity<cd,dim,const ALU3dGrid<dim,dimworld,elType> > * e) const
  {
    return ALU3dGridEntityFactory<MyType,cd>::freeEntity(entityProvider_, e);
  }


  inline const char * elType2Name( ALU3dGridElementType elType )
  {
    switch( elType )
    {
    case tetra  : return "Tetraeder";
    case hexa   : return "Hexaeder";
    case mixed  : return "Mixed";
    default     : return "Error";
    }
  }

  inline bool checkMacroGrid ( ALU3dGridElementType elType , const std::string filename )
  {
    std::fstream file (filename.c_str(),std::ios::in);
    if( file )
    {
      std::string str;
      file >> str;

      std::string cmp ("!");
      cmp += elType2Name( elType );

      if (str != cmp)
      {
        derr << "ALU3DGrid<" << elType2Name(elType) << "> tries to read MacroGridFile with < " << str
             << " >. Identifier should be < " << cmp << " >!\n";
        //abort();
      }

      file.close();
      return true;
    }
    else
    {
      derr << "Couldn't open macro grid file < " << filename << " > !\n";
      abort();
    }
    return false;
  }

} // end namespace Dune
