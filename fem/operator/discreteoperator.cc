// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
namespace Dune
{

  // comilation time check
  template <bool> struct CompileTimeChecker;
  template <> struct CompileTimeChecker<true> {};

  //***************************************************************************
  //
  //  --FV1stOrd Finite Volume scheme first order
  //  implements a special DiscretOperator
  //
  //***************************************************************************
  //
#if 0
  inline FV1stOrd::FV1stOrd()
  {
    helpVec_ = NULL;
    built = false;
  }

  inline FV1stOrd::~FV1stOrd()
  {
    std::cout << "Cleaning FV1stOrd \n";
    if(built) helpVec_->Free();
    delete helpVec_;
  }

  // resize od built the helpVec for the updates
  template <class DiscFunc>
  inline void FV1stOrd::assemble(DiscFunc &func, double dt)
  {
    // ssbm_ should fit to FV
    if(!helpVec_)
      helpVec_ = new ScalarVector(func.getFuncSpace()->ssbm_);

    helpVec_->Build(func.getFuncSpace()->dim(),func.getFuncSpace()->dim());

    dt_ = dt;
    built = true;
  }

  // do the FV
  template <class DiscFunc>
  inline void
  FV1stOrd::solve(DiscFunc &f)
  {
    if(!built)
    {
      std::cout << "assemble the FV1stord first! \n";
      abort();
    }
    DiscFunc::VALTYPE vec = f.getDofVec();
    int dim = f.getFuncSpace()->dim();

    for(int i=0; i<dim; i++)
    {
      double val=0.0;
      vec.Get(i,&val);
      helpVec_->Put(i,&val);
      val = 7.0;
      vec.Put(i,&val);
    }

    return;
  }

  // do the FV, i.e. call solve
  template <class DiscFunc>
  inline DiscFunc&
  FV1stOrd::operator() (DiscFunc &f)
  {
    solve(f);
    return f;
  }

  // finalize, i.e. calc u^n+1
  template <class DiscFunc>
  inline void FV1stOrd::finalize(DiscFunc &f)
  {
    helpVec_->Free();
    built = false;
  }
#endif

  //***************************************************************************
  //
  //  --LinFEM Linear Finite Elements
  //  implements a special DiscretOperator
  //
  //***************************************************************************
  inline LinFEM::LinFEM()
  {
    helpVec_ = NULL;
    matrix_ = NULL;
    built = false;
  }

  inline LinFEM::~LinFEM()
  {
    std::cout << "Cleaning LinFEM \n";
    if(built) remove();
    if(helpVec_) delete helpVec_;
    if(matrix_) delete matrix_;
  }

  inline void LinFEM::remove()
  {
    helpVec_->Free();
    // matrix leer
    built = false;
  }

  template <class DiscFunc, class Entity>
  inline void LinFEM::boundaryValues(DiscFunc &f, Entity &el)
  {
    typedef typename DiscFunc::mySpace FuncSpace;

    FuncSpace &funcSpace = (*f.getFuncSpace());

    typedef typename Entity::NeighborIterator NeighIt;
    int numDof = el.count<0>();

    NeighIt endit = el.nend();
    for(NeighIt it = el.nbegin(); it != endit; ++it)
    {
      if(it.boundary())
      {
        int neigh = it.number_in_self();
        for(int i=1; i<numDof; i++)
        {
          int k = funcSpace.mapIndex(el,(neigh+i)%numDof);
          // unitRow unitCol for boundary
          matrix_->kroneckerKill(k,k);
          double val = 0.0;
          helpVec_->Put(k,&val);
        }
      }
    }
  }

  template <class DiscFunc>
  inline void LinFEM::assembleMatrix(DiscFunc &func, int level)
  {
    typedef typename DiscFunc::GRID GRID;
    typedef typename DiscFunc::LevelIterator LevelIterator;
    typedef typename DiscFunc::mySpace FuncSpace;

    enum { dimdef = 2};

    Vec<GRID::dimension> tmp(1.0);
    GRID &grid = (*func.getGrid());

    FuncSpace &funcSpace = (*func.getFuncSpace());
    LevelIterator endit = grid.lend<0>(level);

    for(LevelIterator it = grid.lbegin<0>(level); it != endit; ++it)
    {
      int vx = it->count<GRID::dimension>();
      int row = 0, col =0;

      Mat<2,2,double> inv = it->geometry().Jacobian_inverse(tmp);

      double vol = (it->geometry().integration_element(tmp));

      for(int p=0; p<vx; p++)
      {
        row = funcSpace.mapIndex((*it),p);
        Vec<FuncSpace::dimdef,double> vec1 =
          //grad[p];
          (funcSpace.getLocalBaseFunc(p)->evalFirstDrv (tmp))(0);
        vec1 = inv*vec1;
        double val = vec1*vec1;

        val *= vol;

        matrix_->add(row,row,val);

        int oth = (p+1)%3;
        val = vol;

        col = funcSpace.mapIndex((*it),oth);
        Vec<FuncSpace::dimdef,double> vec2 =
          //grad[oth];
          (funcSpace.getLocalBaseFunc(oth)->evalFirstDrv (tmp))(0);

        vec2 = inv*vec2;
        double fakeVal = vec1 * vec2;
        val *= fakeVal;

        matrix_->add(row,col,val);
        matrix_->add(col,row,val);
      }
    } //end for(Level...

    for(LevelIterator it = grid.lbegin<0>(level); it != endit; ++it)
    {
      boundaryValues(func,*it);
    }


  }

