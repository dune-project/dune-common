// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include "geldesc.hh"

namespace Dune
{

  //*******************************************************************
  //
  //  Routines for evaluation of the data
  //
  //*******************************************************************
  // evaluate scalar functions, means val has length 1
  template <class GridType, class DiscreteFunctionType>
  inline void EvalDiscreteFunctions<GridType,DiscreteFunctionType>::
  evalScalar (EntityType &en, int geomType,
              DiscreteFunctionType & func, LocalFunctionType &lf,
              const int * comp, int localNum, double * val)
  {
    enum { numberOfComp = DiscreteFunctionType::FunctionSpaceType::DimRange };
    enum { polynomialOrder = FunctionSpaceType :: polynomialOrder };
    static const GrapeLagrangePoints<ctype,dim,dimworld,polynomialOrder> lagrangePoints;
    const FieldVector<ctype,dim> & localPoint =
      lagrangePoints.getPoint(geomType,polynomialOrder,localNum);

    static RangeType tmp_;
    // evaluate local function on local lagrange point
    lf.evaluateLocal(en,localPoint,tmp_);

    // dimval == 1 here
    // 0 because we only have one value (dimVal == 1)
    val[0] = tmp_[comp[0]];
    return;
  }

  template <class GridType, class DiscreteFunctionType>
  inline void EvalDiscreteFunctions<GridType,DiscreteFunctionType>::
  evalVector (EntityType &en, int geomType,
              DiscreteFunctionType & func, LocalFunctionType &lf,
              const int * comp, int vlength , int localNum, double * val)
  {
    enum { dim = EntityType::dimension };
    assert( comp );
    // dimval == dimension here
    // in that case we have only one dof that has to be returned for all
    // corners , kind of hack, but works for the moment
    if(func.getFunctionSpace().polynomOrder() == 0)
    {
      enum { polynomialOrder = FunctionSpaceType :: polynomialOrder };
      static const GrapeLagrangePoints<ctype,dim,dimworld,polynomialOrder> lagrangePoints;
      const FieldVector<ctype,dim> & localPoint =
        lagrangePoints.getPoint(geomType,polynomialOrder,localNum);

      static RangeType tmp_;
      // evaluate local function on local lagrange point
      lf.evaluateLocal(en,localPoint,tmp_);

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

  template <class GridType, class DiscreteFunctionType>
  inline void EvalDiscreteFunctions<GridType,DiscreteFunctionType>::
  evalDofNow (EntityType &en, int geomType, DUNE_FDATA *df , int localNum, double * val)
  {
    assert( df );
    assert( df->discFunc );

    DiscreteFunctionType & func = *((DiscreteFunctionType *)  ( df->discFunc));

    typedef typename DiscreteFunctionType::LocalFunctionType LocalFuncType;

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
        evalScalar(en,geomType, func,lf,comp,localNum,val);
        return;
      }

      case dim :
      {
        evalVector(en,geomType,func,lf,df->comp,dimVal,localNum,val);
        return;
      }
      default :
      {
        assert(false);
        evalVector(en,geomType,func,lf,df->comp,dimVal,localNum,val);
        return;
      }
      }
      return;
    }
  }

  template<class GridType, class DiscreteFunctionType>
  inline void EvalDiscreteFunctions<GridType,DiscreteFunctionType>::
  evalDof (DUNE_ELEM *he, DUNE_FDATA *df,int localNum, double * val)
  {
    enum { dim = GridType::dimension };
    enum { dimRange = DiscreteFunctionType::FunctionSpaceType::DimRange };
    void *iter = he->actElement;
    int geomType = he->type;
    if(iter == he->liter)
    {
      if(he->isLeafIterator == 1)
      {
        typedef typename GridType::template Codim<0>::LeafIterator LeafIt;
        LeafIt *it = (LeafIt *) he->liter;
        evalDofNow( *it[0],geomType,df,localNum,val);
        return;
      }
      else
      {
        typedef typename GridType::Traits::template Codim<0>::LevelIterator LevIt;
        LevIt *it = (LevIt *) he->liter;
        evalDofNow(*it[0],geomType,df,localNum,val);
        return;
      }
    }
    else if(iter == he->hiter)
    {
      typedef typename GridType::Traits::template Codim<0>::Entity::HierarchicIterator HierIt;
      HierIt *it = (HierIt *) he->hiter;
      evalDofNow(*it[0],geomType,df,localNum,val);
      return;
    }
    else
    {
      std::cerr << "ERROR: No Iterator in evalDof! file = " << __FILE__ << ", line = " << __LINE__ << "\n";
      abort();
    }
    return;
  }

  template<class GridType, class DiscreteFunctionType>
  inline void EvalDiscreteFunctions<GridType,DiscreteFunctionType>::
  evalCoordNow(EntityType &en, DUNE_FDATA *df , const double *coord, double * val)
  {
    enum { dim = GridType::dimension };

    assert( df );
    assert( df->discFunc );
    DiscreteFunctionType & func = *((DiscreteFunctionType *) ( df->discFunc));

    typedef typename DiscreteFunctionType::LocalFunctionType LocalFunctionType;

    if(coord)
    {
      // get local function
      LocalFunctionType lf = func.localFunction( en );

      static DomainType domTmp_;

      // convert double to FieldVector
      for(int i=0; i<dim; i++) domTmp_[i] = coord[i];

      static RangeType tmp_;
      // evaluate local function on local (on reference element)
      // point == domTmp
      lf.evaluateLocal( en , domTmp_ , tmp_);

      int dimVal = df->dimVal;
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
  template<class GridType, class DiscreteFunctionType>
  inline void EvalDiscreteFunctions<GridType,DiscreteFunctionType>::
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
        evalCoordNow(*it[0],df,coord,val);
        return;
      }
      else
      {
        typedef typename GridType::Traits::template Codim<0>::LevelIterator LevIt;
        LevIt *it = (LevIt *) he->liter;
        evalCoordNow(*it[0],df,coord,val);
        return;
      }
    }
    else if(iter == he->hiter)
    {
      typedef typename GridType::Traits::template Codim<0>::Entity::HierarchicIterator HierIt;
      HierIt *it = (HierIt *) he->hiter;
      evalCoordNow(*it[0],df,coord,val);
      return;
    }
    else
    {
      std::cerr << "ERROR: No Iterator in evalCoord! file = " << __FILE__ << ", line = " << __LINE__ << "\n";
      abort();
    }
    return;
  }


  //****************************************************************
  //
  // --GrapeDataDisplay, GrapeDataDisplay for given grid
  //
  //****************************************************************
  template <class GridType>
  inline GrapeDataDisplay<GridType>::
  GrapeDataDisplay (const GridType &grid) :
    GrapeGridDisplay < GridType > (grid) , vecFdata_ (0)
  {}

  template <class GridType>
  inline GrapeDataDisplay<GridType>::
  GrapeDataDisplay (const GridType &grid, const int myrank ) :
    GrapeGridDisplay < GridType > (grid,myrank) , vecFdata_ (0)
  {}

  template <class GridType>
  inline GrapeDataDisplay<GridType>::~GrapeDataDisplay()
  {
    typedef typename GridType::Traits::template Codim<0>::LevelIterator LevIter;
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
  // --GrapeDataDisplay, Some Subroutines needed in display
  //
  //****************************************************************

  template<class GridType>
  inline void GrapeDataDisplay<GridType>::
  func_real (DUNE_ELEM *he , DUNE_FDATA * fe,int ind, const double *coord, double *val )
  {
    if(coord)
    {
      fe->evalCoord(he,fe,coord,val);
    }
    else
    {
      fe->evalDof(he,fe,ind,val);
    }
    return;
  }

  template<class GridType>
  template<class DiscFuncType>
  inline void GrapeDataDisplay<GridType>::
  dataDisplay(DiscFuncType &func, bool vector)
  {
    /* add function data */
    this->addData(func,"myFunc",0.0,vector);
    /* display mesh */
    GrapeInterface<dim,dimworld>::handleMesh ( this->hmesh_ );
    return ;
  }


  template<class GridType>
  template<class DiscFuncType>
  inline void GrapeDataDisplay<GridType>::
  addData(DiscFuncType &func , const char * name , double time , bool vector)
  {
    int comp[dim];
    for(int i=0; i<dim; i++) comp[i] = i;
    DATAINFO dinf = { name , name , 0 , (vector) ? dim : 1 , (int *) &comp };
    addData(func,&dinf,time);
  }

  template<class GridType>
  template<class VectorPointerType>
  inline void GrapeDataDisplay<GridType>::
  displayVector(const VectorPointerType * vector)
  {
    /*
       typedef typename FunctionSpaceType :: Traits :: GridPartType GridPartType;
       GridPartType part (this->grid_, this->leafset_ );
       FunctionSpaceType space ( part );

       DiscFuncType func ( "my data" , space , vector );
       dataDisplay(func,false);
     */
  }

  template<class GridType>
  template<class DiscFuncType>
  inline void GrapeDataDisplay<GridType>::
  addData(DiscFuncType &func , const DATAINFO * dinf, double time )
  {
    typedef typename DiscFuncType::FunctionSpaceType FunctionSpaceType;
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
      int num = (int) FunctionSpaceType::DimRange;
      if(vector) num = 1;

      vecFdata_.resize(size+num);
      for(int n=size; n < size+num; n++)
      {
        vecFdata_[n] = new DUNE_FDATA ();

        // set the rigth evaluation functions
        vecFdata_[n]->evalDof =
          EvalDiscreteFunctions<GridType,DiscFuncType>::evalDof;

        vecFdata_[n]->evalCoord =
          EvalDiscreteFunctions<GridType,DiscFuncType>::evalCoord;

        vecFdata_[n]->mynum = n;
        vecFdata_[n]->name = name;
        vecFdata_[n]->allLevels = 0;

        vecFdata_[n]->discFunc = (void *) &func;
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
