// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_DISCRETEFUNCTION_HH
#define DUNE_DISCRETEFUNCTION_HH

#include "functionspace.hh"

namespace Dune
{

  enum VizFormat { USPM , disp };

  //*********************************************************************
  //
  //  --Function
  //
  //*********************************************************************
  template <int def, int drange>
  class Function
  {
  public:
    enum { dimdef = def };
    enum { dimrange = drange};

    Vec<dimrange> eval (Vec<dimdef> &vec);
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
  public:
    template <class Grid>
    struct Traits
    {
      typedef typename Grid::Traits<0>::LevelIterator LevelIterator;
    };

    //! type of grid the clearly the same
    typedef FuncSpace mySpace;

    typedef typename FuncSpace::GRID GRID;

    //! example for nested classes
    //! Know the corresponding LevelIterator
    //typedef typename GRID::Traits<0>::LevelIterator LevelIterator;
    typedef typename Traits<GRID>::LevelIterator LevelIterator;

    //! dimrange: dimension of the image of the DiscretFunction
    enum { dimrange = FuncSpace::dimrange };

    //! dimdef  : dimension of the source of the DiscreteFunction
    enum { dimdef = FuncSpace::dimdef };

    //! polynomial order of the DiscreteFunction
    enum { order = FuncSpace::order };

    //! number of dofs per element
    enum { numDof = FuncSpace::numDof };

    enum { dimBary = GRID::dimension +1};

    typedef typename FuncSpace::BASEFUNC BASEFUNC;
    typedef typename FuncSpace::VALTYPE VALTYPE;
  private:
    //! corresponding function space
    FuncSpace *feSpace_;

    ScalarSparseBLASManager *ssbm_;

    //! dim of function space, known from function space, just for
    //! speed
    int dimOfFunctionSpace_;

    //! my name
    const char *name_;

    //! corresponding grid
    GRID *grid_;

    //! ScalarVector, holding the degrees of freedom (dof)
    VALTYPE *vec_;

  public:

    //! Constructor make a DiscreteFunction with a given
    //! FunctionSpace
    DiscreteFunction(const char* name, FuncSpace *feSpace);

    //! Constructor make a DiscreteFunction with a given
    //! FunctionSpace
    DiscreteFunction(const DiscreteFunction &org);

    ~DiscreteFunction();

    //! dummy method for setting some dofs
    template <class Func>
    void setFunction (Func &initFunc, int polOrd);

    void clone (DiscreteFunction &org);

    //! evaluate the function on point vec
    //! which leads to an hierarchical search for the Entity,
    //! in which the point lies
    Vec<dimrange> eval (Vec<dimdef> &vec);

    //! evaluate the function on an given Entity
    template <class Entity>
    Vec<dimrange> evalElement (Entity& el, Vec<dimdef> &vec);

    //! evaluate the function on a given DofNodeNumber
    template <class Entity>
    Vec<dimrange> evalDof (Entity& el, int localDof);

    //! say my name
    const char* name () { return name_; };

    //! LNorm
    template <class Func> double lNorm(Func &f,int power);


    //! access to the ScalarVector, which holds the dofs
    VALTYPE& getDofVec();
    FuncSpace*  getFuncSpace() { return feSpace_; };
    GRID*  getGrid() { return grid_; };

    void print (std::ostream& s) const;

    template <VizFormat format>
    void print2File (char * outfile) const;

    template <>
    void print2File<USPM> (char * outfile) const
    {
      writeUspm(outfile);
    };

    template <>
    void print2File<disp> (char * outfile) const
    {
      writeDisp(outfile);
    };

  private:
    void writeUspm(char *outfile) const;
    void writeDisp(char *outfile) const;

    //! go down grid until deepest level and then evaluate the function
    template <class Entity>
    Vec<dimrange> goDeeper(Entity &it,Vec<dimdef> &vec);


  }; // end class DiscreteFunction


} // end namespace Dune

#include "localbase/discretefunction.cc"

#endif