  // resize od built the helpVec for the updates
  template <class DiscFunc>
  inline void LinFEM::assemble(DiscFunc &func, double dt, double time)
  {
    // check that base type of function space is LagrangeOne at compile time
    //CompileTimeChecker<DiscFunc::mySpace::type == LagrangeOne> check;

    int thisdim = func.getFuncSpace()->dim();
    // ssbm_ should fit to FV

    if(myDim_ != thisdim)
    {
      myDim_ = thisdim;
      std::cout << "first removing \n";
      if(built) remove();
    }

    if(!built)
    {
      myDim_ = thisdim;
      if(!helpVec_)
        helpVec_ = new ScalarVector(func.getFuncSpace()->ssbm_);

      if(!matrix_)
        matrix_ = new SparseRowMatrix<double> (thisdim,thisdim,maxcol_,0.0);

      helpVec_->Build(thisdim,thisdim);
      func.getFuncSpace()->ssbm_->vcopy(helpVec_,&func.getDofVec());

      assembleMatrix(func,-1);

      dt_ = dt;
      built = true;
    }
  }

  // do the FV
  template <class DiscFunc>
  inline void LinFEM::solve(DiscFunc &f)
  {
    if(!built)
    {
      std::cout << "assemble the LinFEM first! \n";
      abort();
    }

    typedef typename DiscFunc::mySpace FuncSpace;
    typedef typename DiscFunc::VALTYPE VALTYPE;
    FuncSpace &fSpace = (*f.getFuncSpace());
    VALTYPE &vec = f.getDofVec();
    fSpace.ssbm_->vset(&vec,0.0);

    int thisdim = f.getFuncSpace()->dim();

    double *u = vec.getVec();
    double *b = helpVec_->getVec();

    CG((*matrix_),u,b,0.000001,10*thisdim,thisdim);

    return;
  }

  // do the FV, i.e. call solve
  template <class DiscFunc>
  inline DiscFunc& LinFEM::operator() (DiscFunc &f)
  {
    solve(f);
    return f;
  }

  // finalize, i.e. calc u^n+1
  template <class DiscFunc>
  inline void LinFEM::finalize(DiscFunc &f)
  {
    //matrix_->print("matrix");
  }



  //******************************************************************
  //
  // Time Solve FV
  //
  //******************************************************************
  template <class SpaceDiscr>
  inline TimeEulerFV<SpaceDiscr>::TimeEulerFV()
  {
    fv_ = new SpaceDiscr();
    built = false;
  }

  template <class SpaceDiscr>
  inline TimeEulerFV<SpaceDiscr>::~TimeEulerFV()
  {
    delete fv_;
    if(timegrid_)
      delete timegrid_;
    std::cout << "Cleaned TimeEulerFV \n";
  }


  template <class SpaceDiscr> template <class DiscFunc>
  inline void TimeEulerFV<SpaceDiscr>::
  assemble(DiscFunc &f, double startTime, double endTime)
  {
    // calc timestep size
    double dt = fv_->Getdt(); // hier fkt aufruf einbauen

    int steps = static_cast<int> ((endTime - startTime)/dt+1);

    timegrid_ = new TimeGrid (Tupel<int,1>(steps),
                              Tupel<double,1>(endTime-startTime), true);
    built = true;
  }

  template <class SpaceDiscr> template <class DiscFunc>
  inline DiscFunc& TimeEulerFV<SpaceDiscr>::operator() (DiscFunc &f)
  {
    if(!built)
    {
      std::cout << "assemble the TimeEulerFV first! \n";
      abort();
    }

    SpaceDiscr& fake = (*fv_);
    Vec<1> tmp(0.0);

    TimeGrid::LevelIterator endit = timegrid_->lend<0>(0);
    for(TimeGrid::LevelIterator it = timegrid_->lbegin<0>(0); it != endit; ++it)
    {
      std::cout << "TimeStep " << it->index() << "\n";
      double timestep = (it->geometry().integration_element(tmp));
      fv_->assemble(f,timestep,it->geometry()[0](0));
      fake(f);
      fv_->finalize(f);
      char na[20];
      sprintf(na,"file%d",it->index());
      //f.print2File<disp>(na);
    }

    return f;
  }

  template <class SpaceDiscr> template <class DiscFunc>
  inline void TimeEulerFV<SpaceDiscr>::finalize(DiscFunc &f)
  {
    if(timegrid_)
    {
      delete timegrid_;
      timegrid_ = NULL;
    }
    built = false;
  }

} // end namespace Dune
