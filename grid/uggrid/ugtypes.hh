// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGTYPES_HH
#define DUNE_UGTYPES_HH

/** \file
 * \brief Encapsulates a few types from UG
 */

namespace UG2d {
  struct multigrid;
  struct grid;

  union element;
  struct node;
  struct edge;
  struct vector;
};

namespace UG3d {
  struct multigrid;
  struct grid;

  union element;
  struct node;
  struct edge;
  struct vector;
};


namespace Dune {

  template <int dim>
  class UGTypes {};

  template <>
  class UGTypes<2>
  {
  public:
    typedef UG2d::multigrid MultiGridType;

    typedef UG2d::grid GridType;

    typedef UG2d::node Node;

    typedef UG2d::element Element;
  };

  template <>
  class UGTypes<3>
  {
  public:
    typedef UG3d::multigrid MultiGridType;

    typedef UG3d::grid GridType;

    typedef UG3d::node Node;

    typedef UG3d::element Element;
  };



  /*****************************************************************/
  /*****************************************************************/
  /*****************************************************************/
  /*****************************************************************/

  template <int dim>
  class UGVectorType {};

  template <>
  class UGVectorType<3>
  {
  public:
    typedef UG3d::vector T;
  };

  template <>
  class UGVectorType<2>
  {
  public:
    typedef UG2d::vector T;
  };

  template <int codim, int dim>
  class TargetType {};

  template <>
  class TargetType<0,3>
  {
  public:
    typedef UG3d::element T;
  };

  template <>
  class TargetType<2,3>
  {
  public:
    typedef UG3d::edge T;
  };

  template <>
  class TargetType<3,3>
  {
  public:
    typedef UG3d::node T;
  };

  template <>
  class TargetType<0,2>
  {
  public:
    typedef UG2d::element T;
  };

  template <>
  class TargetType<1,2>
  {
  public:
    typedef UG2d::edge T;
  };

  template <>
  class TargetType<2,2>
  {
  public:
    typedef UG2d::node T;
  };

} // end namespace Dune

#endif
