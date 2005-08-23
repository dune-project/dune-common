// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
namespace Dune
{

  //****************************************************************
  //
  // --GrapeDataDisplay, GrapeDataDisplay for given grid
  //
  //****************************************************************
  template <class GridType, class DiscFuncType>
  inline GrapeDataDisplay<GridType,DiscFuncType>::GrapeDataDisplay (GridType &grid) :
    GrapeGridDisplay < GridType > (grid) , vecFdata_ (0) , quad_ (0)
  {
    createQuadrature();
  }

  template <class GridType, class DiscFuncType>
  inline GrapeDataDisplay<GridType,DiscFuncType>::
  GrapeDataDisplay (GridType &grid, const int myrank ) :
    GrapeGridDisplay < GridType > (grid,myrank) , vecFdata_ (0) , quad_ (0)
  {
    createQuadrature();
  }

  template <class GridType, class DiscFuncType>
  inline void GrapeDataDisplay<GridType,DiscFuncType>::createQuadrature ()
  {
    typedef typename GridType::Traits::template Codim<0>::LevelIterator LevIter;
    LevIter it    = this->grid_.template lbegin<0> (0);
    LevIter endit = this->grid_.template lend<0> (0);
    if(it != endit)
      quad_ = new QuadType ( *it );
  }

  //*******************************************************************
  //
  //  Routines for evaluation of the data
  //
  //*******************************************************************
  template <class GridType, class DiscFuncType>
  template <class EntityType, class LocalFunctionType >
  inline void GrapeDataDisplay<GridType,DiscFuncType>::
  evalCoord (EntityType &en, DiscFuncType &func , LocalFunctionType & lf ,
             int comp, const double *coord, double * val)
  {
    enum { dim = GridType::dimension };

    if(coord)
    {
      func.localFunction ( en , lf );

      int dimVal = func.getFunctionSpace().dimensionOfValue();
      for(int i=0; i<dim; i++) domTmp_[i] = coord[i];

      // evaluate local function on point coord == domTmp
      lf.evaluate( en , domTmp_ , tmp_);

      for(int i=0; i<dimVal; i++) val[i] = tmp_[i];
      return;
    }
    else
    {
      std::cerr << "ERROR: No Coord, are ye crazzy? file = " << __FILE__ << ", line = " << __LINE__ << "\n";
      val [0] = 0.0;
      return;
    }
  }


  template<class GridType, class DiscFuncType>
  inline void GrapeDataDisplay<GridType,DiscFuncType>::
  evalCoord (DUNE_ELEM *he, DUNE_FDATA *df, const double *coord, double * val)
  {
    enum { dim = GridType::dimension };
    typedef typename GridType::Traits::template Codim<0>::Entity EntityType;
    DiscFuncType *func = (DiscFuncType *)  ( df->discFunc);

    typedef typename DiscFuncType::LocalFunctionType LocalFuncType;
    LocalFuncType *lf = (LocalFuncType *) (df->lf);

    void *iter = he->actElement;
    if(iter == he->liter)
    {
      if(he->isLeafIterator)
      {
        typedef typename GridType::template Codim<0>::LeafIterator LeafIt;
        LeafIt *it = (LeafIt *) he->liter;
        evalCoord(*it[0],*func,*lf,df->component,coord,val);
        return;
      }
      else
      {
        typedef typename GridType::Traits::template Codim<0>::LevelIterator LevIt;
        LevIt *it = (LevIt *) he->liter;
        evalCoord(*it[0],*func,*lf,df->component,coord,val);
        return;
      }
    }
    else if(iter == he->hiter)
    {
      typedef typename GridType::Traits::template Codim<0>::Entity::HierarchicIterator HierIt;
      HierIt *it = (HierIt *) he->hiter;
      evalCoord(*it[0],*func,*lf,df->component,coord,val);
      return;
    }
    else
    {
      std::cerr << "ERROR: No Iterator in evalCoord! file = " << __FILE__ << ", line = " << __LINE__ << "\n";
      abort();
    }
    return;
  }

  template <class GridType, class DiscFuncType>
  template <class EntityType, class LocalFuncType>
  inline void GrapeDataDisplay<GridType,DiscFuncType>::
  evalVector (EntityType &en, DiscFuncType & func, LocalFuncType &lf,
              int comp , int localNum, double * val)
  {
    enum { dim = EntityType::dimension };
    // dimval == dimension here
    // in that case we have only one dof that has to be returned for all
    // corners , kind of hack, but works for the moment
    if(func.getFunctionSpace().polynomOrder() == 0)
    {
      assert(quad_);
      lf.evaluate(en,(*quad_),0,tmp_);
      for(int i=0; i<dim; i++) val[i] = tmp_[i];
      return;
    }
    else
    {
      std::cerr << "ERROR: evalVector for polOrd > 0 not implemented! file = " << __FILE__ << ", line = " << __LINE__ << "\n";
      abort();
    }
    return;
  }

  template <class GridType, class DiscFuncType>
  template <class EntityType, class LocalFuncType>
  inline void GrapeDataDisplay<GridType,DiscFuncType>::
  evalScalar (EntityType &en, DiscFuncType & func, LocalFuncType &lf,
              int comp , int localNum, double * val)
  {
    // dimval == 1 here
    enum { numberOfComp = DiscFuncType::FunctionSpaceType::DimRange };

    // in that case we have only one dof that has to be returned for all
    // corners , kind of hack, but works for the moment
    if(func.getFunctionSpace().polynomOrder() == 0)
    {
      val[0] = lf[comp];
      return;
    }
    else
    {
      // for linear data
      int num = ( localNum * numberOfComp ) + comp;

      switch ( en.geometry().type())
      {
      // check for quadrilaterals and hexahedrons
      case quadrilateral :
      {
        val[0] = lf[mapElType<quadrilateral>(num)]; break;
      }
      case hexahedron   :
      { val[0] = lf[mapElType<hexahedron>(num)]; break; }
      default :
      {
        // i.e. triangles, tetrahedron and so on
        val[0] = lf[num];
        break;
      }
      }
    }
    return;
  }


