// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_UGTYPES_HH__
#define __DUNE_UGTYPES_HH__

namespace UG2d {
  struct multigrid;
  struct domain;

  union element;
  struct node;
};

namespace UG3d {
  struct multigrid;
  struct domain;

  union element;
  struct node;
};


namespace Dune {

  template <int dim>
  class UGTypes
  {
  public:
    typedef void MultiGridType;

    typedef void DomainType;
  };

  template <>
  class UGTypes<2>
  {
  public:
    typedef UG2d::multigrid MultiGridType;

    typedef UG2d::domain DomainType;
  };

  template <>
  class UGTypes<3>
  {
  public:
    typedef UG3d::multigrid MultiGridType;

    typedef UG3d::domain DomainType;
  };



  /*****************************************************************/
  /*****************************************************************/
  /*****************************************************************/
  /*****************************************************************/


  template <int codim, int dim>
  class TargetType
  {
  public:
    typedef void T;

  };

  template <>
  class TargetType<0,3>
  {
  public:
    typedef UG3d::element T;
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
  class TargetType<2,2>
  {
  public:
    typedef UG2d::node T;
  };

} // end namespace Dune

#endif
