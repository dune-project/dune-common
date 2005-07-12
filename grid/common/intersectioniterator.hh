// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_INTERSECTIONITERATOR_HH
#define DUNE_GRID_INTERSECTIONITERATOR_HH

#include <dune/common/iteratorfacades.hh>

namespace Dune
{

  /**
     Mesh entities of codimension 0 ("elements") allow to visit all
     neighbors, where a neighbor is an entity of codimension 0 which has
     a common entity of codimension 1 with the entity.  These neighbors
     are accessed via a IntersectionIterator. This allows the
     implementation of non-matching meshes. The number of neigbors may be
     different from the number of faces/edges of an element!

     @ingroup GridInterface
   */
  template<class GridImp, template<class> class IntersectionIteratorImp>
  class IntersectionIterator
  {
    IntersectionIteratorImp<const GridImp> realIterator;

    enum { dim=GridImp::dimension };
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  public:
    typedef typename GridImp::template Codim<0>::Entity Entity;
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;
    typedef typename GridImp::template Codim<0>::BoundaryEntity BoundaryEntity;
    typedef typename GridImp::template Codim<1>::Geometry Geometry;
    typedef typename GridImp::template Codim<1>::LocalGeometry LocalGeometry;
    //! know your own dimension
    enum { dimension=dim };
    //! know your own dimension of world
    enum { dimensionworld=dimworld };
    //! define type used for coordinates in grid module
    typedef ct ctype;

    /** ask for level of entity */
    int level () const
    {
      return realIterator.level();
    }

    /** @brief Preincrement operator. */
    IntersectionIterator& operator++()
    {
      this->realIterator.increment();
      return *this;
    }

    /** @brief Postincrement operator. */
    IntersectionIterator operator++(int)
    {
      const IntersectionIterator tmp(*this);
      this->realIterator.increment();
      return tmp;
    }

    //! return true if intersection is with boundary. \todo connection with boundary information, processor/outer boundary
    bool boundary () const
    {
      return this->realIterator.boundary();
    }
    const BoundaryEntity & boundaryEntity () const
    {
      return this->realIterator.boundaryEntity();
    }
    //! return true if intersection is with neighbor on this level.
    bool neighbor () const
    {
      return this->realIterator.neighbor();
    }

    //! return EntityPointer to the Entity on the inside of this intersection
    //! (that is the Entity where we started this Iterator)
    EntityPointer inside() const
    {
      return this->realIterator.inside();
    }

    //! return EntityPointer to the Entity on the outside of this intersection
    //! (that is the neighboring Entity)
    EntityPointer outside() const
    {
      return this->realIterator.outside();
    }

