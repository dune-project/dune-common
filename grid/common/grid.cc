// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_CC
#define DUNE_GRID_CC

#include <algorithm>
#include <iostream>
#include <fstream>

namespace Dune {

  //************************************************************************
  // R E F E R E N C E T O P O L O G Y
  //************************************************************************

  // the general version, throws only exception ...
  template<int dim, class ct>
  inline ReferenceTopology<dim,ct>::ReferenceTopology ()
  {
    DUNE_THROW(GridError, "dimension too large");
  }

  template<int dim, class ct>
  inline FieldVector<ct, dim>& ReferenceTopology<dim,ct>::center_codim0_local (int elemtype)
  {
    DUNE_THROW(GridError, "dimension too large");
  }

  template<int dim, class ct>
  inline FieldVector<ct, dim-1>& ReferenceTopology<dim,ct>::center_codim1_local (int elemtype, int i)
  {
    DUNE_THROW(GridError, "dimension too large");
  }

  // Specialization for dim=1, GeometryType=1
  template<class ct>
  inline ReferenceTopology<1,ct>::ReferenceTopology ()
  {
    center0_local[0] = FieldVector<ct, 1>(0.5);
  }

  template<class ct>
  inline FieldVector<ct, 1>& ReferenceTopology<1,ct>::center_codim0_local (int elemtype)
  {
    return center0_local[0];
  }

  template<class ct>
  inline FieldVector<ct, 0>& ReferenceTopology<1,ct>::center_codim1_local (int elemtype, int i)
  {
    return center1_local[0];
  }

  // Specialization for dim=2, GeometryType=2,3
  template<class ct>
  inline ReferenceTopology<2,ct>::ReferenceTopology ()
  {
    center0_local[0] = FieldVector<ct, 2>(1.0/3.0);
    center0_local[1] = FieldVector<ct, 2>(0.5);
    center1_local[0] = FieldVector<ct, 1>(0.5);
  }

  template<class ct>
  inline FieldVector<ct, 2>& ReferenceTopology<2,ct>::center_codim0_local (int elemtype)
  {
    return center0_local[elemtype-2];
  }

  template<class ct>
  inline FieldVector<ct, 1>& ReferenceTopology<2,ct>::center_codim1_local (int elemtype, int i)
  {
    return center1_local[0];
  }

  // Specialization for dim=3, GeometryType=4..7
  template<class ct>
  inline ReferenceTopology<3,ct>::ReferenceTopology ()
  {
    center0_local[0] = FieldVector<ct, 3>(0.25);
    center0_local[1] = FieldVector<ct, 3>(0.0); // pyramid is missing !
    center0_local[2] = FieldVector<ct, 3>(0.0); // prism is missing
    center0_local[3] = FieldVector<ct, 3>(0.5);
    for (int i=0; i<6; i++) center1_local[0][i] = FieldVector<ct, 2>(1.0/3.0);
    for (int i=0; i<6; i++) center1_local[1][i] = FieldVector<ct, 2>(0.0);
    for (int i=0; i<6; i++) center1_local[2][i] = FieldVector<ct, 2>(0.0);
    for (int i=0; i<6; i++) center1_local[3][i] = FieldVector<ct, 2>(0.5);
  }

  template<class ct>
  inline FieldVector<ct, 3>& ReferenceTopology<3,ct>::center_codim0_local (int elemtype)
  {
    return center0_local[elemtype-4];
  }

  template<class ct>
  inline FieldVector<ct, 2>& ReferenceTopology<3,ct>::center_codim1_local (int elemtype, int i)
  {
    return center1_local[elemtype-4][i];
  }

} // end namespace Dune

#endif
