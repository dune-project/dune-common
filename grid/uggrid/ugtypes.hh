// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_UGTYPES_HH__
#define __DUNE_UGTYPES_HH__

namespace Dune {

  template <int dim>
  class UGTypes
  {
  public:
    typedef void MultiGridType;

    typedef void DomainType;
  };

#ifdef _3
  template <>
  class UGTypes<3>
  {
  public:
    typedef UG3d::multigrid MultiGridType;

    typedef UG3d::domain DomainType;
  };
#else
  template <>
  class UGTypes<2>
  {
  public:
    typedef UG2d::multigrid MultiGridType;

    typedef UG2d::domain DomainType;
  };
#endif

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

#ifdef _3
  template <>
  class TargetType<0,3>
  {
  public:
    typedef UG3d::ELEMENT T;
  };

  template <>
  class TargetType<3,3>
  {
  public:
    typedef UG3d::NODE T;
  };
#else
  template <>
  class TargetType<0,2>
  {
  public:
    typedef UG2d::ELEMENT T;
  };

  template <>
  class TargetType<2,2>
  {
  public:
    typedef UG2d::NODE T;
  };
#endif

} // end namespace Dune

#endif
