// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_INTERSECTIONITERATOR_HH
#define DUNE_GRID_INTERSECTIONITERATOR_HH

#include <dune/common/iteratorfacades.hh>

namespace Dune
{

  /**
     \brief Mesh entities of codimension 0 ("elements") allow to visit all
     intersections with neighboring elements and with the domain
     boundary.

     @warning The number of neigbors may be different from the number of
     faces/edges of an element!

     <h2>Overview</h2>

     These intersections are codimension 1 objects. These
     intersections are accessed via a IntersectionIterator. This allows
     the implementation of non-matching meshes, as one face can now
     consist of several intersections.

     Currently it is not possible to access this codimension 1 entity
     itself, but the IntersectionIterator provides the basic entity
     informations.


     Depending on the state of boundary() and neighbor() one can destinguish different cases:

     <table>
     <tr>
     <td></td><td></td><td>neighbor()</td><td>boundary()</td><td>outside()</td>
     </tr>

     <tr>
     <td>1</td><td>periodic boundary</td>
     <td>true</td><td>true</td><td>Ghost-/Overlap cell@br (with transformed geometry)</td>
     </tr>

     <tr>
     <td>2</td><td>inner cell</td>
     <td>true</td><td>true <em>for inner boundaries,</em><br>false <em>otherwise</em></td>
     <td>the real neighbor entity</td>
     </tr>

     <tr>
     <td>3</td><td>domain boundary</td>
     <td>false</td><td>true</td><td><em>undefined</em></td></tr>

     <tr>
     <td>4</td><td>processor boundary</td>
     <td>false</td><td>true <em>for inner boundaries,</em><br>false <em>otherwise</em></td>
     <td><em>undefined</em></td></tr>
     </table>

     <h2>Handling periodic boundaries</h2>
     - The IntersectionIterator stops at periodic boundaries
     - periodic grids are handled in correspondence to parallel grids
     - At the periodic boundary one can adjust an overlap- or ghost-layer.
     - outer() returns a ghost or overlap cell (for gost and overlap look into the documentation of the parallel grid interface)
     - outer() cell has a periodicelly transformed geometry (so that one does not see a jump or something like that)
     - outer() cell has its own index
     - outer() cell has the same id as the corresponding "original" cell

     <h2>Handling physical boundaries</h2>
     - We require differently constructed geometries outside the domain
     - The kind of construction depends on the problem ine is discretizing
     - Therefor these constructions can't be part of the Grid interface
     - Utility classes are required to do this construction
     - The utility classes must be parametrized with the intersection (in our case the IntersectionIterator)
     - The utility classen return a suitable transformation of of the inner() entitys geometry (with respect to the intersection)
     - reflection at the intersection
     - point reflection
     - reflection combined with translation
     - ...

     <h2>Identifying boundaries</h2> As long we don't have a domain
     interface in Dune one can only identify a boundary by it's
     boundaryId().

     @ingroup GIIterators
   */
  template<class GridImp, template<class> class IntersectionIteratorImp>
  class IntersectionIterator
  {
    IntersectionIteratorImp<const GridImp> realIterator;

    enum { dim=GridImp::dimension };
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  public:

    //! type of real implementation
    typedef IntersectionIteratorImp<const GridImp> ImplementationType;

    /** \brief Type of entity that this IntersectionIterator belongs to */
    typedef typename GridImp::template Codim<0>::Entity Entity;

    /** \brief Pointer to the type of entities that this IntersectionIterator belongs to */
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;

    /** \brief Codim 1 geometry returned by intersectionGlobal() */
    typedef typename GridImp::template Codim<1>::Geometry Geometry;

    /** \brief Codim 1 geometry returned by intersectionSelfLocal()
        and intersectionNeighborLocal() */
    typedef typename GridImp::template Codim<1>::LocalGeometry LocalGeometry;

    //! @brief Export grid dimension
    enum { dimension=dim /*!< grid dimension */ };

    //! @brief Export dimension of world
    enum { dimensionworld=dimworld /*!< dimension of world */ };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    /** @brief level of inside() entity */
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
    IntersectionIterator operator++(int) DUNE_DEPRECATED
    {
      const IntersectionIterator tmp(*this);
      this->realIterator.increment();
      return tmp;
    }

