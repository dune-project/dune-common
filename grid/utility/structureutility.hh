// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_STRUCTUREUTILITY_HH
#define DUNE_STRUCTUREUTILITY_HH

//#include <dune/grid/yaspgrid.hh>
#include <dune/grid/sgrid.hh>

namespace Dune {

  template <class GridImp>
  struct IsUnstructured {
    static const bool value;
  };

  template <class GridImp>
  const bool IsUnstructured<GridImp>::value = true;

  template <int dim, int dimw>
  struct IsUnstructured<SGrid<dim, dimw> > {
    static const bool value;
  };

  template <int dim, int dimw>
  const bool IsUnstructured<SGrid<dim, dimw> >::value = false;

  /*
     template <int dim, int dimw>
     struct IsUnstructured<YaspGrid<dim, dimw> > {
     static const bool value = false;
     };
   */

  template <class GridImp>
  struct OneGeometryType {
    static const bool value;
  };

  template <class GridImp>
  const bool OneGeometryType<GridImp>::value = true;

  /*
     template <int dim, int dimw>
     struct OneGeometryType<UGGrid<dim, dimw> > {
     static const bool value = false;
     };
   */
} // end namespace Dune

#endif
