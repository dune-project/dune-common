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
  DiscreteFunction(const char *name, FuncSpace *feSpace) :
    name_(name), feSpace_(feSpace)
  {
    grid_ = feSpace_->grid_;
    ssbm_ = feSpace_->ssbm_;
    dimOfFunctionSpace_ = feSpace_->dimOfFunctionSpace();

    vec_ = new ScalarVector(ssbm_);
    vec_->Build(dimOfFunctionSpace_,dimOfFunctionSpace_);

    // set all components of vec_ to 0.0
    ssbm_->vset(vec_,0.0);
  }

  template <class FuncSpace>
  inline DiscreteFunction<FuncSpace>::
  DiscreteFunction(const DiscreteFunction &org)
  {
    feSpace_ = org.feSpace_;
    grid_ = org.grid_;
    ssbm_ = feSpace_->ssbm_;
    dimOfFunctionSpace_ = org.dimOfFunctionSpace_;
    char *na = new char [256];
    sprintf(na,"copy of %s",org.name_);
    name_ = na;

    vec_ = new ScalarVector(feSpace_->ssbm_);
    vec_->Clone(org.vec_);

    ssbm_->vcopy(vec_,org);
    vec_->print(name_,1);
  }

  template <class FuncSpace>
  inline DiscreteFunction<FuncSpace>::
  ~DiscreteFunction()
  {
    std::cout << "Cleaning Discrete Function ... ";
    if(vec_)
    {
      vec_->Free();
      delete vec_;
    }
    std::cout << "done!\n";
  }

  template <class FuncSpace> template <class Func>
  inline void DiscreteFunction<FuncSpace>::
  setFunction (Func &initFunc, int polOrd)
  {
    double val = 0.0;

    int level = -1;
    LevelIterator endit = grid_->lend<0>(level);
    ssbm_->vset(vec_,0.0);

    for(LevelIterator it = grid_->lbegin<0>(level); it != endit; ++it)
    {
      Vec<dimdef> vec(1.0);
      double vol = it->geometry().integration_element(vec);

      vec = 0.0;

      for(int i=0; i<it->geometry().corners(); i++)
      {
        vec += it->geometry()[i];
      }

      vec *= (1.0/3.0);
      if(numDof > 1)
        val = vol*initFunc.eval(vec) (0)/(double) numDof;
      else
        val = initFunc.eval(vec) (0);

      for(int i=0; i<FuncSpace::numDof; i++)
      {
        int k = feSpace_->mapIndex((*it),i);
        vec_->Add(k,&val);
      }

    }
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
      Vec<dimdef> newTmp1 = tmp->evalFirstDrv(bary) (0);

      for(int j=0; j<dimrange; j++)
        newVal(j) *= newTmp(j);

      value = value + newVal;
    }

    return value;
  }

  template <class FuncSpace> template <class Func>
  inline double DiscreteFunction<FuncSpace>::lNorm (Func &f, int power)
  {
    // Gitterdurchlauf und auf jedem Element (\int_T f(x)^p dx)^(\frac{1}{p})
  }


  template <class FuncSpace> template <class Entity>
  inline Vec<DiscreteFunction<FuncSpace>::dimrange>
  DiscreteFunction<FuncSpace>::evalDof (Entity& el, int localDof)
  {
    Vec<dimrange> newVal = feSpace_->map(el,vec_,localDof);
    return newVal;
  }


  template <class FuncSpace>
  inline DiscreteFunction<FuncSpace>::VALTYPE&
  DiscreteFunction<FuncSpace>::getDofVec()
  {
    return (*vec_);
  }

  template <class FuncSpace>
  inline void DiscreteFunction<FuncSpace>::print( std::ostream& s) const
  {
    std::cout << name_ << std::endl;
    std::cout << dimOfFunctionSpace_ << " " << dimrange << " " << order << std::endl;
    vec_->print(const_cast<char *> (name_),1);
  }

  template <class FuncSpace> template <VizFormat format>
  inline void DiscreteFunction<FuncSpace>::print2File(char * outfile) const
  {
    double *values = new double [dimrange];
    for(int i=0; i< dimrange; i++) values[i] = 0.0;

    std::ofstream s;
    s.open(outfile, std::ios_base::out | std::ios_base::trunc);

    std::cout << "Writing (default) `" << name_ << "' to `" << outfile << "' \n";

    s << name_ << endl;
    s << dimOfFunctionSpace_ << " " << dimrange << " " << order << endl;
    for(int i=0; i<dimOfFunctionSpace_; i++)
    {
      vec_->Get(i,values);
      for(int j=0; j<dimrange; j++)
        s << values[j] << " ";
      s << endl;
    }
  }

  template <class FuncSpace>
  inline void DiscreteFunction<FuncSpace>::writeDisp(char * outfile) const
  {
    double *values = new double [dimrange];
    for(int i=0; i< dimrange; i++) values[i] = 0.0;

    std::ofstream s;

    s.open(outfile, std::ios_base::out | std::ios_base::trunc);

    std::cout << "Writing (disp) `" << name_ << "' to `" << outfile << "'. \n";

    s << dimOfFunctionSpace_ << " " << dimrange << " " << FuncSpace::order << std::endl;

    for(int i=0; i<dimOfFunctionSpace_; i++)
    {
      vec_->Get(i,values);
      for(int j=0; j<dimrange; j++)
        s << values[j] << " ";
      s << std::endl;
    }

    s.close();
  }

  template <class FuncSpace>
  inline void DiscreteFunction<FuncSpace>::writeUspm(char * outfile) const
  {
    double *values = new double [dimrange];
    for(int i=0; i< dimrange; i++) values[i] = 0.0;

    std::ofstream s;

    s.open(outfile, std::ios_base::out | std::ios_base::trunc);

    std::cout << "Writing (USPM) `" << name_ << "' to `" << outfile << "'. \n";

    for(int i=0; i<dimOfFunctionSpace_; i++)
    {
      vec_->Get(i,values);
      for(int j=0; j<dimrange; j++)
        s << values[j] << " ";
      s << endl;
    }

    s.close();
  }

} // end namespace Dune
