// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

/*

   Implements a generic grid check

   TODO:
   - check return types

 */

#include "../../common/capabilities.hh"
#include "../../common/helpertemplates.hh"

// --- compile-time check of element-interface

template <class Element, int codim, int dimw>
struct ElementInterface
{
  static void check(Element &e)
  {
    IsTrue<dimw == Element::dimension>::yes();
    IsTrue<dimw == Element::dimensionworld>::yes();

    typedef typename Element::ctype ctype;

    e.type();
    e.corners();
    e[0];
    e.refelem();

    Dune::FieldVector<ctype, Element::dimension> v;
    e.global(v);
    e.local(v);
    e.checkInside(v);
    e.integration_element(v);
    e.Jacobian_inverse(v);
  }
  ElementInterface()
  {
    c = check;
  };
  void (*c)(Element&);
};

// reduced test on vertices
template <class Element, int dimw>
struct ElementInterface <Element, dimw, dimw>
{
  static void check(Element &e)
  {
    IsTrue<dimw == Element::dimension>::yes();
    IsTrue<dimw == Element::dimensionworld>::yes();

    // vertices have only a subset of functionality
    e.type();
    e.corners();
    e[0];
  }
  ElementInterface()
  {
    c = check;
  };
  void (*c)(Element&);
};

// --- compile-time check of entity-interface

// tests that should work on entities of all codimensions
template <class Entity>
void DoEntityInterfaceCheck (Entity &e)
{
  // exported types
  typedef typename Entity::ctype ctype;
  typedef typename Entity::Traits::IntersectionIterator IntersectionIterator;
  typedef typename Entity::Traits::HierarchicIterator HierarchicIterator;
  typedef typename Entity::Traits::LevelIterator LevelIterator;

  // methods on each entity
  e.level();
  e.index();
  e.partition_type();
  // !!! Albert-only method!
  // e.boundaryId();
  e.geometry();

  // check interface of attached element-interface
  ElementInterface<typename Entity::Traits::Element, Entity::codimension, Entity::dimension>();
}

// check codim-entity and pass on to codim + 1
template <class Grid, int codim, int dim>
struct EntityInterface
{
  typedef typename Grid::template Traits<codim>::Entity Entity;

  static void check (Entity &e)
  {
    // consistent?
    IsTrue<codim == Entity::codimension>::yes();
    IsTrue<dim == Entity::dimension>::yes();

    // do the checking
    DoEntityInterfaceCheck(e);

    // recursively check sub-entities
    EntityInterface<Grid, codim + 1, dim>();
  }
  EntityInterface ()
  {
    c = check;
  }
  void (*c)(Entity&);
};

// recursive check of codim-0-entity methods count(), entity(), subIndex()
template <class Grid, int cd, bool hasEntity>
struct ZeroEntityMethodCheck
{
  typedef typename Grid::template Traits<0>::Entity Entity;
  typedef typename Grid::template Traits<cd>::Entity SubEntity;
  typedef typename Grid::template Traits<cd-1>::Entity NextSubEntity;
  static void check(Entity &e)
  {
    e.template count<cd>();
    e.template entity<cd>(0);
    e.template subIndex<cd>(0);

    // recursively check on
    ZeroEntityMethodCheck<Grid, cd - 1,
        Dune::Capabilities::hasEntity<Grid, NextSubEntity>::v >();
  }
  ZeroEntityMethodCheck ()
  {
    c = check;
  }
  void (*c)(Entity &e);
};

// just the recursion if the grid does not know about this codim-entity
template<class Grid, int cd>
struct ZeroEntityMethodCheck<Grid, cd, false>
{
  typedef typename Grid::template Traits<0>::Entity Entity;
  typedef typename Grid::template Traits<cd>::Entity SubEntity;
  typedef typename Grid::template Traits<cd-1>::Entity NextSubEntity;
  static void check(Entity &e)
  {
    // recursively check on
    ZeroEntityMethodCheck<Grid, cd - 1,
        Dune::Capabilities::hasEntity<Grid, NextSubEntity>::v >();
  }
  ZeroEntityMethodCheck ()
  {
    c = check;
  }
  void (*c)(Entity &e);
};