  template <class GridType, class DiscFuncType>
  template <class EntityType>
  inline void GrapeDataDisplay<GridType,DiscFuncType>::
  evalDof (EntityType &en, DUNE_FDATA *df , int localNum, double * val)
  {
    //typedef typename GridType::Traits::template Codim<0>::Entity EntityType;

    assert( df->discFunc );
    DiscFuncType & func = *((DiscFuncType *)  ( df->discFunc));

    assert( df->lf );
    typedef typename DiscFuncType::LocalFunctionType LocalFuncType;
    LocalFuncType & lf = *((LocalFuncType *) (df->lf));

    enum { dim = EntityType::dimension };
    {
      int comp = df->component;
      func.localFunction ( en , lf );
      int dimVal = df->dimVal;
      switch (dimVal)
      {
      case 1 :
      {
        evalScalar(en,func,lf,comp,localNum,val);
        return;
      }

      case dim :
      {
        evalVector(en,func,lf,comp,localNum,val);
        return;
      }
      default :
      {
        evalVector(en,func,lf,comp,localNum,val);
        return;
      }
        //{
        //  std::cerr << "ERROR: No evalMethod for data set! file = " << __FILE__ << ", line = " << __LINE__ << "\n";
        //  abort();
        //}

      }
      return;
    }
  }

  template<class GridType, class DiscFuncType>
  inline void GrapeDataDisplay<GridType,DiscFuncType>::
  evalDof (DUNE_ELEM *he, DUNE_FDATA *df,int localNum, double * val)
  {
    enum { dim = GridType::dimension };
    enum { dimRange = DiscFuncType::FunctionSpaceType::DimRange };
    void *iter = he->actElement;
    if(iter == he->liter)
    {
      if(he->isLeafIterator == 1)
      {
        typedef typename GridType::template Codim<0>::LeafIterator LeafIt;
        LeafIt *it = (LeafIt *) he->liter;
        evalDof( *it[0],df,localNum,val);
        return;
      }
      else
      {
        typedef typename GridType::Traits::template Codim<0>::LevelIterator LevIt;
        LevIt *it = (LevIt *) he->liter;
        evalDof(*it[0],df,localNum,val);
        return;
      }
    }
    else if(iter == he->hiter)
    {
      typedef typename GridType::Traits::template Codim<0>::Entity::HierarchicIterator HierIt;
      HierIt *it = (HierIt *) he->hiter;
      evalDof(*it[0],df,localNum,val);
      return;
    }
    else
    {
      std::cerr << "ERROR: No Iterator in evalDof! file = " << __FILE__ << ", line = " << __LINE__ << "\n";
      abort();
    }
    return;
  }

  //****************************************************************
  //
  // --GrapeDataDisplay, Some Subroutines needed in display
  //
  //****************************************************************

  template<class GridType, class DiscFuncType>
  inline void GrapeDataDisplay<GridType,DiscFuncType>::
  func_real (DUNE_ELEM *he , DUNE_FDATA * fe,int ind, const double *coord, double *val )
  {
    MyDisplayType * disp = (MyDisplayType *) he->display;
    if(coord)
      disp[0].evalCoord(he,fe,coord,val);
    else
      disp[0].evalDof(he,fe,ind,val);

    return;
  }

  template<class GridType, class DiscFuncType>
  inline void GrapeDataDisplay<GridType,DiscFuncType>::dataDisplay(DiscFuncType &func)
  {
    /* add function data */
    this->addData(func,"myFunc",0.0);
    /* display mesh */
    GrapeInterface<dim,dimworld>::handleMesh ( this->hmesh_ );
    return ;
  }


  template<class GridType, class DiscFuncType>
  inline void GrapeDataDisplay<GridType,DiscFuncType>::
  addData(DiscFuncType &func , const char *name , double time , bool vector )
  {
    typedef typename DiscFuncType::LocalFunctionType LocalFuncType;

    bool already=false;
    int size = vecFdata_.size();

    // add function wether is exists or not
    if(!already)
    {
      int num = (int) DiscFuncType::FunctionSpace::DimRange;
      if(vector) num = 1;

      vecFdata_.resize(size+num);
      for(int n=size; n < size+num; n++)
      {
        vecFdata_[n] = new DUNE_FDATA ();

        vecFdata_[n]->mynum = n;
        vecFdata_[n]->name = name;
        vecFdata_[n]->allLevels = 0;

        vecFdata_[n]->discFunc = (void *) &func;
        vecFdata_[n]->dimVal   = func.getFunctionSpace().dimensionOfValue();
        if(vector) vecFdata_[n]->dimVal = DiscFuncType::FunctionSpace::DimRange;
        vecFdata_[n]->lf = (void *) new LocalFuncType ( func.newLocalFunction() );
        vecFdata_[n]->polyOrd = func.getFunctionSpace().polynomOrder();
        vecFdata_[n]->continuous = (func.getFunctionSpace().continuous() == true ) ? 1 : 0;
        vecFdata_[n]->component = n-size;
        vecFdata_[n]->compName = n-size;

        GrapeInterface<dim,dimworld>::addDataToHmesh(this->hmesh_,vecFdata_[n],&func_real);
      }
    }
  }

} // end namespace Dune