    /*! return an outer normal (length not necessarily 1)

       The returned Vector is copied to take advantage from the return type
       optimization.  Usually you will use the normal in several
       calculations, so that you store it before you use it.
     */
    FieldVector<ct, dimworld> outerNormal (const FieldVector<ct, dim-1>& local) const
    {
      return this->realIterator.outerNormal(local);
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of the element where iteration started.
     */
    const LocalGeometry& intersectionSelfLocal () const
    {
      return this->realIterator.intersectionSelfLocal();
    }
    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of neighbor
     */
    const LocalGeometry& intersectionNeighborLocal () const
    {
      return this->realIterator.intersectionNeighborLocal();
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in GLOBAL coordinates of the element where iteration started.
     */
    const Geometry& intersectionGlobal () const
    {
      return this->realIterator.intersectionGlobal();
    }

    //! local number of codim 1 entity in self where intersection is contained in
    int numberInSelf () const
    {
      return this->realIterator.numberInSelf ();
    }

    //! local number of codim 1 entity in neighbor where intersection is contained in
    int numberInNeighbor () const
    {
      return this->realIterator.numberInNeighbor ();
    }

    //! return unit outer normal, this should be dependent on
    //! local coordinates for higher order boundary
    //! the normal is scaled with the integration element
    FieldVector<ct, dimworld> integrationOuterNormal (const FieldVector<ct, dim-1>& local) const
    {
      return this->realIterator.integrationOuterNormal(local);
    }
    //! return unit outer normal
    FieldVector<ct, dimworld> unitOuterNormal (const FieldVector<ct, dim-1>& local) const
    {
      return this->realIterator.unitOuterNormal(local);
    }

    /** @brief Checks for equality.

        Only Iterators pointing same intersection from the same Entity
        are equal. Pointing to the same intersection from neighbor is
        unequal as inside and outside are permuted.
     */
    bool operator==(const IntersectionIterator& rhs) const
    {
      return rhs.equals(*this);
    }

    /** @brief Checks for inequality.

        Only Iterators pointing same intersection from the same Entity
        are equal. Pointing to the same intersection from neighbor is
        unequal as inside and outside are permuted.
     */
    bool operator!=(const IntersectionIterator& rhs) const
    {
      return ! rhs.equals(*this);
    }

    /** @brief forward equality check to realIterator */
    bool equals(const IntersectionIterator& rhs) const
    {
      return this->realIterator.equals(rhs.realIterator);
    }


    /** Copy Constructor from IntersectionIteratorImp */
    IntersectionIterator(const IntersectionIteratorImp<const GridImp> & i) :
      realIterator(i) {};

    /** Copy constructor */
    IntersectionIterator(const IntersectionIterator& i) :
      realIterator(i.realIterator) {}

  };

  /**********************************************************************/
  /**
     @brief Interface Definition for IntersectionIteratorImp

     @ingroup GridDevel
   */
  template<class GridImp, template<class> class IntersectionIteratorImp>
  class IntersectionIteratorInterface
  {
    enum { dim=GridImp::dimension };
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  public:
    typedef typename GridImp::template Codim<0>::Entity Entity;
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;
    typedef typename GridImp::template Codim<0>::BoundaryEntity BoundaryEntity;
    typedef typename GridImp::template Codim<1>::Geometry Geometry;
    typedef typename GridImp::template Codim<1>::LocalGeometry LocalGeometry;
    //! know your own dimension
    enum { dimension=dim };
    //! know your own dimension of world
    enum { dimensionworld=dimworld };
    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! increment
    void increment()
    {
      asImp().increment();
    }

    //! return true if intersection is with boundary. \todo connection with boundary information, processor/outer boundary
    bool boundary () const
    {
      return asImp().boundary();
    }
    const BoundaryEntity & boundaryEntity () const
    {
      return asImp().boundaryEntity();
    }

    //! return true if intersection is with neighbor on this level.
    bool neighbor () const
    {
      return asImp().neighbor();
    }

    //! return EntityPointer to the Entity on the inside of this intersection
    //! (that is the Entity where we started this Iterator)
    //   EntityPointer inside() const
    //     {
    //       return asImp().inside();
    //     }

    //! return EntityPointer to the Entity on the outside of this intersection
    //! (that is the neighboring Entity)
    //   EntityPointer outside() const
    //     {
    //       return asImp().outside();
    //     }

    /*! return an outer normal (length not necessarily 1)

       The returned Vector is copied to take advantage from the return type
       optimization.  Usually you will use the normal in several
       calculations, so that you store it before you use it.
     */
    FieldVector<ct, dimworld> outerNormal (const FieldVector<ct, dim-1>& local) const
    {
      asImp().outerNormal(local);
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of the element where iteration started.
     */
    const LocalGeometry& intersectionSelfLocal () const
    {
      return asImp().intersectionSelfLocal();
    }
    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of neighbor
     */
    const LocalGeometry& intersectionNeighborLocal () const
    {
      return asImp().intersectionNeighborLocal();
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in GLOBAL coordinates of the element where iteration started.
     */
    const Geometry& intersectionGlobal () const
    {
      return asImp().intersectionGlobal();
    }

    //! local number of codim 1 entity in self where intersection is contained in
    int numberInSelf () const
    {
      return asImp().numberInSelf ();
    }

    //! local number of codim 1 entity in neighbor where intersection is contained in
    int numberInNeighbor () const
    {
      return asImp().numberInNeighbor ();
    }

  private:
    //!  Barton-Nackman trick
    IntersectionIteratorImp<GridImp>& asImp ()
    {return static_cast<IntersectionIteratorImp<GridImp>&>(*this);}
    const IntersectionIteratorImp<GridImp>& asImp () const
    {return static_cast<const IntersectionIteratorImp<GridImp>&>(*this);}
  };

  //**********************************************************************
  /**
     @brief Default Implementations for IntersectionIteratorImp

     @ingroup GridDevel
   */
  template<class GridImp, template<class> class IntersectionIteratorImp>
  class IntersectionIteratorDefault
    : public IntersectionIteratorInterface <GridImp,IntersectionIteratorImp>
  {
    enum { dim=GridImp::dimension };
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  public:
    //! return unit outer normal, this should be dependent on
    //! local coordinates for higher order boundary
    //! the normal is scaled with the integration element
    FieldVector<ct, dimworld> integrationOuterNormal (const FieldVector<ct, dim-1>& local) const
    {
      FieldVector<ct, dimworld> n = unitOuterNormal(local);
      n *= asImp().intersectionGlobal().integrationElement(local);
      return n;
    }
    //! return unit outer normal
    FieldVector<ct, dimworld> unitOuterNormal (const FieldVector<ct, dim-1>& local) const
    {
      FieldVector<ct, dimworld> n = asImp().outerNormal(local);
      n /= n.two_norm();
      return n;
    }
  private:
    //!  Barton-Nackman trick
    IntersectionIteratorImp<GridImp>& asImp ()
    {return static_cast<IntersectionIteratorImp<GridImp>&>(*this);}
    const IntersectionIteratorImp<GridImp>& asImp () const
    {return static_cast<const IntersectionIteratorImp<GridImp>&>(*this);}
  };

}

#endif // DUNE_GRID_INTERSECTIONITERATOR_HH
