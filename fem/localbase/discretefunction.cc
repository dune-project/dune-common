// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
namespace Dune
{
  //****************************************************************
  //
  // --DiscreteFunction, for a given FunctionSpace
  //
  //****************************************************************
  template <class FuncSpace>
  inline DiscreteFunction<FuncSpace>::
  DiscreteFunction(char *name, FuncSpace *feSpace) : name_(name), feSpace_(feSpace)
  {
    grid_ = feSpace_->grid_;
    dimOfFunctionSpace_ = feSpace_->dimOfFunctionSpace();

    vec_ = new ScalarVector(feSpace_->ssbm_);
    vec_->Build(dimOfFunctionSpace_,dimOfFunctionSpace_);

    double val = 0.0;
    for(int i=0; i<dimOfFunctionSpace_; i++)
    {
      val = static_cast<double> (i);
      vec_->Put(i,&val);
    }

  }

  template <class FuncSpace>
  inline DiscreteFunction<FuncSpace>::
  ~DiscreteFunction()
  {
    std::cout << "Cleaning Discrete Function \n";
    vec_->Free();
    delete vec_;
  }

  template <class FuncSpace>
  inline void DiscreteFunction<FuncSpace>::
  setFunction (INITFUNC *initFunc, int polOrd)
  {
    double val = 0.0;
#if 0
    for(int i=0; i<dimOfFunctionSpace_; i++)
    {
      vec_->Put(i,&val);
    }
#endif
    int level = -1;
    LevelIterator endit = grid_->lend<0>(level);

    for(LevelIterator it = grid_->lbegin<0>(level); it != endit; ++it)
    {
      Vec<dimdef> vec(1.0);

      for(int i=0; i<numDof; i++)
      {
        int k = feSpace_->mapIndex(it->index(),i);

        val = (initFunc(it->geometry()[i]))(0);
        vec_->Put(k,&val);
      }
    }

    vec_->print("vec",1);
  }


  template <class FuncSpace> template <class Entity>
  inline Vec<DiscreteFunction<FuncSpace>::dimrange>
  DiscreteFunction<FuncSpace>::goDeeper(Entity &it,Vec<dimdef> &vec)
  {
    Vec<dimrange> fake(0.0);
    typedef typename Entity::HierarchicIterator HIERit;
    HIERit hierend = it.hend(-1);
    for(HIERit hierit = it.hbegin(-1); hierit != hierend; ++hierit)
    {
      // Hier kann noch Option mit Levelabfrage eingebaut werden
      if((hierit->geometry().pointIsInside(vec)) && (hierit->level() ==
                                                     grid_->maxlevel()) )
        return evalElement((*hierit),vec);
    }

    return fake;
  }

  template <class FuncSpace>
  inline Vec<DiscreteFunction<FuncSpace>::dimrange>
  DiscreteFunction<FuncSpace>::eval (Vec<dimdef> &vec)
  {
    // Wertet die GridFunction auf dem Leaf Level aus
    LevelIterator endit = grid_->lend<0>(0);
    Vec<dimrange> fake(0.0);

    for(LevelIterator it = grid_->lbegin<0>(0); it != endit; ++it)
    {
      if(it->geometry().pointIsInside(vec))
      {
        if(it->level() != grid_->maxlevel())
          return goDeeper(*it,vec);
        else
          return evalElement((*it),vec);
        break;
      }

    }

    std::cout << "No Element with Point ";
    vec.print(std::cout,dimdef); std::cout << std::endl;
    // Wenn kein Element gefunden wurde, dann wird eine Vector mit Nullen
    // zurueckgegeben
    Vec<dimrange> tmp(0.0);
    return tmp;
  }

  template <class FuncSpace> template <class Entity>
  inline Vec<DiscreteFunction<FuncSpace>::dimrange>
  DiscreteFunction<FuncSpace>::evalElement (Entity& el, Vec<dimdef> &vec)
  {
    std::cout << "\nEvaluiere Element " << el.index() << std::endl;
    Vec<dimrange> value (0.0);

    Vec<dimBary> Tbary = el.geometry().localB<dimBary>(vec);
    Vec<dimdef> bary;
    for(int i=0; i<dimdef; i++)
      bary(i) = Tbary(i+1);

    for(int i=0; i<numDof; i++)
    {
      BASEFUNC *tmp = feSpace_->getLocalBaseFunc(i);
      Vec<dimrange> newVal = feSpace_->map(el,vec_,i);
      Vec<dimrange> newTmp = tmp->eval(bary);
      Mat<numDof,dimrange> newTmp1 = tmp->evalFirstDrv(bary);

      for(int j=0; j<dimrange; j++)
        newVal(j) *= newTmp(j);

      value = value + newVal;
    }

    return value;
  }

  template <class FuncSpace>
  inline DiscreteFunction<FuncSpace>::VALTYPE&
  DiscreteFunction<FuncSpace>::getDofVec()
  {
    return (*vec_);
  }

} // end namespace Dune
