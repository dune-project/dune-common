// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "mappings.hh"

namespace Dune {
  const double TrilinearMapping :: _epsilon = 1.0e-8 ;

  NonConformingFaceMapping<tetra>::
  NonConformingFaceMapping(RefinementRuleType rule,
                           int nChild) :
    rule_(rule),
    nChild_(nChild) {}

  NonConformingFaceMapping<tetra>::
  NonConformingFaceMapping(const NonConformingFaceMapping<tetra>& orig) :
    rule_(orig.rule_),
    nChild_(orig.nChild_) {}

  NonConformingFaceMapping<tetra>::
  ~NonConformingFaceMapping<tetra>() {}

  void NonConformingFaceMapping<tetra>::
  child2parent(const CoordinateType& childCoordinates,
               CoordinateType& parentCoordinates) const {
    if (rule_ == RefinementRuleType::nosplit) {
      child2parentNosplit(childCoordinates, parentCoordinates);
    }
    else if (rule_ == RefinementRuleType::e01) {
      child2parentE01(childCoordinates, parentCoordinates);
    }
    else if (rule_ == RefinementRuleType::e12) {
      child2parentE12(childCoordinates, parentCoordinates);
    }
    else if (rule_ == RefinementRuleType::e20) {
      child2parentE20(childCoordinates, parentCoordinates);
    }
    else if (rule_ == RefinementRuleType::iso4) {
      child2parentIso4(childCoordinates, parentCoordinates);
    }
    else {
      // check with cases in Hface3Rule (gitter_sti.h)
      assert(false);
    } // end if
  }

  void NonConformingFaceMapping<tetra>::
  child2parentNosplit(const CoordinateType& childCoordinates,
                      CoordinateType& parentCoordinates) const {
    parentCoordinates = childCoordinates;
  }
  void NonConformingFaceMapping<tetra>::
  child2parentE01(const CoordinateType& childCoordinates,
                  CoordinateType& parentCoordinates) const {
    DUNE_THROW(NotImplemented,
               "This refinement rule is currently not supported");
  }

  void NonConformingFaceMapping<tetra>::
  child2parentE12(const CoordinateType& childCoordinates,
                  CoordinateType& parentCoordinates) const {
    DUNE_THROW(NotImplemented,
               "This refinement rule is currently not supported");
  }

  void NonConformingFaceMapping<tetra>::
  child2parentE20(const CoordinateType& childCoordinates,
                  CoordinateType& parentCoordinates) const {
    DUNE_THROW(NotImplemented,
               "This refinement rule is currently not supported");
  }

  void NonConformingFaceMapping<tetra>::
  child2parentIso4(const CoordinateType& childCoordinates,
                   CoordinateType& parentCoordinates) const {
    switch(nChild_) {
    case 0 :
      parentCoordinates[0] =
        1.0 - 0.5*childCoordinates[1] - 0.5*childCoordinates[2];
      parentCoordinates[1] = 0,5*childCoordinates[1];
      parentCoordinates[2] = 0.5*childCoordinates[2];
      break;
    case 1 :
      parentCoordinates[0] = 0.5*childCoordinates[0];
      parentCoordinates[1] =
        1.0 - 0.5*childCoordinates[0] - 0.5*childCoordinates[2];
      parentCoordinates[2] = 0.5*childCoordinates[2];
      break;
    case 2 :
      parentCoordinates[0] = 0.5*childCoordinates[0];
      parentCoordinates[1] = 0.5*childCoordinates[1];
      parentCoordinates[2] =
        1.0 - 0.5*childCoordinates[0] - 0.5*childCoordinates[1];
      break;
    case 3 :
      parentCoordinates[0] = 0.5 - 0.5*childCoordinates[0];
      parentCoordinates[1] = 0.5 - 0.5*childCoordinates[1];
      parentCoordinates[2] = 0.5 - 0.5*childCoordinates[2];
      break;
    default :
      DUNE_THROW(RangeError, "Only 4 children on a tetrahedron face (val = "
                 << nChild_ << ")");

    } // end switch
  }

  //- Specialisation for hexa
  NonConformingFaceMapping<hexa>::
  NonConformingFaceMapping(RefinementRuleType rule,
                           int nChild) :
    rule_(rule),
    nChild_(nChild) {}

  NonConformingFaceMapping<hexa>::
  NonConformingFaceMapping(const NonConformingFaceMapping<hexa>& orig) :
    rule_(orig.rule_),
    nChild_(orig.nChild_) {}

  NonConformingFaceMapping<hexa>::
  ~NonConformingFaceMapping<hexa>() {}

  void NonConformingFaceMapping<hexa>::
  child2parent(const CoordinateType& childCoordinates,
               CoordinateType& parentCoordinates) const {
    if (rule_ == RefinementRuleType::nosplit) {
      child2parentNosplit(childCoordinates, parentCoordinates);
    }
    else if (rule_ == RefinementRuleType::iso4) {
      child2parentIso4(childCoordinates, parentCoordinates);
    }
    else {
      // check with cases in Hface3Rule (gitter_sti.h)
      assert(false);
    }
  }

  void NonConformingFaceMapping<hexa>::
  child2parentNosplit(const CoordinateType& childCoordinates,
                      CoordinateType& parentCoordinates) const {
    parentCoordinates = childCoordinates;
  }

  void NonConformingFaceMapping<hexa>::
  child2parentIso4(const CoordinateType& childCoordinates,
                   CoordinateType& parentCoordinates) const {

    // The ordering of the coordinates are according to a Dune reference elemen
    switch(nChild_) {
    case 0 :
      parentCoordinates[0] = 0.5*childCoordinates[0];
      parentCoordinates[1] = 0.5*childCoordinates[1];
      break;
    case 1 :
      parentCoordinates[0] = 0.5*childCoordinates[0];
      parentCoordinates[1] = 0.5*childCoordinates[1] + 0.5;
      break;
    case 2 :
      parentCoordinates[0] = 0.5*childCoordinates[0] + 0.5;
      parentCoordinates[1] = 0.5*childCoordinates[1] + 0.5;
      break;
    case 3 :
      parentCoordinates[0] = 0.5*childCoordinates[0] + 0.5;
      parentCoordinates[1] = 0.5*childCoordinates[1];
      break;
    default :
      DUNE_THROW(RangeError, "Only 4 children on a hexahedron face (val = "
                 << nChild_ << ")");
    } // end switch
  }

} // end namespace Dune
