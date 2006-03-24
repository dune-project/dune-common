// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
namespace Dune
{

  //****************************************************************
  //
  // --CombinedGrapeDisplay, CombinedGrapeDisplay for given grid
  //
  //****************************************************************

  template<class DisplayType>
  inline CombinedGrapeDisplay<DisplayType>::
  CombinedGrapeDisplay() : disp_(0) , hmesh_ (0)
  {
    GrapeInterface<dim,dimworld>::init();
  }

  template<class DisplayType>
  inline CombinedGrapeDisplay<DisplayType>::
  ~CombinedGrapeDisplay()
  {
    for(unsigned int i=0 ; i<vecFdata_.size(); i++)
    {
      DUNE_FDATA * fd = vecFdata_[i];
      if( fd )
      {
        int * comps = fd->comp;
        if( comps ) delete [] comps;
        delete fd;
        vecFdata_[i] = 0;
      }
    }
  }


  //****************************************************************
  //
  // --GridDisplay, Some Subroutines needed in display
  //
  //****************************************************************
  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  first_macro (DUNE_ELEM * he)
  {
    grditer_ = dispList_.begin();
    enditer_ = dispList_.end();
    disp_ = 0;

    if(grditer_ != enditer_)
    {
      disp_ = *grditer_;
      GrapeInterface<dim,dimworld>::setThread( disp_->myRank() );
      return disp_->firstMacro(he);
    }
    return 0;
  }

  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  next_macro (DUNE_ELEM * he)
  {
    int ret = 0;
    if( disp_ )
    {
      ret = disp_->nextMacro(he);
      if(!ret)
      {
        ++grditer_;
        disp_ = 0;
        if(grditer_ != enditer_)
        {
          disp_ = *grditer_;
          GrapeInterface<dim,dimworld>::setThread( disp_->myRank() );
          return disp_->firstMacro(he);
        }
      }
    }
    return ret;
  }

  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  first_child(DUNE_ELEM * he)
  {
    if(disp_)
      return disp_->firstChild(he);
    else
      return 0;
  }


  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  next_child(DUNE_ELEM * he)
  {
    if( disp_ )
      return disp_->nextChild(he);
    else
      return 0;
  }


  template<class DisplayType>
  inline void * CombinedGrapeDisplay<DisplayType>::
  copy_iterator (const void * i)
  {
    std::cerr << "ERROR: copt_iterator not implemented! file = " << __FILE__ << ", line = " << __LINE__ << "\n";
    abort () ;
    return 0 ;
  }

