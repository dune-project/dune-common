// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_LOCALBASE_HH
#define DUNE_LOCALBASE_HH

#include "../common/matvec.hh"
#include "../grid/common/grid.hh"

namespace Dune
{

  enum BaseType {Const , LagrangeOne, LagrangeTwo, DGOne, DGTwo };

  static const char BaseName [5][20] = {"Const", "LagrangeOne", "LagrangeTwo",
                                        "DGOne", "DGTwo" };


  //! default Implementation of Lagrange, i.e. Lagrange 0
  //! ord specifies the degree of the Lagrange base
  //! ElementType specifies the element type for which the current
  //! Lagrange base is
  template <int ddef, ElementType elType, BaseType basetype>
  class LocalBase
  {
  public:
    enum { id = 1 };

    enum { dimdef = ddef };
    enum { order = 0 };
    enum { numDof = 1 };
    enum { dimrange = 1 };
    // order of Lagrange element, 1 = linear Lagrange element

    typedef Vec<dimrange> FUNC (Vec<ddef> &x);

  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);

  private:
    static Vec<dimrange> phi(Vec<ddef> &x);
  }; // end default LocalBase

  //! const base functions for arbitrary elements
  template <int ddef, ElementType elType>
  class LocalBase<ddef,elType,Const>
  //: public BaseFunction<ddef>
  {
  public:
    enum { dimdef = ddef };
    enum { order = 0 };
    enum { numDof = 1 };
    enum { dimrange = 1 };

    typedef Vec<dimrange> FUNC (Vec<ddef> &x);

  public:
    LocalBase()  { }

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);

  private:
    static Vec<dimrange> phi(Vec<ddef> &x);
  }; // end Const LocalBase


  //****************************************************************
  // linear DG elements for triangles (2D)
  //****************************************************************
  template <int ddef, ElementType elType>
  class LocalBase<ddef,elType,DGOne>
  //: public BaseFunction<ddef>
  {
  public:
    enum { dimdef = ddef };
    enum { order = 1 };
    enum { numDof = dimdef+1 };
    enum { dimrange = 1 };

    typedef Vec<dimrange> FUNC (Vec<ddef> &x);
  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);

  private:
    static Vec<dimrange> phi0(Vec<dimdef> &x); // 1-x-y
    static Vec<dimrange> phi1(Vec<dimdef> &x); // x
    static Vec<dimrange> phi2(Vec<dimdef> &x); // y
  }; // end default LocalBase

  //****************************************************************
  // quadratic DG elements for triangles (2D)
  //****************************************************************
  template <int ddef, ElementType elType>
  class LocalBase<ddef,elType,DGTwo>
  //: public BaseFunction<ddef>
  {
  public:
    enum { dimdef = ddef };
    enum { order = 2 };
    enum { numDof = (dimdef+1)*2 };
    enum { dimrange = 1 };

    typedef Vec<dimrange> FUNC (Vec<ddef> &x);
  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);

  private:
    static Vec<dimrange> phi0(Vec<dimdef> &x);
    static Vec<dimrange> phi1(Vec<dimdef> &x);
    static Vec<dimrange> phi2(Vec<dimdef> &x);
    static Vec<dimrange> phiEdge0(Vec<dimdef> &x);
    static Vec<dimrange> phiEdge1(Vec<dimdef> &x);
    static Vec<dimrange> phiEdge2(Vec<dimdef> &x);
  }; // end default LocalBase

  // linear Lagrange elements for triangles (2D)
  template <int ddef, ElementType elType>
  class LocalBase<ddef,elType,LagrangeOne>
  //: public BaseFunction<ddef>
  {
  public:
    enum { dimdef = ddef };
    enum { order = 1 };
    enum { numDof = dimdef+1 };
    enum { dimrange = 1 };
    // order of Lagrange element, 1 = linear Lagrange element

    typedef Vec<dimrange> FUNC (Vec<ddef> &x);
  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);

  private:
    static Vec<dimrange> phi0(Vec<dimdef> &x); // 1-x-y
    static Vec<dimrange> phi1(Vec<dimdef> &x); // x
    static Vec<dimrange> phi2(Vec<dimdef> &x); // y
  }; // end default LocalBase


  // linear Lagrange elements for triangles (2D)
  template <int ddef, ElementType elType>
  class LocalBase<ddef,elType,LagrangeTwo>
  //: public BaseFunction<ddef>
  {
  public:
    enum { dimdef = ddef };
    enum { order = 2 };
    enum { numDof = (dimdef+1)*2 };
    enum { dimrange = 1 };
    // order of Lagrange element, 1 = linear Lagrange element

    typedef Vec<dimrange> FUNC (Vec<ddef> &x);
  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);
  private:
    static Vec<dimrange> phi0(Vec<dimdef> &x);
    static Vec<dimrange> phi1(Vec<dimdef> &x);
    static Vec<dimrange> phi2(Vec<dimdef> &x);
    static Vec<dimrange> phiEdge0(Vec<dimdef> &x);
    static Vec<dimrange> phiEdge1(Vec<dimdef> &x);
    static Vec<dimrange> phiEdge2(Vec<dimdef> &x);
  }; // end default LocalBase


  //************************************************************************
  //
  // --LocalBaseFunction
  //
  //************************************************************************
  //! local base functions on the reference element
  template <class RefElement, class BaseFuncType, int dimrange, int dimdef>
  class LocalBaseFunction
  {
    typedef Vec<dimrange> FUNC (Vec<dimdef> &x);

    FUNC* phi_;

  public:
    LocalBaseFunction(FUNC *p) : phi_(p) { };

    //!  evaluation of the base function
    Vec<dimrange> eval (Vec<dimdef> & localCoords);

    //! evaluation of the first derivative of the basefunction
    Vec<dimrange> evalFirstDrv (Vec<dimdef> &localCoords);

    //! evaluation of the second derivative of the basefunction
    Vec<dimrange> evalSecondDrv (Vec<dimdef> &localCoords);

  }; // end class LocalBaseFunction


  //************************************************************************
  //
  // --FunctionSpace, FunctionSpace for a given grid and BaseType ( i.e.
  // Lagrange
  //
  //************************************************************************
  template<class Grid, BaseType basetype>
  class FunctionSpace
  {
  public:
    // nur so, damit man nicht immer alles umschreiben muss
    typedef Grid GRID;
    typedef typename GRID::LevelIterator LevelIterator;
    enum { elType = GRID::ReferenceElement::type };
    enum { dimdef = GRID::ReferenceElement::dimension };

    // type of the local base
    typedef LocalBase<dimdef,ElementType(elType),basetype> LOCALBASE;
    enum { order = LOCALBASE::order };
    enum { numDof = LOCALBASE::numDof };
    enum { dimrange = LOCALBASE::dimrange };

    typedef typename ScalarVector VALTYPE;
    typedef LocalBaseFunction<GRID::ReferenceElement,
        BaseType,dimrange,dimdef> BASEFUNC;


    //! MemoryManager, later given outside, dont know yet
    DefaultDSMM *dsmm_;
    DefaultGHMM *ghmm_;
    ScalarSparseBLASManager *ssbm_;

    //! Grid, to which the FunctionSpace belongs
    GRID * grid_;
  private:
    const char* name_;

    //! dim of function space, number of dofs
    int dimOfFunctionSpace_;
    int gridSize_;

    // in der Grid Klasse unterbringen
    int *mapElNumber_;

    //! the local base functions
    Vec<numDof,BASEFUNC*> localBase_;

    //! storage class of the local base functions
    LOCALBASE *baseType_;

  public:

    //! Constructor, make a FunctionSpace for given grid and BaseType
    FunctionSpace(Grid *grid);

    ~FunctionSpace();

    //! access to base function i
    BASEFUNC * getLocalBaseFunc(int i);

    //! map indices to dofs
    template <class Entity>
    Vec<dimrange> map(Entity & el,VALTYPE *val,int dof);

    template <class Entity>
    void  doMapping(Entity & e);

    template <BaseType bt>
    int mapper(int index, int dof);

    const char* name () { return name_; };

    //! map indices to dofs
    int mapIndex(int index, int dof);

    //! access to dimension of function space
    int dimOfFunctionSpace();

  private:
    //! get Base with id = BaseType::id
    //! makes dofs on elements
    void makeBase();

    void makeMapVec();
    void makeMapVecLag();

    template <>
    int mapper<LagrangeOne>(int index, int dof)
    {
      return (mapElNumber_[mapDefault(index,dof)]);
    }

    //! map indices to dofs
    int mapDefault(int index, int dof);

  };

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

#include "localbase/localbase.cc"

#endif
