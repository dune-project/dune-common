// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
namespace Dune
{
  //****************************************************************
  //
  // --FunctionSpace, FunctionSpace for a given grid and DOF
  //
  //****************************************************************
  template<class Grid, BaseType basetype>
  inline FunctionSpace<Grid,basetype>::FunctionSpace(Grid *grid) : grid_(grid)
  {
    name_ = BaseName[basetype];

    dsmm_ = new DefaultDSMM (100000,4);
    ghmm_ = new DefaultGHMM ();
    ssbm_ = new ScalarSparseBLASManager(dsmm_,ghmm_,100,100);

    //! could be, that gridsize calculation is expensive
    gridSize_ = grid_->size(-1,0);

    makeBase();
  }

  template<class Grid, BaseType basetype>
  inline FunctionSpace<Grid,basetype>::~FunctionSpace()
  {
    std::cout << "Cleaning FunctionSpace \n";

    delete mapElNumber_;

    delete dsmm_;
    delete ghmm_;
    delete ssbm_;

  }

  template<class Grid,BaseType basetype>
  inline void FunctionSpace<Grid,basetype>::makeMapVec()
  {

    // map global element number to local element number
    int numDofGrid = grid_->hiersize(-1,0);
    mapElNumber_ = new int [numDofGrid];

    for(int i=0; i<numDofGrid; i++)
      mapElNumber_[i] = 0;

    int level = -1;
    LevelIterator endit = grid_->lend<0>(level);

    // remember which local number each element has
    int i=0;
    for(LevelIterator it = grid_->lbegin<0>(level); it != endit; ++it)
    {
      mapElNumber_[it->index()] = i;
      i++;
    }

    //for(int i=0; i< numDofGrid; i++)
    //  std::cout << "mapNum " << mapElNumber_[i] << endl;
  }

  template<class Grid,BaseType basetype>
  inline void FunctionSpace<Grid,basetype>::makeMapVecLag()
  {
    // der mapNumberAbschnitt
    int numDofGrid = numDof * grid_->hiersize(-1,0);

    mapElNumber_ = new int [numDofGrid];
    for(int i=0; i< numDofGrid; i++)
      mapElNumber_[i] = 0;

    int level = -1;
    LevelIterator endit = grid_->lend<0>(level);

    for(LevelIterator it = grid_->lbegin<0>(level); it != endit; ++it)
      doMapping(*it);

    //  for(int i=0; i< numDofGrid; i++)
    //    std::cout << "mapNumLag " << mapElNumber_[i] << endl;
  }

  template<class Grid,BaseType basetype> template <class Entity>
  inline void FunctionSpace<Grid,basetype>::doMapping(Entity &e)
  {
    enum { mydim = Entity::dimension };
    int numVx = e.count<mydim>();

    for(int i=0; i<numVx; i++)
    {
      mapElNumber_[mapDefault(e.index(),i)] = (e.entity<mydim>(i))->index();
    }
  }

  template<class Grid,BaseType basetype>
  inline void FunctionSpace<Grid,basetype>::makeBase()
  {
    std::cout << "make function space base functions type: ";
    switch (basetype)
    {
    case Const :
    {
      std::cout << "const ! \n";
      baseType_ = new LOCALBASE ();

      dimOfFunctionSpace_ = grid_->size(-1,0);

      for(int i=0; i<numDof; i++)
        localBase_(i) = new BASEFUNC (baseType_->getBaseFunc(i));
      makeMapVec();
      break;
    }
    case LagrangeOne :
    {
      std::cout << "linear Lagrange ! \n";
      baseType_ = new LOCALBASE ();
      dimOfFunctionSpace_ = grid_->numberVertices();
      for(int i=0; i<numDof; i++)
        localBase_(i) = new BASEFUNC (baseType_->getBaseFunc(i));
      makeMapVecLag();
      break;
    }
    case DGOne :
    {
      std::cout << "linear DG ! \n";
      baseType_ = new LOCALBASE ();
      dimOfFunctionSpace_ = numDof*grid_->size(-1,0);
      for(int i=0; i<numDof; i++)
        localBase_(i) = new BASEFUNC (baseType_->getBaseFunc(i));
      makeMapVec();
      break;

    }
    default :
    {
      std::cout << "type of base not available yet! \n";
      abort();
    }
    }
  }

#if 0
  template<class Grid>
  inline void FunctionSpace<Grid,Const>::makeBase()
  {
    std::cout << "make default base! \n";
    baseType_ = new LOCALBASE ();

    dimOfFunctionSpace_ = grid_->hiersize(-1,0);
    for(int i=0; i<numDof; i++)
      localBase_(i) = new BASEFUNC (baseType_->getBaseFunc(i));
  }

  template<class Grid>
  inline void FunctionSpace<Grid,LagrangeOne>::makeBase()
  {
    baseType_ = new LOCALBASE ();
    dimOfFunctionSpace_ = numDof*grid_->size(-1,0);
    for(int i=0; i<numDof; i++)
      localBase_(i) = new BASEFUNC (baseType_->getBaseFunc(i));
  }

#endif

  template<class Grid, BaseType basetype>
  inline typename FunctionSpace<Grid,basetype>::BASEFUNC*
  FunctionSpace<Grid,basetype>::getLocalBaseFunc(int i)
  {
    return localBase_(i);
  }

  template<class Grid, BaseType basetype> template <class Entity>
  inline Vec<FunctionSpace<Grid,basetype>::dimrange>
  FunctionSpace<Grid,basetype>::map(Entity & el,VALTYPE *val, int dof)
  {
    double values;
    //! access to the ScalarVector
    val->Get(mapIndex(el.index(),dof),&values);

    Vec<dimrange> tmp (&values);
    return tmp;
  }

  template<class Grid, BaseType basetype> template <BaseType bt>
  inline int FunctionSpace<Grid,basetype>::mapper(int index,int dof)
  {
    return (gridSize_ * dof + mapElNumber_[index]);
  }

  template<class Grid, BaseType basetype>
  inline int FunctionSpace<Grid,basetype>::mapIndex(int index,int dof)
  {
    return mapper<basetype>(index,dof);
  }

  template<class Grid, BaseType basetype>
  inline int FunctionSpace<Grid,basetype>::mapDefault(int index,int dof)
  {
    return (gridSize_ * dof + index);
  }

  template<class Grid, BaseType basetype>
  inline int FunctionSpace<Grid,basetype>::
  dimOfFunctionSpace()
  {
    return dimOfFunctionSpace_;
  }


} // end namespace Dune
