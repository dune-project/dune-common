// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_DISCRETEFUNCTION_HH
#define DUNE_DISCRETEFUNCTION_HH

#include "functionspace.hh"

namespace Dune
{

  //*********************************************************************
  //
  //  --Function
  //
  //*********************************************************************
  template <int def, int dimrange>
  class Function
  {
  public:

    Vec<dimrange> eval (Vec<def> &vec);
    void print();

  }; // end Function



  //*********************************************************************
  //
  //  --DiscreteFunction
  //
  //*********************************************************************
  //! DiscreteFunction is parametrized through FunctionSpace
  //! all needed information is stored in FunctionSpace
  template <class FuncSpace>
  class DiscreteFunction
  {
    //! type of grid the clearly the same
    typedef typename FuncSpace::GRID GRID;
    typedef typename GRID::LevelIterator LevelIterator;


    //! dimrange: dimension of the image of the DiscretFunction
    enum { dimrange = FuncSpace::dimrange };

    //! dimdef  : dimension of the source of the DiscreteFunction
    enum { dimdef = FuncSpace::dimdef };

    //! polynomial order of the DiscreteFunction
    enum { order = FuncSpace::order };

    //! number of dofs per element
    enum { numDof = FuncSpace::numDof };

    enum { dimBary = GRID::dimension +1};

    typedef Vec<dimrange> INITFUNC (Vec<dimdef> &x);

  private:
    int dimOfFunctionSpace_;
    FuncSpace *feSpace_;

    char *name_;


    GRID *grid_;

    typedef FuncSpace::BASEFUNC BASEFUNC;
    typedef typename FuncSpace::VALTYPE VALTYPE;

    VALTYPE *vec_;

  public:

    //! Constructor make a DiscreteFunction with a given
    //! FunctionSpace
    DiscreteFunction(char* name, FuncSpace *feSpace);

    ~DiscreteFunction();

    //! dummy method for setting some dofs
    void setFunction (INITFUNC *initFunc, int polOrd);

    //! go down grid until deepest level and then evaluate the function
    template <class Entity>
    Vec<dimrange> goDeeper(Entity &it,Vec<dimdef> &vec);

    //! evaluate the function on point vec
    //! which leads to an hierarchical search for the Entity,
    //! in which the point lies
    Vec<dimrange> eval (Vec<dimdef> &vec);

    //! evaluate the function on an given Entity
    template <class Entity>
    Vec<dimrange> evalElement (Entity& el, Vec<dimdef> &vec);

    const char* name () { return name_; };

    //! access to the ScalarVector, which holds the dofs
    VALTYPE& getDofVec();

  }; // end class DiscreteFunction


} // end namespace Dune

#include "localbase/discretefunction.cc"

#endif