// end recursive checking
template <class Grid>
struct ZeroEntityMethodCheck<Grid, 0, true>
{
  typedef typename Grid::template Traits<0>::Entity Entity;
  static void check(Entity &e)
  {
    e.template count<0>();
    e.template entity<0>(0);
    e.template subIndex<0>(0);
  }
  ZeroEntityMethodCheck ()
  {
    c = check;
  }
  void (*c)(Entity &e);
};

// end recursive checking - same as true
// ... codim 0 is always needed
template <class Grid>
struct ZeroEntityMethodCheck<Grid, 0, false>
{
  typedef typename Grid::template Traits<0>::Entity Entity;
  static void check(Entity &e)
  {
    e.template count<0>();
    e.template entity<0>(0);
    e.template subIndex<0>(0);
  }
  ZeroEntityMethodCheck ()
  {
    c = check;
  }
  void (*c)(Entity &e);
};

// codim-0 entities have different interface
template <class Grid, int dim>
struct EntityInterface<Grid, 0, dim>
{
  typedef typename Grid::template Traits<0>::Entity Entity;
  typedef typename Grid::template Traits<dim>::Entity SubEntity;

  static void check (Entity &e)
  {
    // consistent?
    IsTrue<0 == Entity::codimension>::yes();
    IsTrue<dim == Entity::dimension>::yes();

    // do the common checking
    DoEntityInterfaceCheck(e);

    // special codim-0-entity methods which are parametrized by a codimension
    ZeroEntityMethodCheck
    <Grid, dim, Dune::Capabilities::hasEntity<Grid, SubEntity>::v >();

    // grid hierarchy
    e.father();
    e.father_relative_local();

    // intersection iterator
    e.ibegin();
    e.iend();

    // hierarchic iterator
    e.hbegin(0);
    e.hend(0);

    // adaption
    e.mark(0);
    e.state();

    // recursively check sub-entities
    EntityInterface<Grid, 1, dim>();
  }
  EntityInterface ()
  {
    c = check;
  }
  void (*c)(Entity&);
};

// end the recursion over entity-codimensions
template <class Grid, int dim>
struct EntityInterface<Grid, dim, dim>
{
  typedef typename Grid::template Traits<dim>::Entity Entity;

  // end recursion
  static void check (Entity &e)
  {
    // consistent?
    IsTrue<dim == Entity::codimension>::yes();
    IsTrue<dim == Entity::dimension>::yes();

    // run common test
    DoEntityInterfaceCheck(e);
  }

  EntityInterface()
  {
    c = check;
  }
  void (*c)(Entity&);
};

template<class Grid, bool hasLeaf>
struct LeafInterface
{
  static void check(Grid &g)
  {
    g.leafbegin(0);
    g.leafend(0);
  }
  LeafInterface()
  {
    c = check;
  }
  void (*c)(Grid&);
};
template<class Grid>
struct LeafInterface<Grid, false>
{
  static void check(Grid &g) {}
  LeafInterface()
  {
    c = check;
  }
  void (*c)(Grid&);
};

template <class Grid>
struct GridInterface
{
  static void check (Grid &g)
  {
    // check for exported types
    typedef typename Grid::ctype ctype;
    typedef typename Grid::template Traits<0>::LevelIterator LevelIterator;
    typedef typename Grid::LeafIterator LeafIterator;

    // check for member functions
    g.maxlevel();
    g.size(0,0);
    g.overlap_size(0,0);
    g.ghost_size(0,0);

    // adaption
    g.adapt();
    g.preAdapt();
    g.postAdapt();

    // check for iterator functions
    g.template lbegin<0>(0);
    g.template lend<0>(0);

    LeafInterface< Grid, Dune::Capabilities::hasLeafIterator<Grid>::v >();

    // recursively check entity-interface
    EntityInterface<Grid, 0, Grid::dimension>();

    // !!! check for parallel grid?
    /*
       g.template lbegin<0, Dune::Ghost_Partition>(0);
       g.template lend<0, Dune::Ghost_Partition>(0);
     */
  }
  GridInterface()
  {
    c = check;
  }
  // member just to avoid "unused variable"-warning in constructor
  void (*c)(Grid&);
};

template <class Grid>
void iterate()
{};

template <class Grid>
void gridcheck (Grid &g)
{
  /*
   * first do the compile-test: this will not produce any code but
   * fails if an interface-component is missing
   */
  GridInterface<Grid>();

  /*
   * now the runtime-tests
   */
  //  g.checkIF();
};
