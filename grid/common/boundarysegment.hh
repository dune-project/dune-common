// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BOUNDARY_SEGMENT_HH
#define DUNE_BOUNDARY_SEGMENT_HH

/** \file
    \brief Base class for grid boundary segments of arbitrary geometry
 */

namespace Dune {

  /** \brief Base class for classes implementing geometries of boundary segments

     Some grid implementations, as for example UGGrid, allow to use boundary segments of
     arbitrary geometry.  That means that you can have grid boundaries approach smooth
     shapes when refining the grid.

     Such curved boundary segments are specified by giving classes that implement them.
     Each boundary segment is implemented by an object of a class derived from
     BoundarySegment.  The set of these objects is handed over to the grid upon grid
     construction.
   */
  template <int dimworld>
  class BoundarySegment {
  public:

    /** \brief Dummy virtual destructor */
    virtual ~BoundarySegment() {}

    /** \brief A function mapping local coordinates on a boundary segment to world coordinates
     */
    virtual FieldVector<double,dimworld> operator()(const FieldVector<double, dimworld-1>& local) const = 0;

  };

}  // end namespace Dune

#endif
