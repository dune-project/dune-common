// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRID_BOUNDARY_ENTITY_HH
#define DUNE_UGGRID_BOUNDARY_ENTITY_HH

/** \file
 * \brief The UGGridBoundaryEntity class
 */

namespace Dune {

  /** \todo Please doc me!
   * \ingroup UGGrid
   */
  template<class GridImp>
  class UGGridBoundaryEntity
    : public BoundaryEntityDefault <GridImp,UGGridBoundaryEntity>
  {

    friend class UGGridIntersectionIterator<GridImp>;

    enum {dim=GridImp::dimension};

    enum {coorddim=GridImp::dimensionworld};

  public:
    UGGridBoundaryEntity () : _neigh(-1), _geom()
    {}

    //! return identifier of boundary segment, number
    int id ()
    {
      std::cerr << "UGGridBoundaryEntity::id() Preliminary implementation,"
      "always returns -1\n";
      return -1;
    }

    //! return true if geometry of ghost cells was filled
    bool hasGeometry () {
      std::cerr << "UGGridBoundaryEntity::hasGeometry() Preliminary implementation,"
      "always returns 'false'\n";
      return false;
    }

    //! return geometry of the ghost cell
    UGGridGeometry<dim,coorddim,GridImp> geometry ()
    {
      return _geom;
    }

  private:

    int _neigh;

    UGGridGeometry<dim,coorddim,GridImp> _geom;

  };

}  // namespace Dune

#endif
