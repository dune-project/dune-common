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
  inline FunctionSpace<Grid,basetype>::FunctionSpace(Grid *grid, int level) :
    grid_(grid) , level_(level)
  {
    name_ = BaseName[basetype];

    dsmm_ = new DefaultDSMM (80*256*256,32);
    ghmm_ = new DefaultGHMM ();
    ssbm_ = new ScalarSparseBLASManager(dsmm_,ghmm_,100,100);

    //! could be, that gridsize calculation is expensive
    gridSize_ = grid_->size(-1,0);

    // is allocated later
    mapElNumber_ = NULL;

    makeBase();
  }

  template<class Grid, BaseType basetype>
  inline FunctionSpace<Grid,basetype>::~FunctionSpace()
  {
    std::cout << "Cleaning FunctionSpace ... ";
    if(mapElNumber_ ) delete mapElNumber_;
    if(ssbm_) delete ssbm_;
    if(dsmm_) delete dsmm_;
    if(ghmm_) delete ghmm_;
    std::cout << "done! \n";
  }

  template<class Grid,BaseType basetype>
  inline void FunctionSpace<Grid,basetype>::makeMapVec()
  {

    // map global element number to local element number
    int numDofGrid = grid_->hiersize(-1,0);
    mapElNumber_ = new int [numDofGrid];

    for(int i=0; i<numDofGrid; i++)
      mapElNumber_[i] = 0;

    LevelIterator endit = grid_->lend<0>(level_);

    // remember which local number each element has
    int i=0;
    for(LevelIterator it = grid_->lbegin<0>(level_); it != endit; ++it)
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
    int numDofGrid = numDof * grid_->hiersize(level_,0);

    mapElNumber_ = new int [numDofGrid];
    for(int i=0; i< numDofGrid; i++)
      mapElNumber_[i] = 0;

    LevelIterator endit = grid_->lend<0>(level_);

    for(LevelIterator it = grid_->lbegin<0>(level_); it != endit; ++it)
      doMapping(*it);

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

      dimOfFunctionSpace_ = grid_->size(level_,0);

      for(int i=0; i<numDof; i++)
        localBase_(i) = new BASEFUNC (
          baseType_->getBaseFunc(i),
          baseType_->getDrv1st(i),
          baseType_->getDrv2nd(i)
          );
      makeMapVec();
      break;
    }
    case LagrangeOne :
    {
      std::cout << "linear Lagrange ! \n";
      baseType_ = new LOCALBASE ();
      dimOfFunctionSpace_ = grid_->numberVertices();
      for(int i=0; i<numDof; i++)
        localBase_(i) = new BASEFUNC (
          baseType_->getBaseFunc(i),
          baseType_->getDrv1st(i),
          baseType_->getDrv2nd(i)
          );
      //makeMapVecLag();
      break;
    }
    case DGOne :
    {
      std::cout << "linear DG ! \n";
      baseType_ = new LOCALBASE ();
      dimOfFunctionSpace_ = numDof*grid_->size(level_,0);
      for(int i=0; i<numDof; i++)
        localBase_(i) = new BASEFUNC (
          baseType_->getBaseFunc(i),
          baseType_->getDrv1st(i),
          baseType_->getDrv2nd(i)
          );
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
    val->Get(mapIndex(el,dof),&values);

    Vec<dimrange> tmp (&values);
    return tmp;
  }

#if 0
  template<class Grid, BaseType basetype> template <class Entity, BaseType bt>
  inline int FunctionSpace<Grid,basetype>::mapper(Entity &e,int index,int dof)
  {
    return (gridSize_ * dof + mapElNumber_[index]);
  }
#endif

  template<class Grid, BaseType basetype> template <class Entity>
  inline int FunctionSpace<Grid,basetype>::mapIndex(Entity &e,int dof)
  {
    switch (basetype)
    {
    case LagrangeOne :
    {
      // return global Vertex number
      enum { mydim = Entity::dimension };
      return (e.entity<mydim>(dof))->index();
    }
    default :
    {
      // map global element number to element number on level
      int index = mapElNumber_[e.index()];
      return (gridSize_ * dof + index);
    }
    }


  }

  template<class Grid, BaseType basetype>
  inline int FunctionSpace<Grid,basetype>::
  dimOfFunctionSpace()
  {
    return dimOfFunctionSpace_;
  }


} // end namespace Dune