    //! return true if intersection is with boundary
    bool boundary () const
    {
      return this->realIterator.boundary();
    }

    /**
       \brief Identifier for boundary segment from macro grid.

       One can attach a boundary Id to a boundary segment on the macro
       grid. This Id will also be used for all fragment of these
       boundary segments.

       The numbering is defined as:
       - Id==0 for all intersections without boundary()==false
       - Id>=0 for all intersections without boundary()==true

       The way the Identifiers are attached to the grid may differ
       between the different grid implementations.
     */
    int boundaryId () const
    {
      return this->realIterator.boundaryId();
    }

    //! @brief return true if intersection is with neighbor on this level.
    bool neighbor () const
    {
      return this->realIterator.neighbor();
    }

    /*! @brief return EntityPointer to the Entity on the inside of this
       intersection.

       (that is the Entity where we started this Iterator)
     */
    EntityPointer inside() const
    {
      return this->realIterator.inside();
    }

    /*! @brief return EntityPointer to the Entity on the outside of this
       intersection.

       (that is the neighboring Entity)

       @warning Don't call this method if there is no neighboring Entity
       (neighbor() returns false). In this case the result is undefined.
     */
    EntityPointer outside() const
    {
      return this->realIterator.outside();
    }

    /*! @brief geometrical information about this intersection in local
       coordinates of the inside() entity.

       This method return a Geometry object that provides a mapping from
       local coordinates of the intersection to local coordinates of the
       inside() entity.
     */
    const LocalGeometry& intersectionSelfLocal () const
    {
      return this->realIterator.intersectionSelfLocal();
    }
    /*! @brief geometrical information about this intersection in local
       coordinates of the outside() entity.

       This method return a Geometry object that provides a mapping from
       local coordinates of the intersection to local coordinates of the
       outside() entity.
     */
    const LocalGeometry& intersectionNeighborLocal () const
    {
      return this->realIterator.intersectionNeighborLocal();
    }

    /*! @brief geometrical information about this intersection in global coordinates.

       This method return a Geometry object that provides a mapping from
       local coordinates of the intersection to global coordinates.
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

    /*! @brief return an outer normal (length not necessarily 1)

       The returned Vector is copied to take advantage from the return
       type optimization.  Usually one will use the normal in several
       calculations, so that one stores it before using it. The
       returned vector should be dependent on local coordinates for
       higher order boundary.
     */
    FieldVector<ct, dimworld> outerNormal (const FieldVector<ct, dim-1>& local) const
    {
      return this->realIterator.outerNormal(local);
    }

    /*! @brief return outer normal scaled with the integration element

       @copydoc outerNormal

       The normal is scaled with the integration element of the intersection.
     */
    FieldVector<ct, dimworld> integrationOuterNormal (const FieldVector<ct, dim-1>& local) const
    {
      return this->realIterator.integrationOuterNormal(local);
    }
    /*! @brief return unit outer normal (length == 1)

       @copydoc outerNormal

       The normal is scaled a length of 1.
     */
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

    typedef typename RemoveConst<GridImp>::Type mutableGridImp;
  protected:
    // give the GridDefaultImplementation class access to the realImp
    friend class GridDefaultImplementation<
        GridImp::dimension, GridImp::dimensionworld,
        typename GridImp::ctype,
        typename GridImp::GridFamily> ;

    //! return reference to the real implementation
    ImplementationType & getRealImp() { return realIterator; }
    //! return reference to the real implementation
    const ImplementationType & getRealImp() const { return realIterator; }
  };

  //**********************************************************************
  /**
     @brief Default Implementations for IntersectionIteratorImp

     @ingroup GridDevel
   */
  template<class GridImp, template<class> class IntersectionIteratorImp>
  class IntersectionIteratorDefaultImplementation
  {
    enum { dim=GridImp::dimension };
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  public:
    //! return unit outer normal, this should be dependent on
    //! local coordinates for higher order boundary
    //! the normal is scaled with the integration element of the intersection.
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
