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
                   CoordinateType& parentCoordinates) const
  {
    /*
       // The ordering of the coordinates are according to a Dune reference triangle
       //
       //  NOTE: all coordinates are barycentric (with respect to (P_0, P_1, P_2)
       //
       //                  P_2 = (0,0,1)
       //                   |\
       //                   | \
       //                   |  \      each sub triangle is numbered as used below
       //                   |   \     local numbering is count clockwise
       //                   |    \    starting with the lower left vertex
       //                   |     \   (i.e. child 0 consits of  { P_0, (P_0+P_1)/2 , (P_0+P_2)/2 }  )
       //                   |      \
       //                   |   1   \
       //                   |        \
       //                   |         \
       //      (0.5,0,0.5)  |----------\  (0,0.5,0.5) = (P_1 + P_2)/2
       //   = (P_0 + P_2)/2 |\         |\
       //                   | \        | \
       //                   |  \   3   |  \
       //                   |   \      |   \
       //                   |    \     |    \
       //                   |     \    |     \
       //                   |      \   |      \
       //                   |  0    \  |   2   \
       //                   |        \ |        \
       //                   |         \|         \
       //                   -----------------------
       //         (1,0,0) = P_0   (0.5,0.5,0)    P_1 = (0,1,0)
       //                         = (P_0 + P_1)/
       //
       //  NOTE: the strange numbering of the childs is due to the swap
       //  of the vertex number form ALUGrid to Dune reference triangles faces
       //  This means that in ALUGrid child 1 and 2 are swaped compared to
       //  this example here.
     */

    // this mapping map from the points (P_0,P_1,P_2) to the
    // 4 sub triangles from the picture above
    //
    // TODO: this mapping is static, so store it in an map
    switch(nChild_) {
    case 0 :
      // (1,0,0) --> (1,0,0)
      // (0,1,0) --> (0.5,0,5,0)
      // (0,0,1) --> (0.5,0,0.5)
      parentCoordinates[0] =
        1.0 - 0.5*childCoordinates[1] - 0.5*childCoordinates[2];

      // this rocks , best bug ever
      //parentCoordinates[1] = 0,5*childCoordinates[1];

      parentCoordinates[1] = 0.5*childCoordinates[1];
      parentCoordinates[2] = 0.5*childCoordinates[2];
      break;
    case 1 : // swaped case 1 and case 2
      // (1,0,0) --> (0.5,0,0.5)
      // (0,1,0) --> (0,0,5,0)
      // (0,0,1) --> (0,0,1)
      parentCoordinates[0] = 0.5*childCoordinates[0];
      parentCoordinates[1] = 0.5*childCoordinates[1];
      parentCoordinates[2] =
        1.0 - 0.5*childCoordinates[0] - 0.5*childCoordinates[1];
      break;
    case 2 :
      // (1,0,0) --> (0.5,0,5,0)
      // (0,1,0) --> (0,1,0)
      // (0,0,1) --> (0.5,0.5,0)
      parentCoordinates[0] = 0.5*childCoordinates[0];
      parentCoordinates[1] =
        1.0 - 0.5*childCoordinates[0] - 0.5*childCoordinates[2];
      parentCoordinates[2] = 0.5*childCoordinates[2];
      break;
    case 3 :
      // (1,0,0) --> (0.5,0,0.5)
      // (0,1,0) --> (0.5,0.5,0)
      // (0,0,1) --> (0,0.5,0.5)
      // here swaped all to the next position
      parentCoordinates[1] = 0.5 - 0.5*childCoordinates[0];
      parentCoordinates[2] = 0.5 - 0.5*childCoordinates[1];
      parentCoordinates[0] = 0.5 - 0.5*childCoordinates[2];
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
    //
    //
    //   (0,1)                   (1,1)
    //    -------------------------
    //    |           |           |     childs within the reference
    //    |           |           |     quadrilateral of Dune
    //    |    1      |     2     |
    //    |           |           |
    //    |           |           |
    //    |-----------|-----------|
    //    |           |           |
    //    |           |           |
    //    |    0      |     3     |
    //    |           |           |
    //    |           |           |
    //    -------------------------
    //  (0,0)                    (1,0)
    //
    //
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
