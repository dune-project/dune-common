// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_FUNCTIONSPACE_HH
#define DUNE_FUNCTIONSPACE_HH

#include "localbase.hh"
#include "localbase/defaultmmgr.hh"
#include "localbase/scalarblas.hh"

namespace Dune
{

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
    typedef typename GRID::Traits<0>::LevelIterator LevelIterator;
    enum { elType = GRID::ReferenceElement::type };
    enum { dimdef = GRID::ReferenceElement::dimension };

    enum { type = basetype };

    // type of the local base
    typedef LocalBase<dimdef,ElementType(elType),basetype> LOCALBASE;
    enum { order = LOCALBASE::order };
    enum { numDof = LOCALBASE::numDof };
    enum { dimrange = LOCALBASE::dimrange };

    typedef ScalarVector VALTYPE;
    typedef LocalBaseFunction<typename GRID::ReferenceElement,
        basetype,dimrange,dimdef> BASEFUNC;


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

    //! a discrete function can live on diffrent levels, i.e -1 the leaf
    //! level
    int level_;

  public:

    //! Constructor, make a FunctionSpace for given grid and BaseType
    FunctionSpace(Grid *grid, int level);

    ~FunctionSpace();

    //! access to base function i
    BASEFUNC * getLocalBaseFunc(int i);

    //! map indices to dofs
    template <class Entity>
    Vec<dimrange> map(Entity & el,VALTYPE *val,int dof);

    template <class Entity>
    void  doMapping(Entity & e);

    const char* name () { return name_; };

    //! map indices to dofs global Numbers
    template <class Entity>
    int mapper(Entity& e, int dof);

    template <class Entity>
    int mapIndex(Entity& e, int dof);

    //! access to dimension of function space
    int dimOfFunctionSpace();
    int dim() { return dimOfFunctionSpace_;  };

  private:

    //! get Base with id = BaseType::id
    //! makes dofs on elements
    void makeBase();

    void makeMapVec();
    void makeMapVecLag();

#if 0
    template <class Entity>
    int mapperLagrangeOne(Entity &e,int index, int dof)
    {
      return (mapElNumber_[mapDefault(index,dof)]);
    }
#endif
    //! map indices to dofs
    int mapDefault(int index, int dof);


  };


} // end namespace Dune

#include "localbase/functionspace.cc"

#endif
