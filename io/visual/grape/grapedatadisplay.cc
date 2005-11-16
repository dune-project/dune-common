// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include "geldesc.hh"

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

  template <class GridType, class DiscFuncType>
  inline GrapeDataDisplay<GridType,DiscFuncType>::~GrapeDataDisplay()
  {
    typedef typename GridType::Traits::template Codim<0>::LevelIterator LevIter;
    if(quad_) delete quad_;
    for(unsigned int i=0 ; i<vecFdata_.size(); i++)
    {
      typedef typename DiscFuncType :: LocalFunctionType LFType;
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

  //*******************************************************************
  //
  //  Routines for evaluation of the data
  //
  //*******************************************************************
  template <class GridType, class DiscFuncType>
  template <class EntityType>
  inline void GrapeDataDisplay<GridType,DiscFuncType>::
  evalCoord(EntityType &en, DUNE_FDATA *df , const double *coord, double * val)
  {
    enum { dim = GridType::dimension };

    assert( df );
    assert( df->discFunc );
    DiscFuncType & func = *((DiscFuncType *)  ( df->discFunc));

    typedef typename DiscFuncType::LocalFunctionType LocalFuncType;

    int dimVal = df->dimVal;
    if(coord)
    {
      LocalFuncType lf = func.localFunction( en );

      for(int i=0; i<dim; i++) domTmp_[i] = coord[i];

      // evaluate local function on local (on reference element)
      // point == domTmp
      lf.evaluateLocal( en , domTmp_ , tmp_);

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

    void *iter = he->actElement;
    // cast pointer back to the iterator we have
    if(iter == he->liter)
    {
      if(he->isLeafIterator)
      {
        typedef typename GridType::template Codim<0>::LeafIterator LeafIt;
        LeafIt *it = (LeafIt *) he->liter;
        evalCoord(*it[0],df,coord,val);
        return;
      }
      else
      {
        typedef typename GridType::Traits::template Codim<0>::LevelIterator LevIt;
        LevIt *it = (LevIt *) he->liter;
        evalCoord(*it[0],df,coord,val);
        return;
      }
    }
    else if(iter == he->hiter)
    {
      typedef typename GridType::Traits::template Codim<0>::Entity::HierarchicIterator HierIt;
      HierIt *it = (HierIt *) he->hiter;
      evalCoord(*it[0],df,coord,val);
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
              const int * comp, int vlength , int localNum, double * val)
  {
    enum { dim = EntityType::dimension };
    assert( comp );
    // dimval == dimension here
    // in that case we have only one dof that has to be returned for all
    // corners , kind of hack, but works for the moment
    if(func.getFunctionSpace().polynomOrder() == 0)
    {
      assert(quad_);
      lf.evaluate(en,(*quad_),0,tmp_);
      for(int i=0; i<vlength; i++)
      {
        val[i] = tmp_[comp[i]];
      }
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
              const int * comp , int localNum, double * val)
  {
    // dimval == 1 here
    enum { numberOfComp = DiscFuncType::FunctionSpaceType::DimRange };

    // in that case we have only one dof that has to be returned for all
    // corners , kind of hack, but works for the moment
    if(func.getFunctionSpace().polynomOrder() == 0)
    {
      val[0] = lf[comp[0]];
      return;
    }
    else if(func.getFunctionSpace().polynomOrder() == 1)
    {
      // for linear data
      // needs to be implemented more efficiently
      int num = ( localNum * numberOfComp ) + comp[0];
      int geomType = convertToGrapeType ( en.geometry().type() , dim );
      // see grapegrid.display.hh and geldesc.hh for definitions
      int renum = mapDune2GrapeVertex(geomType,num);
      val[0] = lf[renum];
    }
    else
    {
      std::cerr << "ERROR: GrapeDataDisplay::evalScalar not implemented for higher polynom degrees! in:" << __FILE__ << " line: " << __LINE__ << std::endl;
      abort();
    }
    return;
  }


  template <class GridType, class DiscFuncType>
  template <class EntityType>
  inline void GrapeDataDisplay<GridType,DiscFuncType>::
  evalDof (EntityType &en, DUNE_FDATA *df , int localNum, double * val)
  {
    assert( df );
    assert( df->discFunc );

    DiscFuncType & func = *((DiscFuncType *)  ( df->discFunc));

    typedef typename DiscFuncType::LocalFunctionType LocalFuncType;

    enum { dim = EntityType::dimension };
    {
      const int * comp = df->comp;
      assert( comp );

      LocalFuncType lf = func.localFunction( en );

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
        evalVector(en,func,lf,df->comp,dimVal,localNum,val);
        return;
      }
      default :
      {
        assert(false);
        //evalVector(en,func,lf,df->comp,dimVal,localNum,val);
        return;
      }
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
  inline void GrapeDataDisplay<GridType,DiscFuncType>::dataDisplay(DiscFuncType &func, bool vector)
  {
    /* add function data */
    this->addData(func,"myFunc",0.0,vector);
    /* display mesh */
    GrapeInterface<dim,dimworld>::handleMesh ( this->hmesh_ );
    return ;
  }


  template<class GridType, class DiscFuncType>
  inline void GrapeDataDisplay<GridType,DiscFuncType>::
  addData(DiscFuncType &func , const char * name , double time , bool vector)
  {
    int comp[dim];
    for(int i=0; i<dim; i++) comp[i] = i;
    DATAINFO dinf = { name , name , 0 , (vector) ? dim : 1 , (int *) &comp };
    addData(func,&dinf,time);
  }

  template<class GridType, class DiscFuncType>
  inline void GrapeDataDisplay<GridType,DiscFuncType>::
  addData(DiscFuncType &func , const DATAINFO * dinf, double time )
  {
    typedef typename DiscFuncType::LocalFunctionType LocalFuncType;
    assert(dinf);
    const char * name = dinf->name;
    assert( dinf->dimVal > 0);
    bool vector = (dinf->dimVal > 1) ? true : false;

    bool already=false;
    int size = vecFdata_.size();

    // add function wether is exists or not
    if(!already)
    {
      int num = (int) DiscFuncType::FunctionSpaceType::DimRange;
      if(vector) num = 1;

      vecFdata_.resize(size+num);
      for(int n=size; n < size+num; n++)
      {
        vecFdata_[n] = new DUNE_FDATA ();

        vecFdata_[n]->mynum = n;
        vecFdata_[n]->name = name;
        vecFdata_[n]->allLevels = 0;

        vecFdata_[n]->discFunc = (void *) &func;
        //vecFdata_[n]->lf = (void *) new LocalFuncType ( func.newLocalFunction() );
        vecFdata_[n]->polyOrd = func.getFunctionSpace().polynomOrder();
        vecFdata_[n]->continuous = (func.getFunctionSpace().continuous() == true ) ? 1 : 0;
        if(vecFdata_[n]->polyOrd == 0) vecFdata_[n]->continuous = 0;

        int dimVal = dinf->dimVal;
        int * comp = new int [dimVal];
        vecFdata_[n]->comp = comp;
        if(vector)
        {
          for(int j=0; j<dimVal; j++) comp[j] = dinf->comp[j];
          vecFdata_[n]->compName = -1;
        }
        else
        {
          comp[0] = n-size;
          vecFdata_[n]->compName = n-size;
        }
        vecFdata_[n]->dimVal = dimVal;
        GrapeInterface<dim,dimworld>::addDataToHmesh(this->hmesh_,vecFdata_[n],&func_real);
      }
    }
  }

} // end namespace Dune
