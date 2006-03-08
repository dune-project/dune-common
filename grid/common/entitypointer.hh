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
      \ingroup GIEntityPointer

      The EntityPointer can be used like a static iterator. It points to a
      Dune::Entity and can be dereferenced, compared and it knows the
      Entity's level.

      You should be able to initialize and interpret every Dune::XxxIterator
      that iterates over entities
      as a Dune::EntityPointer. Therefore we need an inheritance hierarchy of
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

      This hierarchy must be mimicked in the implementation (i.e. SGrid):
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
      implemented in SEntityPointer -- SLevelIterator inherits these methods.
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
      Dune::EntityPointer from any Dune::XxxIterator class. Even more, you can
      cast an Iterator refence to a reference pointing to Dune::EntityPointer.

      The compiler take care that you only assign/compare Iterators from the same
      Grid.

      The downside (or advantage) of this inheritance is that you can
      not use different comparison operators and different dereference
      oprators for the different Iterators in one Grid. On the first
      sight it is a downside because one might consider it a good idea
      to have special treatment for different iterators. On the other
      hand it's very confusing for the user if different Iterators show
      different behavior in the same situation. So now they are forced to
      show the same behavior.

      \tparam GridImp The grid class whose elements we are encapsulating
      \tparam IteratorImp The class that implements the actual Iterator/Pointer
   */
  template<class GridImp, class IteratorImp>
  class EntityPointer
  {
    // we must be able to initialize a GenericLeafIterator.realIterator from
    // EntityPointer.realIterator
    friend class Dune::GenericLeafIterator<GridImp>;
  protected:
    IteratorImp realIterator;

  public:
    //! type of real implementation
    typedef IteratorImp ImplementationType;

    /// autocheck wether imp is convertable into imp::base
    typedef typename
    Dune::EnableIfInterOperable<typename IteratorImp::base,IteratorImp,
        typename IteratorImp::base>::type base;

    /** \brief The Entity that this EntityPointer can point to */
    typedef typename IteratorImp::Entity Entity;

    /** \brief The codimension of this EntityPointer */
    enum { codim = IteratorImp::codimension };

    /** \todo Please doc me! */
    typedef IteratorImp DerivedType;

    /** \brief Dereferencing operator. */
    Entity & operator*() const
    {
      return realIterator.dereference();
    }

    /** \brief Pointer operator. */
    Entity * operator->() const
    {
      return & realIterator.dereference();
    }

    /** \brief Ask for level of entity */
    int level () const
    {
      return realIterator.level();
    }

    /** \brief Copy Constructor from an Iterator implementation.

       You can supply LeafIterator LevelIterator HierarchicIterator
       EntityPointer or IntersectionIterator.
     */
    EntityPointer(const IteratorImp & i) :
      realIterator(i) {};

    /** \brief Indirect copy constructor from arbitrary IteratorImp */
    template<class ItImp>
    EntityPointer(const EntityPointer<GridImp,ItImp> & ep) :
      realIterator(ep.realIterator) {}

    /** \brief Cast to ,,base class'' */
    operator EntityPointer<GridImp,base>&()
    {
      return reinterpret_cast<EntityPointer<GridImp,base>&>(*this);
    };

    /** \brief Cast to const ,,base class'' */
    operator const EntityPointer<GridImp,base>&() const
    {
      return reinterpret_cast<const EntityPointer<GridImp,base>&>(*this);
    };

    /** \briefChecks for equality.

       Only works EntityPointers on the same grid */
    bool operator==(const EntityPointer<GridImp,base>& rhs) const
    {
      return rhs.equals(*this);
    }

    /** \brief Checks for inequality.

       Only works EntityPointers on the same grid */
    bool operator!=(const EntityPointer<GridImp,base>& rhs) const
    {
      return ! rhs.equals(*this);
    }

    /** @brief forward equality check to realIterator */
    bool equals(const EntityPointer& rhs) const
    {
      return this->realIterator.equals(rhs.realIterator);
    }
  };

  //**********************************************************************
  //
  //  --EntityPointerDefault
  //
  //! Default implementation of EntityPointer.
  //**********************************************************************
  template<int codim, class GridImp, class IteratorImp>
  class EntityPointerDefaultImplementation
  {
  public:
    typedef IteratorImp base;

    //! codimension of entity pointer
    enum { codimension = codim };

  private:
    //!  Barton-Nackman trick
    IteratorImp& asImp () {
      return static_cast<IteratorImp&>(*this);
    }
    const IteratorImp& asImp () const {
      return static_cast<const IteratorImp&>(*this);
    }
  }; // end EntityPointerDefaultImplementation

}

#endif // DUNE_GRID_LEVELITERATOR_HH
