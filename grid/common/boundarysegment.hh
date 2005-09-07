// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BOUNDARY_SEGMENT_HH
#define DUNE_BOUNDARY_SEGMENT_HH

/** \file
    \brief Base class for grid boundary segments of arbitrary geometry
 */

namespace Dune {

  template <int dimworld>
  class BoundarySegment {
  public:

    virtual ~BoundarySegment() {}

    virtual FieldVector<double,dimworld> operator()(const FieldVector<double, dimworld-1>& local) const = 0;

  };

}  // end namespace Dune

#endif
