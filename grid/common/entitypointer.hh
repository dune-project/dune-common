// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_ENTITYPOINTER_HH
#define DUNE_GRID_ENTITYPOINTER_HH

#include <dune/common/iteratorfacades.hh>

/** \file
    \brief Wrapper and interface classe for a static iterator (EntityPointer)
 */

namespace Dune
{

  /** \brief Encapsulates the static part of an arbitrary Grid::Iterator
      \ingroup GridCommon

      The EntityPointer can be used like a static iterator. It point to a
      Dune::Entity and can be dereferenced, compared and knows the
      Entities level.

      You should be able to initialize and interpret every Dune::XxxIterator
      as Dune::EntityPointer. There for we need an inheritance hierarchy of
      the Iterator wrappers:
      \code
      class Dune::EntityPointer<...>;

      class Dune::LevelIterator<...> :
         public Dune::EntityPointer<...>;

      class Dune::HierarchicIterator<...> :
         public Dune::EntityPointer<...>;

      class Dune::LeafIterator<...> :
         public Dune::EntityPointer<...>;

      class Dune::IntersectionIterator<...> :
         public Dune::EntityPointer<...>;
      \endcode

      This hierarchy must be resambled in the implementation (i.e. SGrid):
      \code
      class SEntityPointer<...> :
         public Dune::EntityPointerDefault<..., SEntityPointer>;

      class SLevelIterator<...> :
         public SEntityPointer <...>,
         public Dune::LevelIteratorDefault <..., SLevelIterator>;

      class SHierarchicIterator<...> :
         public SEntityPointer <...>,
         public Dune::HierarchicIteratorDefault <..., SHierarchicIterator>;

      ...
      \endcode
      Please note that dereference(...), equals(...) and level() are only
      implemented in SEntityPointer, SLevelIterator inherits these methods.
      And it is not possible to specialize these, because EntityPointer always
      uses the base class.

      This leads to a hierarchy where
      Dune::LevelIterator<..., SLevelIterator> inherits
      Dune::EntityPointer<..., SLevelIterator> and
      Dune::HierarchicIterator<..., SHierarchicIterator> inherits
      Dune::EntityPointer<..., SHierarchicIterator>.
      And vitualy all Dune::EntityPointer<..., SXxxIterator> are descendents
      of Dune::EntityPointer<..., SEntityPointer>.

      Now you can compare Dune::LevelIterator with Dune::EntityPointer and
      Dune::LeafIterator with Dune::IntersectionIterator. And you can assign
      Dune::EntityPointer from any Dune::XxxIterator class. Even more you can
      cast an Iterator refence to a reference pointing to Dune::EntityPointer.

      The compiler take care that you only assign/compare Iterators from the same
      Grid.

      The downside (or advantage) of this inheritance is that you can
      not use different comparsion operators and different dereference
      oprators for the different Iterators in one Grid. On the first
      sight it is a downside because one might consider it a good idea
      to have special treatment for different iterators. On the other
      hand it's very confusing for the user if different Iterators show
      different behavior in the same situation. So now you are forced to
      show the same behavior.

      \tparam GridImp The grid class whose elements we are encapsulating
      \tparam IteratorImp The class that implements the actual Iterator/Pointer
   */
  template<class GridImp, class IteratorImp>
  class EntityPointer
  {
    friend class Dune::GenericLeafIterator<GridImp>;
  protected:
    IteratorImp realIterator;

  public:
    // autocheck wether imp is convertable into imp::base
    typedef typename
    Dune::EnableIfInterOperable<typename IteratorImp::base,IteratorImp,
        typename IteratorImp::base>::type base;

    typedef typename IteratorImp::Entity Entity;
    enum { codim = IteratorImp::codimension };

    typedef IteratorImp DerivedType;
    typedef Entity Value;
    typedef Entity* Pointer;
    typedef Entity& Reference;

    /** @brief Dereferencing operator. */
    Reference operator*() const
    {
      return realIterator.dereference();
    }

    /** @brief Pointer operator. */
    Pointer operator->() const
    {
      return & realIterator.dereference();
    }

    /** @brief ask for level of entity */
    int level () const
    {
      return realIterator.level();
    }

    /** @brief copy constructor from IteratorImp */
    EntityPointer(const IteratorImp & i) :
      realIterator(i) {};

    /** @brief indirect copy constructor from arbitrary IteratorImp */
    template<class ItImp>
    EntityPointer(const EntityPointer<GridImp,ItImp> & ep) :
      realIterator(ep.realIterator) {}

    /** @brief cast to ,,base class'' */
    operator EntityPointer<GridImp,base>&()
    {
      return reinterpret_cast<EntityPointer<GridImp,base>&>(*this);
    };

    /** @brief cast to const ,,base class'' */
    operator const EntityPointer<GridImp,base>&() const
    {
      return reinterpret_cast<const EntityPointer<GridImp,base>&>(*this);
    };

    /** @brief forward equality check to realIterator */
    bool equals(const EntityPointer& rhs) const
    {
      return this->realIterator.equals(rhs.realIterator);
    }

    /** @brief Checks for equality.
     * only works EntityPointers on the same grid */
    bool operator==(const EntityPointer<GridImp,base>& rhs) const
    {
      return rhs.equals(*this);
    }

    /** @brief Checks for inequality.
     * only works EntityPointers on the same grid */
    bool operator!=(const EntityPointer<GridImp,base>& rhs) const
    {
      return ! rhs.equals(*this);
    }

  };

  //************************************************************************
  // E N T I T Y P O I N T E R
  //************************************************************************

  /** \brief Same as LevelIterator but without ++. Can be used as a pointer to an Entity
   */
  template<int codim, class GridImp, class IteratorImp>
  class EntityPointerInterface
  {
  public:
    typedef typename GridImp::template codim<codim>::Entity Entity;

    //! know your own codimension
    enum { codimension=codim };
    //! know your own dimension
    enum { dimension=GridImp::dimension };
    //! know your own dimension of world
    enum { dimensionworld=GridImp::dimensionworld };

    //! define type used for coordinates in grid module
    typedef typename GridImp::ctype ctype;

    //! equality
    bool equals(const IteratorImp& i) const
    {
      return asImp().equals(i);
    }
    //! dereferencing
    Entity& dereference() const { return asImp().dereference(); }

    //! ask for level of entity
    int level () const { return asImp().level(); }
  private:
    //!  Barton-Nackman trick
    IteratorImp& asImp () {
      return static_cast<IteratorImp&>(*this);
    }
    const IteratorImp& asImp () const {
      return static_cast<const IteratorImp&>(*this);
    }
  };

  //**********************************************************************
  //
  //  --EntityPointerDefault
  //
  //! Default implementation of EntityPointer.
  //
  //**********************************************************************
  template<int codim, class GridImp, class IteratorImp>
  class EntityPointerDefault
    : public EntityPointerInterface <codim,GridImp,IteratorImp>
  {
  public:
    typedef IteratorImp base;
  private:
    //!  Barton-Nackman trick
    IteratorImp& asImp () {
      return static_cast<IteratorImp&>(*this);
    }
    const IteratorImp& asImp () const {
      return static_cast<const IteratorImp&>(*this);
    }
  }; // end LevelIteratorDefault

}

#endif // DUNE_GRID_LEVELITERATOR_HH
