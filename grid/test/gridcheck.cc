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
#include "../../common/exceptions.hh"
#include "../../common/stdstreams.hh"

#include <limits>

// machine epsilon is multiplied by this factor
static double factorEpsilon = 10.0;

class CheckError : public Dune::Exception {};

// --- compile-time check of element-interface

template <class Geometry, bool doCheck>
struct JacobianInverse
{
  static void check(const Geometry &e)
  {
    typedef typename Geometry::ctype ctype;
    Dune::FieldVector<ctype, Geometry::mydimension> v;
    e.jacobianInverse(v);
  }
  JacobianInverse()
  {
    c = check;
  };
  void (*c)(const Geometry&);
};

template <class Geometry>
struct JacobianInverse<Geometry, false>
{
  static void check(const Geometry &e)
  {}
  JacobianInverse()
  {
    c = check;
  };
  void (*c)(const Geometry&);
};

template <class Geometry, int codim, int dim>
struct ElementInterface
{
  static void check(const Geometry &e)
  {
    IsTrue<dim-codim == Geometry::mydimension>::yes();
    IsTrue<dim == Geometry::dimension>::yes();

    typedef typename Geometry::ctype ctype;

    e.type();
    e.corners();
    e[0];
    e.refelem();

    Dune::FieldVector<ctype, Geometry::mydimension> v;
    e.global(v);
    Dune::FieldVector<ctype, Geometry::coorddimension> g;
    e.local(g);
    e.checkInside(v);
    e.integrationElement(v);
    JacobianInverse<Geometry,
        (int)Geometry::coorddimension == (int)Geometry::mydimension>();
  }
  ElementInterface()
  {
    c = check;
  };
  void (*c)(const Geometry&);
};

// reduced test on vertices
template <class Geometry, int dim>
struct ElementInterface <Geometry, dim, dim>
{
  static void check(const Geometry &e)
  {
    IsTrue<0 == Geometry::mydimension>::yes();
    IsTrue<dim == Geometry::dimension>::yes();

    // vertices have only a subset of functionality
    e.type();
    e.corners();
    e[0];
  }
  ElementInterface()
  {
    c = check;
  };
  void (*c)(const Geometry&);
};

// --- compile-time check of entity-interface

// tests that should work on entities of all codimensions
template <class Entity>
void DoEntityInterfaceCheck (Entity &e)
{
  // exported types
  typedef typename Entity::ctype ctype;

  // methods on each entity
  e.level();
  e.index();
  e.partitionType();
  e.geometry();

  // check interface of attached element-interface
  ElementInterface<typename Entity::Geometry, Entity::codimension, Entity::dimension>();
}

