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
  template<int dim, int dimworld>
  class OneDGridBoundaryEntity
    : public BoundaryEntityDefault <dim,dimworld, OneDCType,
          OneDGridElement,OneDGridBoundaryEntity>
  {
    friend class OneDGridIntersectionIterator<dim,dimworld>;
  public:
    OneDGridBoundaryEntity () : _neigh(-1), _geom()
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
    OneDGridElement<dim,dimworld> geometry ()
    {
      return _geom;
    }

  private:

    int _neigh;

    OneDGridElement<dim,dimworld> _geom;

  };

}  // namespace Dune

#endif