  // check inside
  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  checkInside(DUNE_ELEM * he, const double * w)
  {
    assert( disp_ );
    return disp_->checkWhetherInside(he,w);
  }
  // check inside
  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  check_inside(DUNE_ELEM * he, const double * w)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].checkInside(he,w);
  }

  template<class DisplayType>
  inline void CombinedGrapeDisplay<DisplayType>::
  local_to_world (DUNE_ELEM * he, const double * c, double * w)
  {
    assert( disp_ );
    disp_->local2world(he,c,w);
    return ;
  }

  template<class DisplayType>
  inline void CombinedGrapeDisplay<DisplayType>::
  ctow (DUNE_ELEM * he, const double * c, double * w)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    disp[0].local_to_world(he,c,w);
    return;
  }

  // world to local
  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  world_to_local(DUNE_ELEM * he, const double * w, double * c)
  {
    assert(disp_);
    return disp_->world2local(he,w,c);
  }

  // world to local
  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  wtoc(DUNE_ELEM * he, const double * w, double * c)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].world_to_local(he,w,c);
  }

  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  first_mac (DUNE_ELEM * he)
  {
    MyDisplayType & disp = *((MyDisplayType *) he->display);
    return disp.first_macro(he);
  }


  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  next_mac (DUNE_ELEM * he)
  {
    MyDisplayType & disp = *((MyDisplayType *) he->display);
    return disp.next_macro(he);
  }

  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  fst_child (DUNE_ELEM * he)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].first_child(he);
  }


  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  nxt_child (DUNE_ELEM * he)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].next_child(he);
  }

  template<class DisplayType>
  inline void CombinedGrapeDisplay<DisplayType>::
  setIterationModus(DUNE_DAT * dat)
  {
    MyDisplayType * disp = (MyDisplayType *) dat->all->display;
    disp[0].setIterationMethods(dat);
  }

  template<class DisplayType>
  inline void CombinedGrapeDisplay<DisplayType>::
  setIterationMethods(DUNE_DAT * dat)
  {
    enditer_ = dispList_.end();
    for(grditer_ = dispList_.begin(); grditer_ != enditer_; ++grditer_)
    {
      (*grditer_)->setIterationMethods(dat);
    }
  }

  template<class DisplayType>
  inline void CombinedGrapeDisplay<DisplayType>::display()
  {
    /* call handle mesh in g_hmesh.c */
    GrapeInterface<dim,dimworld>::handleMesh ( hmesh_ );
    return ;
  }

  template<class DisplayType>
  inline void * CombinedGrapeDisplay<DisplayType>::getHmesh()
  {
    if(!hmesh_) hmesh_ = setupHmesh();
    return (void *) hmesh_;
  }

  template<class DisplayType>
  inline void CombinedGrapeDisplay<DisplayType>::
  evalCoord (DUNE_ELEM *he, DUNE_FDATA *df, const double *coord, double * val)
  {
    assert( disp_ );
    std::vector < DUNE_FDATA * > & vec = disp_->getFdataVec();
    vec[df->mynum]->evalCoord(he,vec[df->mynum],coord,val);
    return ;
  }

  template<class DisplayType>
  inline void CombinedGrapeDisplay<DisplayType>::
  evalDof (DUNE_ELEM *he, DUNE_FDATA *df,int localNum, double * val)
  {
    assert( disp_ );
    std::vector < DUNE_FDATA * > & vec = disp_->getFdataVec();
    vec[df->mynum]->evalDof(he,vec[df->mynum],localNum,val);
    return ;
  }

  template<class DisplayType>
  inline void CombinedGrapeDisplay<DisplayType>::
  evalCoordWrap (DUNE_ELEM *he, DUNE_FDATA *df, const double *coord, double * val)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    assert( disp );
    disp->evalCoord(he,df,coord,val);
    return ;
  }

  template<class DisplayType>
  inline void CombinedGrapeDisplay<DisplayType>::
  evalDofWrap (DUNE_ELEM *he, DUNE_FDATA *df,int localNum, double * val)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    assert( disp );
    disp->evalDof(he,df,localNum,val);
    return ;
  }

  template<class DisplayType>
  inline void CombinedGrapeDisplay<DisplayType>::
  func_real (DUNE_ELEM *he , DUNE_FDATA * fe,int ind, const double *coord, double *val )
  {
    //MyDisplayType * disp = (MyDisplayType *) he->display;
    // this methtod is to be removed
    assert(false);
    abort();
    return;
  }

  template<class DisplayType>
  inline void CombinedGrapeDisplay<DisplayType>::addDisplay(DisplayType & disp)
  {
    dispList_.push_back( &disp );
    if(!hmesh_) hmesh_ = setupHmesh();
    if(disp.hasData())
    {
      // get functions data vector of given display
      std::vector < DUNE_FDATA * > & vec = disp.getFdataVec();

      // only copy functions for the first partition, because
      // all functions should be the same on every partition
      if(vec.size() > vecFdata_.size())
      {
        vecFdata_.resize( vec.size() );

        for(unsigned int n = 0; n < vecFdata_.size(); n++)
        {
          vecFdata_[n] = new DUNE_FDATA () ;
          assert( vecFdata_[n] );

          std::memcpy(vecFdata_[n],vec[n],sizeof(DUNE_FDATA));

          // we only need the two new functions for evaluation
          vecFdata_[n]->evalCoord = this->evalCoordWrap;
          vecFdata_[n]->evalDof = this->evalDofWrap;

          // not needed here
          vecFdata_[n]->comp = 0;

          // add function data to hmesh
          GrapeInterface<dim,dimworld>::addDataToHmesh(hmesh_,vecFdata_[n],&func_real);
        }
      }
    }
  }

  template<class DisplayType>
  inline void CombinedGrapeDisplay<DisplayType>::
  addMyMeshToGlobalTimeScene(double time, int proc)
  {
    if(!hmesh_) hmesh_ = setupHmesh();
    GrapeInterface<dim,dimworld>::addHmeshToGlobalTimeScene(time,this->getHmesh(),proc);
  }

  template<class DisplayType>
  inline void * CombinedGrapeDisplay<DisplayType>::setupHmesh()
  {
    int noe = 0, nov = 0;
    int maxlevel = 0;

    enditer_ = dispList_.end();
    for(grditer_ = dispList_.begin(); grditer_ != enditer_; ++grditer_)
    {
      const GridType & grid = (*grditer_)->getGrid();
      maxlevel = std::max( maxlevel, grid.maxLevel());
      noe += grid.leafIndexSet().size(0);
      nov += grid.leafIndexSet().size(dim);
    }

    hel_.display = (void *) this;
    hel_.liter = NULL;
    hel_.enditer = 0;
    hel_.hiter = NULL;
    hel_.actElement = NULL;

    DUNE_DAT * dune = &dune_;

    dune->first_macro = &first_mac;
    dune->next_macro  = &next_mac;

    dune->first_child = &fst_child;
    dune->next_child  = &nxt_child;

    dune->copy         = 0; // no copy at the moment
    dune->wtoc         = wtoc;
    dune->ctow         = ctow;
    dune->check_inside = check_inside;

    // set method to select iterators
    dune->setIterationModus = &setIterationModus;

    dune->all          = &hel_;
    dune->partition    = __MaxPartition-1;

    dune->iteratorType          = g_LeafIterator;
    dune->partitionIteratorType = g_All_Partition;

    /* return hmesh with no data */
    return GrapeInterface<dim,dimworld>::hmesh(NULL,noe,nov,maxlevel,NULL,dune);
  }

} // end namespace Dune