// check codim-entity and pass on to codim + 1
template <class Grid, int codim, int dim>
struct EntityInterface
{
  typedef typename Grid::template codim<codim>::Entity Entity;

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
  typedef typename Grid::template codim<0>::Entity Entity;
  typedef typename Grid::template codim<cd>::Entity SubEntity;
  typedef typename Grid::template codim<cd-1>::Entity NextSubEntity;
  static void check(Entity &e)
  {
    // check types
    typedef typename Entity::IntersectionIterator IntersectionIterator;
    typedef typename Entity::HierarchicIterator HierarchicIterator;
    typedef typename Entity::LevelIterator LevelIterator;

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
  typedef typename Grid::template codim<0>::Entity Entity;
  typedef typename Grid::template codim<cd>::Entity SubEntity;
  typedef typename Grid::template codim<cd-1>::Entity NextSubEntity;
  static void check(Entity &e)
  {
    // check types
    typedef typename Entity::IntersectionIterator IntersectionIterator;
    typedef typename Entity::HierarchicIterator HierarchicIterator;
    typedef typename Entity::EntityPointer EntityPointer;

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
  typedef typename Grid::template codim<0>::Entity Entity;
  static void check(Entity &e)
  {
    // check types
    typedef typename Entity::IntersectionIterator IntersectionIterator;
    typedef typename Entity::HierarchicIterator HierarchicIterator;
    typedef typename Entity::EntityPointer EntityPointer;

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
  typedef typename Grid::template codim<0>::Entity Entity;
  static void check(Entity &e)
  {
    // check types
    typedef typename Entity::IntersectionIterator IntersectionIterator;
    typedef typename Entity::HierarchicIterator HierarchicIterator;
    typedef typename Entity::EntityPointer EntityPointer;

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
  typedef typename Grid::template codim<0>::Entity Entity;
  typedef typename Grid::template codim<dim>::Entity SubEntity;

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
    e.geometryInFather();

    // intersection iterator
    e.ibegin();
    e.iend();

    // hierarchic iterator
    e.hbegin(0);
    e.hend(0);

    // adaption
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
  typedef typename Grid::template codim<dim>::Entity Entity;

  // end recursion
  static void check (Entity &e)
  {
    // consistent?
    IsTrue<dim == Entity::codimension>::yes();
    IsTrue<dim == Entity::dimension>::yes();

    // run common test
    DoEntityInterfaceCheck(e);

    // grid hierarchy
    e.ownersFather();
    e.positionInOwnersFather();
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
    typedef typename Grid::template codim<0>::LevelIterator LevelIterator;
    typedef typename Grid::template codim<0>::EntityPointer EntityPointer;
    typedef typename Grid::LeafIterator LeafIterator;

    // check for member functions
    g.maxlevel();
    g.size(0,0);
    g.overlapSize(0,0);
    g.ghostSize(0,0);

    // adaption
    EntityPointer ept = g.template lbegin<0>(0);
    g.mark(100, ept);
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

// check Entity::geometry()[c] == Entity::entity<dim>.geometry()[0] for codim=cd
template <int cd, class Grid, class Entity, bool doCheck>
struct subIndexCheck
{
  subIndexCheck (const Entity & e)
  {
    const int imax = e.template count<cd>();
    for (int i=0; i<imax; ++i)
    {
      if( (e.template entity<cd>(i)->index() != e.template subIndex<cd>(i)      ) &&
          (e.template entity<cd>(i)->globalIndex() != e.template subIndex<cd>(i)) )
        DUNE_THROW(CheckError, "e.template entity<cd>(i)->index() == e.template subIndex<cd>(i) && "<<
                   "e.template entity<cd>(i)->globalIndex() != e.template subIndex<cd>(i) faild!");
    }
    typedef typename Grid::template codim<cd>::Entity SubEntity;
    subIndexCheck<cd-1,Grid,Entity,
        Dune::Capabilities::hasEntity<Grid,SubEntity>::v> sick(e);
  }
};
// end recursion of subIndexCheck
template <class Grid, class Entity, bool doCheck>
struct subIndexCheck<-1, Grid, Entity, doCheck>
{
  subIndexCheck (const Entity & e)
  {
    return;
  }
};
// do nothing if doCheck==false
template <int cd, class Grid, class Entity>
struct subIndexCheck<cd, Grid, Entity, false>
{
  subIndexCheck (const Entity & e)
  {
    typedef typename Grid::template codim<cd>::Entity SubEntity;
    subIndexCheck<cd-1,Grid,Entity,
        Dune::Capabilities::hasEntity<Grid,SubEntity>::v> sick(e);
  }
};
template <class Grid, class Entity>
struct subIndexCheck<-1, Grid, Entity, false>
{
  subIndexCheck (const Entity & e)
  {
    return;
  }
};

// name says all
template <class Grid>
void zeroEntityConsistency (Grid &g)
{
  typedef typename Grid::template codim<0>::LevelIterator LevelIterator;
  typedef typename Grid::template codim<0>::Geometry Geometry;
  typedef typename Grid::template codim<0>::Entity Entity;
  LevelIterator it = g.template lbegin<0>(g.maxlevel());
  const LevelIterator endit = g.template lend<0>(g.maxlevel());

  for (; it!=endit; ++it)
  {
    // Entity::entity<0>(0) == Entity
    //    assert( it->template entity<0>(0)->index() == it->index() );
    //    assert( it->template entity<0>(0)->level() == it->level() );
    // Entity::count<dim>() == Entity::geometry().corners();
    assert( it->template count<Grid::dimension>() == it->geometry().corners() );
    // Entity::geometry()[c] == Entity::entity<dim>.geometry()[0];
    const int cmax = it->template count<Grid::dimension>();
    for (int c=0; c<cmax; ++c)
    {
      Dune::FieldVector<typename Grid::ctype, Grid::dimensionworld> c1(it->geometry()[c]);
      Dune::FieldVector<typename Grid::ctype, Grid::dimensionworld> c2(it->template entity<Grid::dimension>(c)->geometry()[0]);
      if( (c2-c1).two_norm() > 10 * std::numeric_limits<typename Grid::ctype>::epsilon() )
      {
        DUNE_THROW(CheckError, "geometry[i] == entity<dim>(i) failed: || c1-c2 || = || " <<
                   c1 << " - " << c2 << " || = " << (c2-c1).two_norm() << " [ with i = " << c << " ]");
      }
    }
    // Entity::entity<cd>(i).index() == Entity::subIndex(i)
    subIndexCheck<Grid::dimension, Grid, Entity, true> sick(*it);
  }
}

/*
 * search the LevelIterator for each IntersectionIterator
 */
template <class Grid>
void assertNeighbor (Grid &g)
{
  typedef typename Grid::template codim<0>::LevelIterator LevelIterator;
  typedef typename Grid::template codim<0>::IntersectionIterator IntersectionIterator;
  LevelIterator e = g.template lbegin<0>(0);
  const LevelIterator eend = g.template lend<0>(0);
  for (; e != eend; ++e)
  {
    IntersectionIterator endit = e->iend();
    IntersectionIterator it = e->ibegin();
    assert(e->index() >= 0);
    for(; it != endit; ++it)
    {
      if (it.neighbor())
      {
        assert(it->index() >= 0);
        LevelIterator n = g.template lbegin<0>(it->level());
        LevelIterator nend = g.template lend<0>(it->level());
        while (n->index() != it->index()&& n != nend) ++n;
      }
    }
  }
}

/*
 * Iterate over the grid und do some runtime checks
 */

template <class Grid>
void iterate(Grid &g)
{
  typedef typename Grid::template codim<0>::LevelIterator LevelIterator;
  typedef typename Grid::template codim<0>::Geometry Geometry;
  LevelIterator it = g.template lbegin<0>(0);
  const LevelIterator endit = g.template lend<0>(0);

  Dune::FieldVector<typename Grid::ctype, Grid::dimension> origin(1);
  Dune::FieldVector<typename Grid::ctype, Grid::dimension> result;

  for (; it != endit; ++it)
  {
    result = it->geometry().local(it->geometry().global(origin));
    typename Grid::ctype error = (result-origin).two_norm();
    if(error >= factorEpsilon * std::numeric_limits<typename Grid::ctype>::epsilon())
    {
      DUNE_THROW(CheckError, "|| geom.local(geom.global(" << origin
                                                          << ")) - origin || != 0 ( || " << result << " - origin || ) = " << error);
    };
    it->geometry().integrationElement(origin);
    if((int)Geometry::coorddimension == (int)Geometry::mydimension)
      it->geometry().jacobianInverse(origin);

    it->geometry().type();
    it->geometry().corners();
    it->geometry()[0];
    it->geometry().refelem();
  }

  typedef typename Grid::template codim<0>::LeafIterator LeafIterator;
  LeafIterator lit = g.leafbegin(g.maxlevel());
  const LeafIterator lend = g.leafend(g.maxlevel());
  for (; lit != lend; ++lit)
  {
    result = lit->geometry().local(lit->geometry().global(origin));
    typename Grid::ctype error = (result-origin).two_norm();
    if(error >= factorEpsilon * std::numeric_limits<typename Grid::ctype>::epsilon())
    {
      DUNE_THROW(CheckError, "|| geom.local(geom.global(" << origin
                                                          << ")) - origin || != 0 ( || " << result << " - origin || ) = " << error);
    };
    lit->geometry().integrationElement(origin);
    if((int)Geometry::coorddimension == (int)Geometry::mydimension)
      lit->geometry().jacobianInverse(origin);

    lit->geometry().type();
    lit->geometry().corners();
    lit->geometry()[0];
    lit->geometry().refelem();
  }

};

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
  const Grid & cg = g;
  iterate(g);
  iterate(cg);
  zeroEntityConsistency(g);
  zeroEntityConsistency(cg);
  assertNeighbor(g);
  assertNeighbor(cg);

};
