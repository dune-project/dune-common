// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONE_D_GRID_BOUNDENT_HH
#define DUNE_ONE_D_GRID_BOUNDENT_HH

/** \file
 * \brief The OneDGridBoundaryEntity class
 */

namespace Dune {

  /** \todo Please doc me!
   * \ingroup OneDGrid
   */
  template <class GridImp>
  class OneDGridBoundaryEntity
    : public BoundaryEntityDefault <GridImp, OneDGridBoundaryEntity>
  {
    //friend class OneDGridIntersectionIterator<dim,dimworld>;
  public:
    typedef typename GridImp::template codim<0>::Geometry Geometry;

    OneDGridBoundaryEntity () : _neigh(-1), geom_()
    {}

    //! return identifier of boundary segment, number
    int id ()
    {
      std::cerr << "OneDGridBoundaryEntity::id() Preliminary implementation,"
      "always returns -1\n";
      return -1;
    }

    //! return true if geometry of ghost cells was filled
    bool hasGeometry () {
      std::cerr << "OneDGridBoundaryEntity::hasGeometry() Preliminary implementation,"
      "always returns 'false'\n";
      return false;
    }

    //! return geometry of the ghost cell
    Geometry& geometry ()
    {
      return geom_;
    }

  private:

    int _neigh;

    Geometry geom_;

  };

}  // namespace Dune

#endif
