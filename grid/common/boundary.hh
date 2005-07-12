// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_BOUNDARY_HH
#define DUNE_GRID_BOUNDARY_HH

namespace Dune {


  //********************************************************************
  //  --BoundaryEntity
  //
  /*!
     First Version of a BoundaryEntity which holds some information about
     the boundary on an intersection with the boundary or and ghost boundary
     cell.
   */
  //********************************************************************
  template<class GridImp, template<class> class BoundaryEntityImp>
  class BoundaryEntity
  {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  protected:
    BoundaryEntityImp<GridImp> realBoundaryEntity;
  public:
    typedef typename GridImp::template Codim<0>::Geometry Geometry;

    //! return id of boundary segment, any integer but != 0
    int id () const
    {
      return realBoundaryEntity.id();
    }

    //! return true if ghost boundary cell was generated
    bool hasGeometry () const
    {
      return realBoundaryEntity.hasGeometry();
    }

    //! return geometry of ghostcell
    Geometry & geometry () const
    {
      return realBoundaryEntity.geometry();
    }

    // copy constructor from BoundaryEntityImp
    BoundaryEntity(const BoundaryEntityImp<GridImp> & e) : realBoundaryEntity(e) {}
  };

  //********************************************************************
  //  BoundaryEntityInterface
  //
  //! First Version of a BoundaryEntity which holds some information about
  //! the boundary on an intersection with the boundary
  //
  //********************************************************************
  template<class GridImp, template<class> class BoundaryEntityImp>
  class BoundaryEntityInterface
  {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  public:
    typedef typename GridImp::template Codim<0>::Geometry Geometry;

    int id () const
    {
      return asImp().id();
    }

    //! return true if ghost boundary cell was generated
    bool hasGeometry () const
    {
      return asImp().hasGeometry();
    }

    //! return geometry of ghost boundary cell
    Geometry & geometry () const
    {
      return asImp().geometry();
    }

  private:
    //!  Barton-Nackman trick
    BoundaryEntityImp<GridImp> & asImp () {return static_cast<BoundaryEntityImp<GridImp>&>(*this);}
    const BoundaryEntityImp<GridImp> & asImp () const {return static_cast<const BoundaryEntityImp<GridImp>&>(*this);}
  };

  //********************************************************************
  //
  // BoundaryEntityDefault
  //
  //! Default implementations for the BoundaryEntity
  //!
  //********************************************************************
  template<class GridImp, template<class> class BoundaryEntityImp>
  class BoundaryEntityDefault
  {
  private:
    //!  Barton-Nackman trick
    BoundaryEntityImp<GridImp> & asImp () {return static_cast<BoundaryEntityImp<GridImp>&>(*this);}
    const BoundaryEntityImp<GridImp> & asImp () const {return static_cast<const BoundaryEntityImp<GridImp>&>(*this);}
  };

}

#endif // DUNE_GRID_BOUNDARY_HH
