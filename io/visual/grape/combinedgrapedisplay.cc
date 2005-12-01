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
  first_leaf (DUNE_ELEM * he)
  {
    assert( !dispList_.empty() );
    grditer_ = dispList_.begin();
    enditer_ = dispList_.end();
    disp_ = 0;
    if(grditer_ != enditer_)
    {
      disp_ = *grditer_;
      GrapeInterface<dim,dimworld>::setThread( disp_->myRank() );
      return disp_->first_leaf(he);
    }
    return 0;
  }


  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  next_leaf (DUNE_ELEM * he)
  {
    int ret = 0;
    if( disp_ )
    {
      ret = disp_->next_leaf(he);
      if(!ret)
      {
        ++grditer_;
        disp_ = 0;
        if(grditer_ != enditer_)
        {
          disp_ = *grditer_;
          GrapeInterface<dim,dimworld>::setThread( disp_->myRank() );
          return disp_->first_leaf(he);
        }
      }
    }
    return ret;
  }


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
      return disp_->first_macro(he);
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
      ret = disp_->next_macro(he);
      if(!ret)
      {
        ++grditer_;
        disp_ = 0;
        if(grditer_ != enditer_)
        {
          disp_ = *grditer_;
          GrapeInterface<dim,dimworld>::setThread( disp_->myRank() );
          return disp_->first_macro(he);
        }
      }
    }
    return ret;
  }

  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  first_child(DUNE_ELEM * he)
  {
    if(disp_) return disp_->first_child(he);
    else
      return 0;
  }


  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  next_child(DUNE_ELEM * he)
  {
    if( disp_ )
      return disp_->next_child(he);
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
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].first_macro(he);
  }


  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  next_mac (DUNE_ELEM * he)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].next_macro(he);
  }

  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  fst_leaf (DUNE_ELEM * he)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].first_leaf(he);
  }


  template<class DisplayType>
  inline int CombinedGrapeDisplay<DisplayType>::
  nxt_leaf (DUNE_ELEM * he)
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    return disp[0].next_leaf(he);
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
    MyDisplayType * disp = (MyDisplayType *) he->display;
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
      noe += grid.leafIndexSet().size(0,unknown);
      nov += grid.leafIndexSet().size(dim,unknown);
    }

    hel_.display = (void *) this;
    hel_.liter = NULL;
    hel_.hiter = NULL;
    hel_.actElement = NULL;

    /* return hmesh with no data */
    return GrapeInterface<dim,dimworld>::hmesh(fst_leaf,nxt_leaf,first_mac,next_mac,fst_child,nxt_child,
                                               NULL,check_inside,wtoc,ctow,NULL,noe,nov,maxlevel,__MaxPartition-1,&hel_,NULL);
  }

} // end namespace Dune
